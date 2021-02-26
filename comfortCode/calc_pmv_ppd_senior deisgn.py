from pythermalcomfort.models import pmv_ppd
from pythermalcomfort.psychrometrics import v_relative
from pythermalcomfort.psychrometrics import t_mrt
from pythermalcomfort.utilities import met_typical_tasks
from pythermalcomfort.models import adaptive_ashrae


import csv 

# Environmental Conditions From Sensor Subsystem 

#This code will receive sensor information and send them to GUI for dispersion
#to other subsystems
#This code will flag errors and send any error codes to the GUI
#It will also assess the comfort of the space and determine a setpoint to maintain
#or improve the comfort 



#need to add a line to store the readings for every day or hour in an array and 
#then have a count variable to clear it after a certain amount of time
# then write to an arrray and wipe
tdb = 75# dry-bulb air temperature, [$^{\circ}$C]
tg = 76 #globe temperature
rh = 50  # relative humidity, [%]
tout = 40 #outdoor temperature
occupancy = 1 #occupancy

# data check with expected values

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

# print PMV value
print(f"pmv={results['pmv']}, ppd={results['ppd']}%")

if pmv <= float(0.5) and pmv >= float(-0.5):
    print('Acceptable!')
    
if pmv >= 0.5 : 
    print('Too warm!Turn on fan')
    
if pmv <= -0.5:
    print('Too cold! Turn on heater! or Grab Blanket')

##################################################################
#finding the setpoints

t_running_mean = 68 ### this will need to be calculated based on the past couple of hours

setpoints = adaptive_ashrae(tdb, tr, t_running_mean, v, units="IP")
#print(setpoints)

print(f"low={setpoints['tmp_cmf_90_low']}, high={setpoints['tmp_cmf_90_up']}%")

if pmv <-0.5 & pmv>0.5:
    if occupancy == 1:
        #checking if the change fixed it summer
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
        
        set_point = min(abs(0 - results_low), abs(0 - results_high), abs(0-results_mid))  #this setpoint will be sent to Joseph
        
    if occupancy == 0: #relaxed comfort parameters
        #checking if the change fixed it summer
        setpoint_low = setpoints['tmp_cmf_80_low']
        tg = tdb + 2 #can adjust based on how the gt in the fridge changes with the heating or cooling
        tr = t_mrt(tg, tdb, v, d=0.075, emissivity=0.95)
        
        results_low = pmv_ppd(tdb=tdb, tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
        print('Using low setpoint')
        print(f"pmv={results['pmv']}, ppd={results['ppd']}%")
        
        setpoint_high =setpoints['tmp_cmf_80_up']
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
            
        set_point = min(abs(0 - results_low), abs(0 - results_high), abs(0-results_mid))  #this setpoint will be sent to Joseph
else:
    set_point = set_point # keep the same setpoint if it is in the comfortable range no need to cause the hvac to turn on if it is comfortable






# for users who wants to use the IP system
#results_ip = pmv_ppd(tdb=77, tr=77, vr=0.4, rh=50, met=1.2, clo=0.5, units="IP")
#print(results_ip)

# import pandas as pd
# import os

# df = pd.read_csv(os.getcwd() + "/template-SI.csv")

# df['PMV'] = None
# df['PPD'] = None

# for index, row in df.iterrows():
#     vr = v_relative(v=row['v'], met=row['met'])
#     results = pmv_ppd(tdb=row['tdb'], tr=row['tr'], vr=vr, rh=row['rh'], met=row['met'], clo=row['clo'], standard="ashrae")
#     df.loc[index, 'PMV'] = results['pmv']
#     df.loc[index, 'PPD'] = results['ppd']

# print(df)
# df.to_csv('results.csv')
