/*
  AD convertor voor MQ-135 Gas Sensor Module
  MQ-135 Gas Sensor Module heeft analog out RBPI heeft geen analog in
  Arduino als AD convertor
  By Chris 28/12/2020  
*/
void setup() {
  // put your setup code here, to run once:
  // initialize pin 22 - 43 as input
  pinMode(A0, INPUT);
  Serial.begin(9600);

}

void loop() {
 // put your main code here, to run repeatedly:
 // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V) by " * (5.0 / 1023.0)":
  float voltage = sensorValue;
  // print out the value you read:
  Serial.println(voltage);

}
