import os
import random
import time
from watchdog.observers import Observer
from watchdog.events import PatternMatchingEventHandler
from pythermalcomfort.models import pmv_ppd
from pythermalcomfort.psychrometrics import v_relative
from pythermalcomfort.psychrometrics import t_mrt
from pythermalcomfort.utilities import met_typical_tasks
from pythermalcomfort.models import clo_tout

#Default initial value, updated when GUI runs
desiredTemp = "72"
activity = "resting"

# IPCsendGui function sends sensor reading to the GUI C++ code.
# Expects the arguements in order as float or double
# occupancy is the only exception it must be bool (True or False) 
def IPCsendGui(indoorTemp, outdoorTemp, absHumidity, relHumidity, globeTemp, occupancy, pmv):
    print("Sending to GUI")
    path= "/home/pi/WA/comfortControl/fifo/comfortToGui.fifo"
    try:
        os.mkfifo(path)
    except FileExistsError:
        pass
    f = os.open(path, os.O_NONBLOCK|os.O_RDWR)
    toSend = str(indoorTemp)+','+str(outdoorTemp)+','+str(absHumidity)+','+str(relHumidity)+','+str(globeTemp)+','+str(occupancy)+','+str(pmv)+',\n'
    os.write(f,str.encode(toSend))

class IPCreceiveGui(PatternMatchingEventHandler):
    def on_modified(self, event):
        global desiredTemp, activity
        super(IPCreceiveGui, self).on_modified(event)
        with open("/home/pi/WA/comfortControl/fifo/guiToComfort.fifo", 'r') as f:
            buf = f.read()
        listVal = buf.split(',')
        desiredTemp = listVal[0]
        activity = listVal[1]

def get_first_key(dictionary):
    for key in dictionary:
        return key
    raise IndexError
#TODO Rosemary see this function of where to put your code
# Also I have been using 4 spaces for tabs, look out for python errors with indentation level between out scripts
def comfortAnalysis(tdb, tg, rh, tout, occupancy):
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
    if activity == "resting":
        met = met_typical_tasks['Seated, quiet']    
    if activity == "moderately active":
        met = met_typical_tasks['Walking about']  
    if activity == "active":
        met = met_typical_tasks['Calisthenics']
        if tout < 50:
            clo = .5 
        else:
            clo = 0.36 #workout clothes

    #Held Constant for residencies
    v = 0.1  # average air velocity, [m/s]
    #MRT Calculation
    tr = t_mrt(tg, tdb, v, d=0.075, emissivity=0.95) #print to terminal
    #operative temp calulation
    op = 0.5 * (tdb + tr)
    #print(op)

    # calculate the relative air velocity
    vr = v_relative(v=v, met=met)
    # calculate PMV in accordance with the ASHRAE 55 2017
    results = pmv_ppd(tdb=tdb, tr=tr, vr=vr, rh=rh, met=met, clo=icl, standard="ASHRAE", units="IP")

    key1 = get_first_key(results)
    pmv = results[key1]
    return pmv

    # print PMV value
    # print(f"pmv={results['pmv']}, ppd={results['ppd']}%")


# main initializes sample comfort code and runs test to send and receive from GUI
def main():
    print("Setup")
    #Setup watchdog to handle receive from GUI event
    fifoFile = "/home/pi/WA/comfortControl/fifo/guiToComfort.fifo"
    fifoDir = os.path.split(fifoFile)[0]
    observer = Observer()
    handler = IPCreceiveGui([fifoFile])
    observer.schedule(handler, fifoDir, recursive=True)
    observer.start()
    print("mid")
    try:
        #Main control loop of comfort analysis script
        while(1):
            #Retrieve sensor data
            #TODO Replace this with acutal sensor data
            indoorTemp = 70 + random.randint(-500,500)/100
            outdoorTemp = 32 + random.randint(-500,500)/100
            absHumidity = 38 + random.randint(-500,500)/100
            relHumidity = 40 + random.randint(-500,500)/100
            globeTemp = 68 + random.randint(-500,500)/100
            occupancy = random.choice([True,False])
            print("Using desiredTemp = "+desiredTemp, " activity = ", activity)
            #calculate pmv
            pmv = comfortAnalysis(indoorTemp, globeTemp, relHumidity, outdoorTemp, occupancy)
            print(indoorTemp, " ", outdoorTemp, " ", absHumidity, " ", relHumidity, " ", globeTemp, " ", occupancy, " ", pmv)
            IPCsendGui(indoorTemp, outdoorTemp, absHumidity, relHumidity, globeTemp, occupancy, pmv) 
            time.sleep(4)
    except KeyboardInterrupt:
        observer.stop()
    observer.join()

    #IPCsendGui(70, 32, 38, 40, 68, True, 0) 

main()
