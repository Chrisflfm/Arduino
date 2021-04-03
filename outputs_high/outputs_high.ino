void setup() {
  // put your setup code here, to run once:
 pinMode(D5, OUTPUT);    // sets the digital pin D5 as KETEL
 pinMode(D6, OUTPUT);    // sets the digital pin D6 as LED
}

void loop() {
  // put your main code here, to run repeatedly:
digitalWrite(D5, HIGH);
digitalWrite(D6, HIGH);
delay(1000);
}
