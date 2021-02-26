# -*- coding: utf-8 -*-
"""
Created on Tue Feb 23 17:44:16 2021

@author: rosem
"""


from pythermalcomfort.models import pmv_ppd
from pythermalcomfort.models import adaptive_ashrae
from pythermalcomfort.psychrometrics import v_relative
from pythermalcomfort.psychrometrics import t_mrt
from pythermalcomfort.utilities import met_typical_tasks
from pythermalcomfort.utilities import clo_typical_ensembles

#test data summer case
tdb = 80# dry-bulb air temperature, [$^{\circ}$C]
tg = 82
rh = 65  # relative humidity, [%]
tout = 90 #temperature at 6am
occupancy = 1

#Held Constant for residencies
v = 0.1  # average air velocity, [m/s]

#MRT Calculation
tr = t_mrt(tg, tdb, v, d=0.075, emissivity=0.95)

#operative temp calulation
op = 0.5 * (tdb + tr)
#print(op)

met = met_typical_tasks['Seated, quiet']

if tout >= 75: #summer
    icl = 0.5 # clo_typical_ensembles('Typical summer indoor clothing') # calculate total clothing insulation
if tout < 75 & tout >= 60: #spring
    icl = 0.61 #clo_typical_ensembles('Trousers, long-sleeve sweatshirt')
if tout < 60 & tout >= 45: #fall
    icl = 0.74 #clo_typical_ensembles('Sweat pants, long-sleeve sweatshirt')
if tout < 45: #winter
    icl = 1.0 #clo_typical_ensembles('Typical winter indoor clothing')


#print(icl)

# calculate the relative air velocity
vr = v_relative(v=v, met=met)

results = pmv_ppd(tdb=tdb, tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")

#print(pmv)

# print PMV value
print('Summer worst case')
print('indoor temp')
print(tdb)
print('GT')
print(tg)
print('RH')
print(rh)
print('temp at 6am')
print(tout)
print(f"pmv={results['pmv']}, ppd={results['ppd']}%")

#need the running average of the past temperatures 
#assumed it was cooler before and is heating up at afternoon of the day
#summer
t_running_mean = 74

#Find  temperature set point

setpoints = adaptive_ashrae(tdb, tr, t_running_mean, v, units="IP")
#print(setpoints)

print(f"low={setpoints['tmp_cmf_90_low']}, high={setpoints['tmp_cmf_90_up']}%")

#checking if the change fixed it summer
tdb =setpoints['tmp_cmf_90_low']
tg = tdb + 2
tr = t_mrt(tg, tdb, v, d=0.075, emissivity=0.95)

results = pmv_ppd(tdb=tdb, tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
print('Using low setpoint')
print(f"pmv={results['pmv']}, ppd={results['ppd']}%")

tdb =setpoints['tmp_cmf_90_up']
tg = tdb + 2 
tr = t_mrt(tg, tdb, v, d=0.075, emissivity=0.95)

results = pmv_ppd(tdb=tdb, tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
print('Using high setpoint')
print(f"pmv={results['pmv']}, ppd={results['ppd']}%")

tdb = (setpoints['tmp_cmf_90_up'] + setpoints['tmp_cmf_90_low'])/2
tg = tdb + 2
tr = t_mrt(tg, tdb, v, d=0.075, emissivity=0.95)
print('mid setpoint')
print(tdb)

results = pmv_ppd(tdb=tdb, tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
print('Using mid setpoint')
print(f"pmv={results['pmv']}, ppd={results['ppd']}%")


###########################################################################
#test data winter case
tdb = 68# dry-bulb air temperature, [$^{\circ}$C]
tg = 66
rh = 30 # relative humidity, [%]
tout = 32 #temperature at 6 am - contributes to when you get dressed
occupancy = 1

#Held Constant for residencies
v = 0.1  # average air velocity, [m/s]

#MRT Calculation
tr = t_mrt(tg, tdb, v, d=0.075, emissivity=0.95)

#operative temp calulation
op = 0.5 * (tdb + tr)
#print(op)

met = met_typical_tasks['Seated, quiet']

if tout >= 75: #summer
    icl = 0.5 # clo_typical_ensembles('Typical summer indoor clothing') # calculate total clothing insulation
if tout < 75 & tout >= 60: #spring
    icl = 0.61 #clo_typical_ensembles('Trousers, long-sleeve sweatshirt')
if tout < 60 & tout >= 45: #fall
    icl = 0.74 #clo_typical_ensembles('Sweat pants, long-sleeve sweatshirt')
if tout < 45: #winter
    icl = 1.0 #clo_typical_ensembles('Typical winter indoor clothing')


# calculate the relative air velocity
vr = v_relative(v=v, met=met)

results = pmv_ppd(tdb=tdb, tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")

def get_first_key(dictionary):
    for key in dictionary:
        return key
    raise IndexError
key1 = get_first_key(results)
pmv = results[key1]

#print(pmv)

# print PMV value
print("winter test case")
print('indoor temp')
print(tdb)
print('GT')
print(tg)
print('RH')
print(rh)
print('temp at 6am')
print(tout)
print(f"pmv={results['pmv']}, ppd={results['ppd']}%")

#need the running average of the past temperatures 
#assumed it was cooler before and is heating up at afternoon of the day

#winter
t_running_mean = 68
#Find  temperature set point

setpoints = adaptive_ashrae(tdb, tr, t_running_mean, v, units="IP")
#print(setpoints)

print(f"low={setpoints['tmp_cmf_90_low']}, high={setpoints['tmp_cmf_90_up']}")

#checking if the change fixed it winter
tdb =setpoints['tmp_cmf_90_low']
tg = tdb - 2
tr = t_mrt(tg, tdb, v, d=0.075, emissivity=0.95)


results = pmv_ppd(tdb=tdb, tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
print('Using low setpoint')
print(f"pmv={results['pmv']}, ppd={results['ppd']}%")

tdb =setpoints['tmp_cmf_90_up']
tg = tdb - 2
tr = t_mrt(tg, tdb, v, d=0.075, emissivity=0.95)
           
results = pmv_ppd(tdb=tdb, tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
print('Using high setpoint')
print(f"pmv={results['pmv']}, ppd={results['ppd']}%")

tdb = (setpoints['tmp_cmf_90_up'] + setpoints['tmp_cmf_90_low'])/2
tg = tdb - 2
tr = t_mrt(tg, tdb, v, d=0.075, emissivity=0.95)
print('mid setpoint')
print(tdb)

results = pmv_ppd(tdb=tdb, tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")
print('Using mid setpoint')
print(f"pmv={results['pmv']}, ppd={results['ppd']}%")
