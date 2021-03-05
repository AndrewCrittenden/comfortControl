# Client COMFORTController API/Framework for Arduino/Adafruit ItsyBitsy + AirLift ESP32

## Getting Started
1. Install the libraries specified below in "Required dependencies"
2. Uncomment your node type's #define statement
3. Verify that the struct containing your data you are sending is correct (responseData)
4. Check your pins, ensure the pin numbers match what you are using on the Itsybitsy
5. Write code as per the API. DO NOT EDIT ANYTHING OUTSIDE OF IT.
6. "COMFORTSetup() acts like Arduino's Setup(), TimerLoop() acts like Loop(). See API.
7. Any further questions, please let Tyler know on Microsoft Teams.


## Required Dependencies
Instructions on how to install libraries/boards can be found here: https://learn.adafruit.com/introducing-adafruit-itsybitsy-m4/using-with-arduino-ide
Instructions to install libraries based on a zip you downloaded (i.e. WiFiNiNa) can be found here: https://learn.adafruit.com/adafruit-airlift-breakout/arduino
Add the Adafruit json to the Additional Boards Manager URL, as found on this link: https://learn.adafruit.com/introducing-adafruit-itsybitsy-m4/setup

1. Modified version of WiFiNiNa, found here: https://github.com/adafruit/WiFiNINA/archive/master.zip
2. Arduino SAMD Boards: Library Manager -> Search "Arduino SAMD" -> "Arduino SAMD Boards (32-bits ARM Cortex-M0+)
3. Adafruit SAMD Support: Library Manager -> Search "Adafruit SAMD" -> "Adafruit SAMD Boards"
4. Crypto: Library Manager -> Search "Crypto" -> "Crypto" by "Rhys Weatherley rhys.weatherley@gmail.com"
5. micro-ecc: Library Manager -> Search "micro-ecc" -> "micro-ecc" by Kenneth MacKay

## API

### Common:
void setupCOMFORT() {} -- Used similar to setup, this code will run once before everything begins looping/operating

#define LOOP_TIMER # -- Amount of seconds before the function onTimerLoop should run again

int onTimerLoop() {} -- Returns number of seconds to wait, optional function that runs regularly

void onDataEdge() {} -- Runs after each time the data in sent (useful for sensors that only calculate one time and then send information)
### Input Nodes:

responseData onDataRequest() {} -- Populate struct members here to pass to the transmission function (i.e. your data inside your struct responseData)

### Output Nodes:

void onDataReceived() {} -- Code that runs after receiving a new data packet from server, incoming parameter is the struct with your information

## Example Wiring
![alt text][logo]

[logo]: https://github.com/AndrewCrittenden/comfortControl/raw/main/ClientCOMFORT/image.PNG "Logo Title Text 2"


