from pythermalcomfort.models import pmv_ppd
from pythermalcomfort.psychrometrics import v_relative
from pythermalcomfort.psychrometrics import t_mrt
from pythermalcomfort.utilities import met_typical_tasks
from pythermalcomfort.models import clo_tout

# Environmental Conditions From Sensor Subsystem
tdb = 75# dry-bulb air temperature, [$^{\circ}$C]
tg = 76
rh = 50  # relative humidity, [%]
tout = 40
occupancy = 1

#clothing level prediction
icl = clo_tout(tout, units= "IP") # calculate total clothing insulation
print(icl)
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
tr = t_mrt(tg, tdb, v, d=0.15, emissivity=0.95)
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
