int Vo = A0;
bool out;
int Distance = 500;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int IR;
  IR = analogRead(Vo);

  //Distance to be changed when implementing
  if (IR < Distance) {
    out = true;
    Serial.println(out);
  }
  
  // debug
  Serial.println(IR);
  
  delay(100);
}
