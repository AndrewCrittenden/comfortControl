# -*- coding: utf-8 -*-
"""
Created on Thu Apr  1 17:27:57 2021

@author: rosem

"""

import os
import random
import time
from watchdog.observers import Observer
from watchdog.events import PatternMatchingEventHandler
from pythermalcomfort.models import pmv_ppd
from pythermalcomfort.psychrometrics import v_relative
from pythermalcomfort.psychrometrics import t_mrt
from pythermalcomfort.utilities import met_typical_tasks
from pythermalcomfort.models import adaptive_ashrae
import numpy as np

desiredTemp = 70
activity = "resting" #string
tdb = 82 #tdb, dry-bulb air temperature, [$^{\circ}$C], float
tg = 77.99 #tg, globe temperature, float
rh= 26.21 #rh, relative humidity, [%], float
tout = 76.83 #tout, outdoor temperature, float
occupancy = False # = 1 #occupancy


#sensor data here to be removed once the data is read in
    #tdb = 80# dry-bulb air temperature, [$^{\circ}$C]
    #tg = 78#globe temperature
    #rh = 50 # relative humidity, [%]
    #tout = 90 #outdoor temperature
    #occupancy = True #occupancy
    #print(count, tdb, tg, rh, tout, occupancy)
    #formula to convert to Fahrenheit from Celsius
    #F = (Cx1.8) + 32
for i in range(1):
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
    activity = "moderately active" #  "resting", "moderately active", "active"
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
        
        print(met) 

    #Held Constant for residencies
    v = 0.1  # average air velocity, [m/s]
    
    #MRT Calculation
    tr = t_mrt(tg, tdb, v, d=0.075, emissivity=0.95)
    #operative temp calulation
    op = 0.5 * (tdb + tr)
    #print(op)
    
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
        print("Potential unsafe conditions")
        error_2 = True
    if rh < 0 or rh > 100:   
        print("Malfunction rh")
        error_3 = True
    if tout < -50 or tout > 110:
        print("Malfunction outdoor temp")
        error_4 = True
    
    if tg <= 0 or tg >= 110:
        print("Malfunction gt")
        error_5 = True
        
    if tr <= 50 or tr >= 104:
        print("Mean radiant temperature outside ASHRAE operating conditions")
        error_6 = True
        
    if tdb <= 50 or tdb >= 104:
        print("Indoor temperature outside ASHRAE operating conditions" )
        error_7 = True
        
    if error_1 == False and error_2 == False and error_3 == False and error_4 == False and error_5 == False and error_6 == False and error_7 == False:
        #only enter the ashrae calcs if error free
        # calculate PMV in accordance with the ASHRAE 55 2017
        results = pmv_ppd(tdb=tdb, tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
    
        def get_first_key(dictionary):
            for key in dictionary:
                return key
            raise IndexError
        key1 = get_first_key(results)
        pmv = results[key1]
            
        ##################################################################
        #finding setpoints with iterative loop

        setpoint_options = np.arange(60,80, 0.1)
        pmv_options = np.zeros(len(setpoint_options))
        global setpoint_temp_prev 
        setpoint_temp_prev = 70
        
        if occupancy: #someone is home Occupancy = True
            if pmv >= -0.5 and pmv <= 0.5:
                pmv_bool = 1 #comfort conditions met
                print("Comfort Conditions met")
                print(pmv)
                setpoint_temp = setpoint_temp_prev #maintain the same temperature
                
            else:
                pmv_bool = 0 #comfrot conditons are not met
                print("Comfort Conditions NOT met")
                print(pmv)
                
                #for loop to collect the mathematicaly PMV that would result from each setpoint option
                for x in range(len(setpoint_options)):
                    results = pmv_ppd(tdb=setpoint_options[x], tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
                    key1 = get_first_key(results)
                    pmv_options[x]= results[key1]
        
                pmv_minimum = 5 #nonsensical value to be replaced by the loop
                pmv_minimum_index = 0 #returned to isolate the temperaure setpoint that goes with the minimum PMV
                for x in range(len(pmv_options)):
                    if pmv_options[x] <= 0.5 and pmv_options[x] >= -0.5:
                        if pmv > 0:
                            if 0.5 - pmv_options[x] < pmv_minimum: #want to have to change the pmv the least amount
                                pmv_minimum = 0.5 - pmv_options[x]
                                pmv_minimum_index = x
                        if pmv < 0:
                            if -0.5 + pmv_options[x] < pmv_minimum:
                                pmv_minimum =  -0.5 + pmv_options[x]
                                pmv_minimum_index = x
        
                setpoint_temp = round(setpoint_options[pmv_minimum_index],1 )
                print("PMV selected")
                print(pmv_options[pmv_minimum_index])
                print("setpoint selected:")
                print(setpoint_temp)
            

        else: #someone is not home Occupancy = False
            print("No one home")
            if pmv >= -1.0 and pmv <= 1.0:
                pmv_bool = 1 #comfort conditions met
                print("Comfort Conditions met")
                print(pmv)
                setpoint_temp = setpoint_temp_prev #maintain the same temperature
                print(setpoint_temp_prev)
        
                if pmv <= 0.75 and pmv > 0: ## and pmv >=-0.75
                    print('but want to relax comfort conditons to save energy')
    
                    for x in range(len(setpoint_options)):
                        if setpoint_options[x] >= tout: #need to make sure heat does not turn on if the temperature is relaxed
                            results = pmv_ppd(tdb=setpoint_options[x], tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
                            key1 = get_first_key(results)
                            pmv_options[x]= results[key1]
                        else:
                            pmv_options[x] = 100 #value will never be picked essentially removing those options 
    
                    pmv_minimum = 5 #nonsensical value to be replaced by the loop
                    pmv_minimum_index = 0 #returned to isolate the temperaure setpoint that goes with the minimum PMV
                    for x in range(len(pmv_options)):
                        if pmv_options[x] <= 1.0 and pmv_options[x] >= -1.0:
                        
                            if pmv > 0:
                                if 1 - pmv_options[x] < pmv_minimum:
                                    pmv_minimum = 1- pmv_options[x]
                                    pmv_minimum_index = x
                        
                            if pmv < 0 :
                                if 1 + pmv_options[x] < pmv_minimum:
                                    pmv_minimum = 1 + pmv_options[x]
                                    pmv_minimum_index = x
        
                    setpoint_temp = round(setpoint_options[pmv_minimum_index], 1) 
                    print("PMV selected")
                    print(pmv_options[pmv_minimum_index])
                    print("setpoint selected:")
                    print(setpoint_temp)
                    
                if pmv >= -0.75 and pmv <0: ## and pmv >=-0.75:
                    print('but want to relax comfort conditons to save energy')
    
                    for x in range(len(setpoint_options)):
                        if setpoint_options[x] <= tout: #need to make sure heat does not turn on if the temperature is relaxed
                            results = pmv_ppd(tdb=setpoint_options[x], tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
                            key1 = get_first_key(results)
                            pmv_options[x]= results[key1]
                        else:
                            pmv_options[x] = 100 #value will never be picked essentially removing those options 
    
                    pmv_minimum = 5 #nonsensical value to be replaced by the loop
                    pmv_minimum_index = 0 #returned to isolate the temperaure setpoint that goes with the minimum PMV
                    for x in range(len(pmv_options)):
                        if pmv_options[x] <= 1.0 and pmv_options[x] >= -1.0:
                        
                            if pmv > 0:
                                if 1 - pmv_options[x] < pmv_minimum:
                                    pmv_minimum = 1- pmv_options[x]
                                    pmv_minimum_index = x
                        
                            if pmv < 0 :
                                if 1 + pmv_options[x] < pmv_minimum:
                                    pmv_minimum = 1 + pmv_options[x]
                                    pmv_minimum_index = x
        
                    setpoint_temp = round(setpoint_options[pmv_minimum_index], 1) 
                    print("PMV selected")
                    print(pmv_options[pmv_minimum_index])
                    print("setpoint selected:")
                    print(setpoint_temp)
            
            else:
                pmv_bool = 0 #comfrot conditons are not met
                print("Comfort Conditions NOT met")
                
                #for loop to collect the mathematicaly PMV that would result from each setpoint option
                for x in range(len(setpoint_options)):
                    results = pmv_ppd(tdb=setpoint_options[x], tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
                    key1 = get_first_key(results)
                    pmv_options[x]= results[key1]
        
                pmv_minimum = 5 #nonsensical value to be replaced by the loop
                pmv_minimum_index = 0 #returned to isolate the temperaure setpoint that goes with the minimum PMV
                for x in range(len(pmv_options)):
                    if pmv_options[x] <= 1.0 and pmv_options[x] >= -1.0: #should this be 1 instead of 0.5
                        if 0.5 - abs(pmv_options[x]) < pmv_minimum:
                            pmv_minimum = pmv_options[x]
                            pmv_minimum_index = x
        
                setpoint_temp = round(setpoint_options[pmv_minimum_index], 1) 
                print("PMV minimum")
                print(pmv_minimum)
                print("setpoint selected:")
                print(setpoint_temp)
         
    else: 
        print('Error Detected. Operating to entered Desired Temp')
        setpoint_temp = desiredTemp
        PMV_bool = 0
        
    setpoint_temp_prev = setpoint_temp #saves the previous setpoint
    
    print(setpoint_temp)