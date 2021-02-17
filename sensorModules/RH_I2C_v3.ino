#include <Wire.h>
#include "SHTSensor.h"
const long num_values = 500;
SHTSensor sht;
void setup() { // this  is the set up code that needs to run once
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(9600);
  delay(1000); // let serial console settle
  if (sht.init()) {
    Serial.print("init(): success\n");
  } else {
    Serial.print("init(): failed\n");
  }
}
void loop() { // this loop can run when a ping is sent or it can run and then wait until a ping is received
  
  int sum = 0;
  int num_readings = 0;
  double avg_rh = 0;
  for (uint32_t i = 0; i < num_values; i++) { // for loop to average data points to make the rh sensor more accurate and stable
    //Serial.println(i);
    //Serial.println(sum); //check to see how many data readings it was taking
  sht.readSample();
  double RH = sht.getHumidity();
    sum +=  RH;
    num_readings += 1;

  }
  avg_rh = sum / (num_readings);
  Serial.print("RH: ");
  Serial.println(avg_rh); //avg_rh sent to the board
  delay(1000);

  // code to receive ping from the centralized interface needs to be added

}
