# comfortControl
COMFORT Control: Team 11 from University of Kentucky EE491 Spring 2021 Semester

## Abstract
COMFORT Controller is a system designed to measure environmental conditions of a space and control HVAC equipment to meet American Society of Heating, Refrigerating and Air-Conditioning Engineers (ASHRAE) Comfort Standards.
Design constrained to a proof of concept with a smaller demonstration module due to lack of access to a physical room and HVAC system.

## To Operate the Comfort Controller
1) Plug in and turn on the provided Router
2) Plug in the Raspberry Pi to a separate outlet
3) Plug in the white extension cord to turn on the Demonstration module and all the sensors, ensure the red switch is flipped on and shining a light. Ensure sensors operation by checking for purple light off each microcontroller. Ensure Demonstration module is powered by checking the fan mounted to the back of the fridge. 
4) Start the code by opening a terminal and running the following commands
    cd ~/WA/comfortControl/gui
    qmake
    make
 	  ./gui
5) Wait approximately 15 seconds for the nodes to connect
6) Click the sensor button in bottom left corner
7) click Authenticate button in top right corner
8) If all nodes do not switch from "disconnected" to "connected", wait 15 more seconds, and try again
9) View all sensor measurements and PMV calculations at this window or return to home window to change the activity level input.
10) If a node disconnects reboot that sensor via unplugging and replugging in the sensor module via the labelled cord in the back of the fridge
11) View data saved at ~/WA/comfortControl/gui/data.csv, rename and move to separate folder to ensure it is not overwritten during the next operation of device. 
