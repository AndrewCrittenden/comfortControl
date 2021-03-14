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

### ANDREW SEE HERE
#variable names to be passed to the GUI: tdb, rh, gt, tout, occupancy, errors_1 to 5, 
#setpoint_temp, PMV, PMV_bool (we need PMV bool for Joseph I think)

##need you to add "tdb_reset = np.zeros(1440*365)" and "setpoint_temp = 0" in the pipes to run a single time like the variable count. 
#I have them here so that I can test. I think we should also include the reset if loop at the top of the inside of the function to keep from infinitely storing data
#eventually we should remove all the print() statements and rely on the GUI


tout_saved = np.random.random_sample((1440*30 + 5*1440),) + 60 #testing that the indexing is done right

count = 1440*30 + 1440

setpoint_temp = 67

 
############################################################################################  
if count> 365*1440 : #keep the number of saved data points from being too large - can adjust is 1 year is too many
   tout_reset = np.zeros(1440*365) 
   tout_reset[0:1440*30-1] = tout_saved[365*1440 -30*1440: 365*1440]
   tout_saved = tout_reset
   count = 1440*30 + 1 

#sensor data here to be removed once the data is read in
tdb = 75# dry-bulb air temperature, [$^{\circ}$C]
tg = 76 #globe temperature
rh = 50  # relative humidity, [%]
tout = 40 #outdoor temperature
occupancy = 1 #occupancy

#formula to convert to Fahrenheit from Celsius
#F = (Cx1.8) + 32
    
#reset all error flags, needed all error variables in case more than one is true
error_1 = ""
error_2 = ""
error_3 = ""
error_4 = ""
error_5 = ""

if tdb <= 0 | tdb >= 110:  #test logic behind the measurements 
    error_1 = "Malfunction indoor temp"
    
if tdb <= 55 | tdb >= 90:
    error_2 = "Potential unsafe conditions" 

if rh < 0 | rh > 100:
    error_3 = "Malfunction rh"
    
if tout < -50 | tout > 110:
    error_4 ="Malfunction outdoor temp"
    
if tg <= 0 | tg >= 110:
    error_5 ="Malfunction gt"

#clothing level prediction
if tout >= 75: #summer
    icl = 0.5 # clo_typical_ensembles('Typical summer indoor clothing') # calculate total clothing insulation
if tout < 75 & tout >= 60: #spring
    icl = 0.61 #clo_typical_ensembles('Trousers, long-sleeve sweatshirt')
if tout < 60 & tout >= 45: #fall
    icl = 0.74 #clo_typical_ensembles('Sweat pants, long-sleeve sweatshirt')
if tout < 45: #winter
    icl = 1.0 #clo_typical_ensembles('Typical winter indoor clothing')

#imput from user - recieved from GUI
activity = "moderately active" #  "resting", "moderately active", "active"
if activity == "resting":
    met = met_typical_tasks['Seated, quiet']    
if activity == "moderately active":
    met = met_typical_tasks['Walking about']  
if activity == "active":
    met = met_typical_tasks['Calisthenics']
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

#print(pmv)

# print PMV value- this whole group can be eventually deleted
print(f"pmv={results['pmv']}, ppd={results['ppd']}%")

if pmv <= float(0.5) and pmv >= float(-0.5):
    print('Acceptable!')
    
if pmv >= 0.5 : 
    print('Too warm!Turn on fan')
    
if pmv <= -0.5:
    print('Too cold! Turn on heater! or Grab Blanket')

##################################################################
#finding the setpoints

num_runs = count  #get this value from pipes 

if num_runs <= 1440*30: #for when less than a months worth of data is collected 
    t_running_mean = 45 #average temperature in March in Lexington, KY 
    tout_saved[num_runs] = tout
else : 
    t_running_mean = sum(tout_saved[(num_runs- 30*1440):num_runs])/(30*1440)
    print(t_running_mean)

setpoints = adaptive_ashrae(tdb, tr, t_running_mean, v, units="IP")
#print(setpoints)

print(f"low={setpoints['tmp_cmf_90_low']}, high={setpoints['tmp_cmf_90_up']}%")

if pmv <-0.5 and pmv>0.5: #check if outside the comfortable range
    if occupancy == 1:
        #checking if the change fixed it summer
        PMV_bool = 0
        setpoint_low = setpoints['tmp_cmf_90_low']
        tg = tdb + 2 #can adjust this guess based on how the gt in the fridge changes with the heating or cooling
        tr = t_mrt(tg, tdb, v, d=0.075, emissivity=0.95)
        
        results_low = pmv_ppd(tdb=tdb, tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
        print('Using low setpoint')
        print(f"pmv={results['pmv']}, ppd={results['ppd']}%")
        
        setpoint_high =setpoints['tmp_cmf_90_up']
        tg = setpoint_high + 2 #can adjust based on how the gt in the fridge changes with the heating or cooling 
        tr = t_mrt(tg, tdb, v, d=0.075, emissivity=0.95)
        
        results_high = pmv_ppd(tdb=tdb, tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
        print('Using high setpoint')
        print(f"pmv={results['pmv']}, ppd={results['ppd']}%")
        
        tdb = (setpoints['tmp_cmf_90_up'] + setpoints['tmp_cmf_90_low'])/2
        tg = tdb + 2 #can adjust based on how the gt in the fridge changes with the heating or cooling
        tr = t_mrt(tg, tdb, v, d=0.075, emissivity=0.95)
        print('mid setpoint')
        print(tdb)
        
        results_mid = pmv_ppd(tdb=tdb, tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
        print('Using mid setpoint')
        print(f"pmv={results['pmv']}, ppd={results['ppd']}%")
        
        #want the set point closest to the middle neutral temperature for optimal comfort
        set_point_PMV = min(abs(0 - results_low), abs(0 - results_high), abs(0-results_mid))  #this setpoint will be sent to Joseph
        
        #select the temperature that corresponds to the PMV setpoint 
        if set_point_PMV == results_high:
            set_point_temp = setpoints['tmp_cmf_80_up']
        if set_point_PMV == results_mid:
            set_point_temp = (setpoints['tmp_cmf_80_up'] + setpoints['tmp_cmf_80_low'])/2
        if set_point_PMV == results_low:
            set_point_temp = setpoints['tmp_cmf_80_low']
        
        
    if occupancy == 0: #relaxed comfort parameters
        #checking if the change fixed it summer
        if pmv >1 or pmv<-1:
            PMV_bool = 0
            setpoint_low = setpoints['tmp_cmf_80_low']
            tg = tdb + 2 #can adjust based on how the gt in the fridge changes with the heating or cooling
            tr = t_mrt(tg, tdb, v, d=0.075, emissivity=0.95)
        
            results_low = pmv_ppd(tdb=tdb, tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
            print('Using low setpoint')
            print(f"pmv={results['pmv']}, ppd={results['ppd']}%")
        
            setpoint_high = setpoints['tmp_cmf_80_up']
            tg = setpoint_high + 2 #can adjust based on how the gt in the fridge changes with the heating or cooling 
            tr = t_mrt(tg, tdb, v, d=0.075, emissivity=0.95)
        
            results_high = pmv_ppd(tdb=tdb, tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
            print('Using high setpoint')
            print(f"pmv={results['pmv']}, ppd={results['ppd']}%")
        
            tdb = (setpoints['tmp_cmf_80_up'] + setpoints['tmp_cmf_80_low'])/2
            tg = tdb + 2 #can adjust based on how the gt in the fridge changes with the heating or cooling
            tr = t_mrt(tg, tdb, v, d=0.075, emissivity=0.95)
            print('mid setpoint')
            print(tdb)
        
            results_mid = pmv_ppd(tdb=tdb, tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
            print('Using mid setpoint')
            print(f"pmv={results['pmv']}, ppd={results['ppd']}%")
            
            if results_high < -1 or results_high > 1:
                results_high = 0 #really low compared to the other options and wont be selected as it is an infeasible option
            
            if results_low < -1 or results_low > 1:
                result_low = 0
                
            if results_mid < -1 or results_mid > 1:
                results_mid = 0
                    
                    #comparing the setpoints to select the one closest to the edge of the comfortable window to lead to maximum savings in energy costs
            set_point_PMV = max( abs(0 - results_low), abs(0 - results_high), abs(0 - results_mid))  #this setpoint will be sent to Joseph
                    
            #select the temperature that corresponds to the PMV setpoint 
            if set_point_PMV == results_high:
                set_point_temp = setpoints['tmp_cmf_80_up']
            if set_point_PMV == results_mid:
                set_point_temp = (setpoints['tmp_cmf_80_up'] + setpoints['tmp_cmf_80_low'])/2
            if set_point_PMV == results_low:
                set_point_temp = setpoints['tmp_cmf_80_low']
        else:
            PMV_bool = 1
            setpoint_temp = setpoint_temp
                                
else:
    PMV_bool = 1
    if occupancy == 1:              
        setpoint_temp = setpoint_temp # keep the same setpoint if it is in the comfortable range no need to cause the hvac to turn on if it is comfortable
    else: #allow the comfort conditions to relax to with the (-1 to 1 range)
        setpoint_low = setpoints['tmp_cmf_80_low']
        tg = tdb + 2 #can adjust based on how the gt in the fridge changes with the heating or cooling
        tr = t_mrt(tg, tdb, v, d=0.075, emissivity=0.95)
        
        results_low = pmv_ppd(tdb=tdb, tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
        print('Using low setpoint')
        print(f"pmv={results['pmv']}, ppd={results['ppd']}%")
        
        setpoint_high = setpoints['tmp_cmf_80_up']
        tg = setpoint_high + 2 #can adjust based on how the gt in the fridge changes with the heating or cooling 
        tr = t_mrt(tg, tdb, v, d=0.075, emissivity=0.95)
        
        results_high = pmv_ppd(tdb=tdb, tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
        print('Using high setpoint')
        print(f"pmv={results['pmv']}, ppd={results['ppd']}%")
        
        tdb = (setpoints['tmp_cmf_80_up'] + setpoints['tmp_cmf_80_low'])/2
        tg = tdb + 2 #can adjust based on how the gt in the fridge changes with the heating or cooling
        tr = t_mrt(tg, tdb, v, d=0.075, emissivity=0.95)
        print('mid setpoint')
        print(tdb)
        
        results_mid = pmv_ppd(tdb=tdb, tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
        print('Using mid setpoint')
        print(f"pmv={results['pmv']}, ppd={results['ppd']}%")
            
        if results_high < -1 or results_high > 1:
            results_high = 0 #really low compared to the other options and wont be selected as it is an infeasible option
            
        if results_low < -1 or results_low > 1:
            result_low = 0
                
        if results_mid < -1 or results_mid > 1:
            results_mid = 0
                    
        #comparing the setpoints to select the one closest to the edge of the comfortable window to lead to maximum savings in energy costs
        set_point_PMV = max( abs(0 - results_low), abs(0 - results_high), abs(0 - results_mid))  #this setpoint will be sent to Joseph
                    
        #select the temperature that corresponds to the PMV setpoint 
        if set_point_PMV == results_high:
            set_point_temp = setpoints['tmp_cmf_80_up']
        if set_point_PMV == results_mid:
            set_point_temp = (setpoints['tmp_cmf_80_up'] + setpoints['tmp_cmf_80_low'])/2
        if set_point_PMV == results_low:
            set_point_temp = setpoints['tmp_cmf_80_low']
                           
                  
count = count+1
