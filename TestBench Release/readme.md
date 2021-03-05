# Test Bench -- Latest Release
COMFORT Control: Team 11 from University of Kentucky EE491 Spring 2021 Semester
##### ENSURE TO LET IT THROUGH YOUR FIREWALL & SMARTSCREEN -- It is a false positive due to lack of professional certificates!

![alt text][logo]

[logo]: https://github.com/AndrewCrittenden/comfortControl/raw/main/TestBench%20Release/image.PNG "Logo Title Text 2"

## Implements our designed framework in an easy to use Windows application 

Emulates the Raspberry Pi server's end of the handshaking protocol 

## Features: 

Authenticate – Implements the authentication process over the computer Wi-Fi adapter's subnet mask, can authenticate all devices within a 20 second timeout period 

Request Sensor Data – Begins the data request query over the TCP connection of all nodes, and retrieves their sent sensor data (unique to each sensor) 

Clear Devices – Clears Device List 

Start Auto – Begins automatically querying the nodes every 20 seconds, simulating the server (with a shorter time period between queries for testing purposes) 

Change Output Val – Sends information to the heating/cooling device, changing its value 

Choose-WiFi-Adapter – Due to more than one adapter being on a PC, this allows you to select the adapter you're using for this project 

Save/Clear Logs – Does things to the log on the right 

## Getting Started:

1) Power on node device and wait for it to give the "Heartbeat: ..." output in the Arduino console.
2) You may need to press the Choose-WiFi-Adapter button to select the correct network adapter if you have more than one. If it doesn't work, make sure to check this.
3) Press the Authenticate button. The device should show up in the table below the buttons after a short delay.
4) Input nodes can skip this step. Output nodes should press the Change Output Val button and set the desired output value.
5) Press the Request Sensor Data button. Input node data should appear in the console, data will be sent to output nodes.

You may press the Start Auto button if desired. This will cause a request for sensor data/output to nodes every 20 seconds, similar to the real server.


 
### Devices should appear with their information in the list if validated properly: 

###### If a bug appears, let me know and I can fix it ASAP
