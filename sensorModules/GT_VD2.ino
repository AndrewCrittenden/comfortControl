#include "Arduino.h"

int analogpin = A4;

const long num_values = 10000;
//AverageValue<long> averageValue(num_values);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  analogReadResolution(12);
  analogReference(AR_INTERNAL1V65);
  pinMode(analogpin, INPUT);

}

void loop() {
  // code that will run when pinged by central interface
  
  int sum = 0;
  int num_readings = 0;
  double avg_raw = 0;
  for (uint32_t i = 0; i < num_values; i++){ // code to average and increase accuracy, serial read in very fast so it can do a lot of points really quickly
    //Serial.println(sum); //check to see how many data readings it was taking
    int analog = analogRead(analogpin);
    sum +=  analog; 
    num_readings += 1;
       
  }
  avg_raw = sum/(num_readings);

  Serial.print("raw: ");
  Serial.println(avg_raw);
  double voltage = avg_raw * (1.65 / 4095) ; //12bit ADC with an interal voltage reference to increase accuracy
  Serial.print("volts: ");
  Serial.println(voltage);
  double rt = (1844 * voltage) / (3.2 - voltage) + 21.9; //measured values of the resistance and voltage supplied instead of nominal
                                                          // includes the +21.9 as a shift for the rt to gt formula. The measured resistance at
                                                          //68 degrees is 21.9 different on this particular gt sensor than in the manual
  Serial.print("GT res: ");
  Serial.println(rt);
  double gt = 0.2585 * rt - 258.6; // given in manual 
  Serial.print("GT: ");
  Serial.println(gt);
  
  delay(30000); // collects data about every 30 secs

  //Tylers code on the API addressing to be added
}
