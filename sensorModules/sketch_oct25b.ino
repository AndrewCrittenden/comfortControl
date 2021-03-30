//Code for Sensior Design tempertue and occupancy sensors
//Cameron Wallace
const int sens = 14;
double Room_temp = 298.15;
double Resistor_temp = 100000;
double beta = 3950;
double rThermistor = 0;
double Balance_res = 100000;
int adcSamples[10];
double adcAvg = 0;
#include <math.h> //for log()

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  }

//Read inputs
void loop() {
  double val;
  double temp;
  for (int i=0;i<10;i++) {
    adcSamples[i] = analogRead(sens);
    delay(10);
  }
  for (int i=0;i<10;i++) {
    adcAvg += adcSamples[i];
  }
  adcAvg /= 10;
  
  rThermistor = Balance_res * ((1023/adcAvg)-1);
  temp = (beta*Room_temp)/(beta+(Room_temp*log(rThermistor/100000)));
  temp = temp - 273.15;
  Serial.println(temp);
  delay(1000);
}
