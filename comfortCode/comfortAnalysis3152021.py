#!/usr/bin/env python3

# -*- coding: utf-8 -*-

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

# Environmental Conditions From Sensor Subsystem 

#This code will receive sensor information and send them to GUI for dispersion
#to other subsystems
#This code will flag errors and send any error codes to the GUI
#It will also assess the comfort of the space and determine a setpoint to maintain
#or improve the comfort 

#Default initial value, updated when GUI runs
desiredTemp = 72.0 #float
activity = "resting" #string
indoorTemp= 80.0 #tdb, dry-bulb air temperature, [$^{\circ}$C], float
globeTemp= 78.0 #tg, globe temperature, float
relHumidity= 50 #rh, relative humidity, [%], float
outdoorTemp = 90.0 #tout, outdoor temperature, float
occupancy = True # = 1 #occupancy

# IPCsendGui function sends sensor reading to the GUI C++ code.
def IPCsendGui(pmv, setpointTemp):
    path= "/home/pi/WA/comfortControl/fifo/comfortToGui.fifo"
    try:
        os.mkfifo(path)
    except FileExistsError:
        pass
    f = os.open(path, os.O_NONBLOCK|os.O_RDWR)
    toSend = str(pmv)+','+str(setpointTemp)+',\n'
    os.write(f,str.encode(toSend))
    print("Sent: PMV=", pmv,", setpointTemp=",setpointTemp)

class IPCreceiveGui(PatternMatchingEventHandler):
    def on_modified(self, event):
        global desiredTemp, activity, indoorTemp, globeTemp, relHumidity, outdoorTemp, occupancy
        super(IPCreceiveGui, self).on_modified(event)
        with open("/home/pi/WA/comfortControl/fifo/guiToComfort.fifo", 'r') as f:
            buf = f.read()
        listVal = buf.split(',')
        desiredTemp = float(listVal[0])
        activity = listVal[1]
        if (listVal[2] == "True"):
            occupancy = True
        else:
            occupancy = False
        globeTemp = float(listVal[3])
        relHumidity = float(listVal[4])
        outdoorTemp = float(listVal[5])
        indoorTemp = float(listVal[6])
        print("Received: desiredTemp=",desiredTemp,
             ", activity=", activity,
             ", occupancy=", occupancy,
             ", globeTemp=", globeTemp,
             ", relHumidity=", relHumidity,
             ", outdoorTemp=", outdoorTemp,
             ", indoorTemp=", indoorTemp)

def get_first_key(dictionary):
    for key in dictionary:
        return key
    raise IndexError

#variables to recieve from the GUI: count, tdb, tg, rh, tout, occupancy, desiredTemp, activity
#variables to send to the GUI: errors_1 to 5, setpoint_temp, PMV, PMV_bool (we need PMV bool for Joseph I think)

##need you to add "tdb_reset = np.zeros(1440*365)" and "setpoint_temp = 0" in the pipes to run a single time like the variable count. 
#I have them here so that I can test. I think we should also include the reset if loop at the top of the inside of the function to keep from infinitely storing data

#eventually we should remove all the print() statements and rely on the GUI
#Don't need to do this. When GUI runs none of these print since it is a background process
#tout_saved = np.zeros(1440*365)
setpoint_temp = desiredTemp
setpoint_temp_prev= desiredTemp

def comfortAnalysis(count, tdb, tg, rh, tout, occupancy):
    #START###########################################
    #sensor data here to be removed once the data is read in
    #tdb = 80# dry-bulb air temperature, [$^{\circ}$C]
    #tg = 78#globe temperature
    #rh = 50 # relative humidity, [%]
    #tout = 90 #outdoor temperature
    #occupancy = 1 #occupancy
    print(count, tdb, tg, rh, tout, occupancy)
    #formula to convert to Fahrenheit from Celsius
    #F = (Cx1.8) + 32
        
    #reset all error flags, needed all error variables in case more than one is true
    error_1 = ""
    error_2 = ""
    error_3 = ""
    error_4 = ""
    error_5 = ""
    
    if tdb <= 0 or tdb >= 110:  #test logic behind the measurements 
        error_1 = "Malfunction indoor temp"
        
    if tdb <= 55 or tdb >= 90:
        error_2 = "Potential unsafe conditions" 

    if rh < 0 or rh > 100:   
        error_3 = "Malfunction rh"
    
    if tout < -50 or tout > 110:
        error_4 ="Malfunction outdoor temp"
    
    if tg <= 0 or tg >= 110:
        error_5 ="Malfunction gt"

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

    setpoint_options = np.array([60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,78,79,80])
    pmv_options = np.zeros(len(setpoint_options))
    global setpoint_temp_prev

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
        
            setpoint_temp = setpoint_options[pmv_minimum_index] 
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
        
            if pmv <= 0.75 and pmv >=-0.75:
                print('but want to relax comfort conditons to save energy')
    
                for x in range(len(setpoint_options)):
                    results = pmv_ppd(tdb=setpoint_options[x], tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
                    key1 = get_first_key(results)
                    pmv_options[x]= results[key1]
    
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
        
                setpoint_temp = setpoint_options[pmv_minimum_index] 
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
                if pmv_options[x] <= 0.5 and pmv_options[x] >= -0.5:
                    if 0.5 - abs(pmv_options[x]) < pmv_minimum:
                        pmv_minimum = pmv_options[x]
                        pmv_minimum_index = x
        
            setpoint_temp = setpoint_options[pmv_minimum_index] 
            print("PMV minimum")
            print(pmv_minimum)
            print("setpoint selected:")
            print(setpoint_temp)
        
        setpoint_temp_prev = setpoint_temp #saves the previous setpoint

    #END###########################################
    return pmv, setpoint_temp

# main initializes comfort analysis script
def main():
    print("Setup")
    #Setup watchdog to handle receive from GUI event
    fifoFile = "/home/pi/WA/comfortControl/fifo/guiToComfort.fifo"
    fifoDir = os.path.split(fifoFile)[0]
    observer = Observer()
    handler = IPCreceiveGui([fifoFile])
    observer.schedule(handler, fifoDir, recursive=True)
    observer.start()
    try:
        #Main control loop of comfort analysis script
        numComfortAnalysisRuns = 0
        while(1):
            #calculate pmv
            print("Current: desiredTemp=",desiredTemp,
                 ", activity=", activity,
                 ", occupancy=", occupancy,
                 ", globeTemp=", globeTemp,
                 ", relHumidity=", relHumidity,
                 ", outdoorTemp=", outdoorTemp,
                 ", indoorTemp=", indoorTemp)
            pmv, setpointTemp = comfortAnalysis(numComfortAnalysisRuns, indoorTemp, globeTemp, relHumidity, outdoorTemp, occupancy)
            IPCsendGui(pmv, setpointTemp) 
            time.sleep(1)
            numComfortAnalysisRuns = numComfortAnalysisRuns + 1
    except KeyboardInterrupt:
        observer.stop()
    observer.join()

main()
