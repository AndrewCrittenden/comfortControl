import time
initialize_time = time.time()

import os
import random
import time
from watchdog.observers import Observer
from watchdog.events import PatternMatchingEventHandler
from watchdog.events import FileSystemEventHandler
from pythermalcomfort.models import pmv_ppd
from pythermalcomfort.psychrometrics import v_relative
from pythermalcomfort.psychrometrics import t_mrt
from pythermalcomfort.utilities import met_typical_tasks
from pythermalcomfort.models import adaptive_ashrae
import numpy as np
import psutil

# Environmental Conditions From Sensor Subsystem 

#This code will receive sensor information and send them to GUI for dispersion
#to other subsystems
#This code will flag errors and send any error codes to the GUI
#It will also assess the comfort of the space and determine a setpoint to maintain
#or improve the comfort 
# values to be returned by the comfort code: PMV, PMV_bool, error_1 to error_7 in bool, setpoint_temp. This is 10 things

#Default initial value, updated when GUI runs
desiredTemp = 75.07 #float
activity = "resting" #string
indoorTemp= 76.83 #tdb, dry-bulb air temperature, [$^{\circ}$C], float
globeTemp= 77.99 #tg, globe temperature, float
relHumidity= 26.21 #rh, relative humidity, [%], float
outdoorTemp = 76.83 #tout, outdoor temperature, float
occupancy = False # = 1 #occupancy
pmv = 0
setpoint_temp = desiredTemp
setpoint_temp_prev= desiredTemp

def get_first_key(dictionary):
    for key in dictionary:
        return key
    raise IndexError

def comfortAnalysis(tdb, tg, rh, tout, occupancy):
    start_time = time.time()
    global pmv, setpoint_temp
    #sensor data here to be removed once the data is read in
    #tdb = 80# dry-bulb air temperature, [$^{\circ}$C]
    #tg = 78#globe temperature
    #rh = 50 # relative humidity, [%]
    #tout = 90 #outdoor temperature
    #occupancy = True #occupancy
    #print("        comfortAnalysis.py",count, tdb, tg, rh, tout, occupancy)
    #formula to convert to Fahrenheit from Celsius
    #F = (Cx1.8) + 32

   #clothing level prediction
    if tout >= 75: #summer
        icl = 0.5 # clo_typical_ensembles('Typical summer indoor clothing') # calculate total clothing insulation
    if tout < 75 and tout >= 60: #spring
        icl = 0.61 #clo_typical_ensembles('Trousers, long-sleeve sweatshirt')
    if tout < 60 and tout >= 45: #fall
        icl = 0.74 #clo_typical_ensembles('Sweat pants, long-sleeve sweatshirt')
    if tout < 45: #winter
        icl = 1.0 #clo_typical_ensembles('Typical winter indoor clothing')

    #imput from user - recieved from GUI
    #activity = "moderately active" #  "resting", "moderately active", "active"
    if activity == "resting":
        met = met_typical_tasks['Seated, quiet']  #1.0 met range  
    if activity == "moderately active":
        met = 1.5  #walking around
    if activity == "active":
        met = 2 #light exercise
        if tout < 50:
            clo = .5 
        else :
            clo = 0.36 #workout clothes
        
        #print("        comfortAnalysis.py",met) 

    #Held Constant for residencies
    v = 0.1  # average air velocity, [m/s]
    
    #MRT Calculation
    tr = t_mrt(tg, tdb, v, d=0.075, emissivity=0.95)
    #operative temp calulation
    op = 0.5 * (tdb + tr)
    #print("        comfortAnalysis.py",op)
    
    # calculate the relative air velocity
    vr = v_relative(v=v, met=met)
    
    ############################################################################
    #check data for errors
    #reset all error flags, needed all error variables in case more than one is true
    error_1 = False
    error_2 = False
    error_3 = False
    error_4 = False
    error_5 = False
    error_6 = False
    error_7 = False
    
    if tdb <= 0 or tdb >= 110:  #test logic behind the measurements 
        #error_1 = "Malfunction indoor temp"
        error_1 = True
        
    if tdb <= 55 or tdb >= 90:
        print("        comfortAnalysis.py","WARNING: Potential unsafe conditions")
        error_2 = True
    if rh < 0 or rh > 100:   
        print("        comfortAnalysis.py","Malfunction RH sensor")
        error_3 = True
    if tout < -50 or tout > 110:
        print("        comfortAnalysis.py","Malfunction Outdoor Temp sensor")
        error_4 = True
    
    if tg <= 0 or tg >= 110:
        print("        comfortAnalysis.py","Malfunction GT Sensor")
        error_5 = True
        
    if tr <= 50 or tr >= 104:
        print("        comfortAnalysis.py","ERROR: Mean radiant temperature outside ASHRAE operating conditions")
        error_6 = True
        
    if tdb <= 50 or tdb >= 104:
        print("        comfortAnalysis.py","ERROR: Indoor temperature outside ASHRAE operating conditions" )
        error_7 = True
        
    if error_1 == False and error_2 == False and error_3 == False and error_4 == False and error_5 == False and error_6 == False and error_7 == False:
        #only enter the ashrae calcs if error free
        # calculate PMV in accordance with the ASHRAE 55 2017
        results = pmv_ppd(tdb=tdb, tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
   
        key1 = get_first_key(results) #function from above to use python variable type dictionary
        pmv = results[key1]
            
        ##################################################################
        #finding setpoints with iterative loop

        setpoint_options = np.arange(60,80, 0.1) # want to test setpoints between 60 and 80
        pmv_options = np.zeros(len(setpoint_options)) #will run a hypothetical PMV calc with that temperature to select the best one
        global setpoint_temp_prev #needs to be accessabile in all parts of the code
        
        if occupancy: #someone is home Occupancy = True
            if pmv >= -0.5 and pmv <= 0.5: 
                pmv_bool = 1 #comfort conditions met
                #print("        comfortAnalysis.py","Comfort Conditions met")
                #print("        comfortAnalysis.py",pmv)
                setpoint_temp = setpoint_temp_prev #maintain the same temperature
                
            else: 
                pmv_bool = 0 #comfort conditons are not met
                #print("        comfortAnalysis.py","Comfort Conditions NOT met")
                #print("        comfortAnalysis.py",pmv)
                
                #for loop to iteratively test which PMV that would result from each setpoint option
                for x in range(len(setpoint_options)):
                    results = pmv_ppd(tdb=setpoint_options[x], tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
                    key1 = get_first_key(results)
                    pmv_options[x]= results[key1]
        
                pmv_minimum = 5 #nonsensical value to be replaced by the loop
                pmv_minimum_index = 0 #returned to isolate the temperaure setpoint that goes with the minimum PMV
                for x in range(len(pmv_options)): #check each potential PMV to find the one that best meets the range -0.5 to 0.5
                    if pmv_options[x] <= 0.5 and pmv_options[x] >= -0.5:
                        if pmv > 0:# already know heat or cool must come on because it is outside the range so isolate which one and find the closest value to the edge of comfort range
                            if 0.5 - pmv_options[x] < pmv_minimum: #want to have to change the pmv to the closest to 0.5 to save energy and still be comfortable
                                pmv_minimum = 0.5 - pmv_options[x]
                                pmv_minimum_index = x
                        if pmv < 0: # already know heat or cool must come on because it is outside the range so isolate which one and find the closest value to the edge of comfort range
                            if 0.5 + pmv_options[x] < pmv_minimum:
                                pmv_minimum =  0.5 + pmv_options[x]
                                pmv_minimum_index = x
        
                setpoint_temp = round(setpoint_options[pmv_minimum_index],1 )
                #print("        comfortAnalysis.py","PMV selected")
                #print("        comfortAnalysis.py",pmv_options[pmv_minimum_index])
                #print("        comfortAnalysis.py","setpoint selected:")
                #print("        comfortAnalysis.py",setpoint_temp)
            

        else: #someone is not home Occupancy = False
            #print("        comfortAnalysis.py","No one home")
            if pmv >= -1.0 and pmv <= 1.0:
                pmv_bool = 1 #comfort conditions met
                #print("        comfortAnalysis.py","Comfort Conditions met")
                #print("        comfortAnalysis.py",pmv)
                setpoint_temp = setpoint_temp_prev #maintain the same temperature
                #print("        comfortAnalysis.py",setpoint_temp_prev)
        
                if pmv <= 0.75 and pmv >= 0:#pmv is warm but could be relaxed to be warmer
                    #print("        comfortAnalysis.py",'but want to relax comfort conditons to save energy')
    
                    for x in range(len(setpoint_options)): #get potential PMV for setpoint options
                        if setpoint_options[x] >= tout: #need to make sure heat does not turn on past conditions outside
                            results = pmv_ppd(tdb=setpoint_options[x], tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
                            key1 = get_first_key(results)
                            pmv_options[x]= results[key1]
                        else:
                            pmv_options[x] = 100 #value will never be picked essentially removing those options 
    
                    pmv_minimum = 5 #nonsensical value to be replaced by the loop
                    pmv_minimum_index = 0 #returned to isolate the temperaure setpoint that goes with the minimum PMV
                    for x in range(len(pmv_options)): #for loop to collect iteratively the PMV that would result from each setpoint option
                        if pmv_options[x] <= 1.0 and pmv_options[x] >= 0.75: #only compare the PMV possibilities within the range we want
                            if 1 - pmv_options[x] < pmv_minimum:
                                pmv_minimum = 1- pmv_options[x]
                                pmv_minimum_index = x
                    
                    setpoint_temp = round(setpoint_options[pmv_minimum_index], 1) 
                    #print("        comfortAnalysis.py","PMV selected")
                    #print("        comfortAnalysis.py",pmv_options[pmv_minimum_index])
                    #print("        comfortAnalysis.py","setpoint selected:")
                    #print("        comfortAnalysis.py",setpoint_temp)
                    
                if pmv >= -0.75 and pmv <= 0: ##pmv is cold but could be relaxed to be cooler
                    #print("        comfortAnalysis.py",'but want to relax comfort conditons to save energy')
    
                    for x in range(len(setpoint_options)): #get potential PMV for setpoint options
                        if setpoint_options[x] <= tout: #need to make sure heat does not turn on if the temperature is relaxed
                            results = pmv_ppd(tdb=setpoint_options[x], tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
                            key1 = get_first_key(results)
                            pmv_options[x]= results[key1]
                        else:
                            pmv_options[x] = 100 #value will never be picked essentially removing those options (next if statement will skip all these)
    
                    pmv_minimum = 5 #nonsensical value to be replaced by the loop
                    pmv_minimum_index = 0 #returned to isolate the temperaure setpoint that goes with the minimum PMV
                    for x in range(len(pmv_options)):
                        if pmv_options[x] <= -0.75 and pmv_options[x] >= -1.0: # make sure the potential PMV is within the range                
                            if 1 + pmv_options[x] < pmv_minimum: #pmv will be a negative number so adding to find the minimum distance from edge of range
                                pmv_minimum = 1 + pmv_options[x]
                                pmv_minimum_index = x
        
                    setpoint_temp = round(setpoint_options[pmv_minimum_index], 1) 
                    #print("        comfortAnalysis.py","PMV selected")
                    #print("        comfortAnalysis.py",pmv_options[pmv_minimum_index])
                    #print("        comfortAnalysis.py","setpoint selected:")
                    #print("        comfortAnalysis.py",setpoint_temp)
            
            else:
                pmv_bool = 0 #comfort conditons are not met in case no one is home
                #print("        comfortAnalysis.py","Comfort Conditions NOT met")
                
                if pmv >= 0:#pmv is too warm
                    #print("        comfortAnalysis.py",'but want to relax comfort conditons to save energy')
                    for x in range(len(setpoint_options)): #get potential PMV for setpoint options
                        if setpoint_options[x] >= tout: #need to make sure heat does not turn on past conditions outside
                            results = pmv_ppd(tdb=setpoint_options[x], tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
                            key1 = get_first_key(results)
                            pmv_options[x]= results[key1]
                        else:
                            pmv_options[x] = 100 #value will never be picked essentially removing those options 
    
                    pmv_minimum = 5 #nonsensical value to be replaced by the loop
                    pmv_minimum_index = 0 #returned to isolate the temperaure setpoint that goes with the minimum PMV
                    for x in range(len(pmv_options)): #for loop to collect iteratively the PMV that would result from each setpoint option
                        if pmv_options[x] <= 1.0 and pmv_options[x] >= 0.75: #only compare the PMV possibilities within the range we want
                            if 1 - pmv_options[x] < pmv_minimum:
                                pmv_minimum = 1- pmv_options[x]
                                pmv_minimum_index = x
                    
                    setpoint_temp = round(setpoint_options[pmv_minimum_index], 1) 
                    #print("        comfortAnalysis.py","PMV selected")
                    #print("        comfortAnalysis.py",pmv_options[pmv_minimum_index])
                    #print("        comfortAnalysis.py","setpoint selected:")
                    #print("        comfortAnalysis.py",setpoint_temp)
                    
                if pmv <= 0: ##pmv is too cold 
                    #print("        comfortAnalysis.py",'but want to relax comfort conditons to save energy')
    
                    for x in range(len(setpoint_options)): #get potential PMV for setpoint options
                        if setpoint_options[x] <= tout: #need to make sure heat does not turn on if the temperature is relaxed
                            results = pmv_ppd(tdb=setpoint_options[x], tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
                            key1 = get_first_key(results)
                            pmv_options[x]= results[key1]
                        else:
                            pmv_options[x] = 100 #value will never be picked essentially removing those options (next if statement will skip all these)
    
                    pmv_minimum = 5 #nonsensical value to be replaced by the loop
                    pmv_minimum_index = 0 #returned to isolate the temperaure setpoint that goes with the minimum PMV
                    for x in range(len(pmv_options)):
                        if pmv_options[x] <= -0.75 and pmv_options[x] >= -1.0: # make sure the potential PMV is within the range                
                            if 1 + pmv_options[x] < pmv_minimum: #pmv will be a negative number so adding to find the minimum distance from edge of range
                                pmv_minimum = 1 + pmv_options[x]
                                pmv_minimum_index = x
        
                    setpoint_temp = round(setpoint_options[pmv_minimum_index], 1) 
                    #print("        comfortAnalysis.py","PMV selected")
                    #print("        comfortAnalysis.py",pmv_options[pmv_minimum_index])
                    #print("        comfortAnalysis.py","setpoint selected:")
                    #print("        comfortAnalysis.py",setpoint_temp)
            
        
                setpoint_temp = round(setpoint_options[pmv_minimum_index], 1) 
                #print("        comfortAnalysis.py","PMV minimum")
                #print("        comfortAnalysis.py",pmv_minimum)
                #print("        comfortAnalysis.py","setpoint selected:")
                #print("        comfortAnalysis.py",setpoint_temp)
         
    else: 
        #print("        comfortAnalysis.py",'Error Detected. Operating to entered Desired Temp')
        setpoint_temp = desiredTemp
        PMV_bool = False 
        PMV =  0.00
        
    setpoint_temp_prev = setpoint_temp #saves the previous setpoint
    
    #print("        comfortAnalysis.py",pmv, setpoint_temp)
    #print("        comfortAnalysis.py","ComfortAnalysis Code took", time.time() - start_time, "to run")
    return (pmv,pmv_bool,setpoint_temp, error_1, error_2, error_3, error_4, error_5, error_6, error_7)
    #END###########################################

#print("        comfortAnalysis.py","Intialize code took", time.time() - initialize_time, "to run")
