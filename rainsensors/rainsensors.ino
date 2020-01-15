/*
  Rainsensor collector 18 sensors to 1 output whne 2 sensors are detecting rain
  Turns an LED on for one second, then off for one second, repeatedly as alive signal to controller.
  By Chris 15/01/2020  
*/

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  // initialize digital pin 44 as an output.
  pinMode(44, OUTPUT);
  // initialize pin 22 - 43 as input
  pinMode(22, INPUT);
  pinMode(24, INPUT);
  pinMode(26, INPUT);
  pinMode(28, INPUT);
  pinMode(30, INPUT);
  pinMode(31, INPUT);
  pinMode(32, INPUT);
  pinMode(33, INPUT);
  pinMode(34, INPUT);
  pinMode(35, INPUT);
  pinMode(36, INPUT);
  pinMode(37, INPUT);
  pinMode(38, INPUT);
  pinMode(39, INPUT);
  pinMode(40, INPUT);
  pinMode(41, INPUT);
  pinMode(42, INPUT);
  pinMode(43, INPUT);
 
  
}

// the loop function runs over and over again forever
void loop() {
  // read inputs
  int sensorVal1 = digitalRead(22);
  int sensorVal2 = digitalRead(24);
  int sensorVal3 = digitalRead(26);
  int sensorVal4 = digitalRead(28);
  int sensorVal5 = digitalRead(30);
  int sensorVal6 = digitalRead(31);
  int sensorVal7 = digitalRead(32);
  int sensorVal8 = digitalRead(33);
  int sensorVal9 = digitalRead(34);
  int sensorVal10 = digitalRead(35);
  int sensorVal11 = digitalRead(36);
  int sensorVal12 = digitalRead(37);
  int sensorVal13 = digitalRead(38);
  int sensorVal14 = digitalRead(39);
  int sensorVal15 = digitalRead(40);
  int sensorVal16 = digitalRead(41);
  int sensorVal17 = digitalRead(42);
  int sensorVal18 = digitalRead(43);
  // init counter
  int highCounter = 0;

  // when the input is high add 1 to counter
  if (sensorVal1 == HIGH) {
    highCounter = highCounter +1 ;
  } 

  if (sensorVal2 == HIGH) {
    highCounter = highCounter +1 ;
  } 

  if (sensorVal3 == HIGH) {
    highCounter = highCounter +1 ;
  } 

  if (sensorVal4 == HIGH) {
    highCounter = highCounter +1 ;
  } 

  if (sensorVal5 == HIGH) {
    highCounter = highCounter +1 ;
  } 

  if (sensorVal6 == HIGH) {
    highCounter = highCounter +1 ;
  } 

  if (sensorVal7 == HIGH) {
    highCounter = highCounter +1 ;
  } 

  if (sensorVal8 == HIGH) {
    highCounter = highCounter +1 ;
  } 

  if (sensorVal9 == HIGH) {
    highCounter = highCounter +1 ;
  } 

  if (sensorVal10 == HIGH) {
    highCounter = highCounter +1 ;
  } 

  if (sensorVal11 == HIGH) {
    highCounter = highCounter +1 ;
  } 

  if (sensorVal12 == HIGH) {
    highCounter = highCounter +1 ;
  } 

  if (sensorVal13 == HIGH) {
    highCounter = highCounter +1 ;
  } 

  if (sensorVal14 == HIGH) {
    highCounter = highCounter +1 ;
  } 

  if (sensorVal15 == HIGH) {
    highCounter = highCounter +1 ;
  } 

  if (sensorVal16 == HIGH) {
    highCounter = highCounter +1 ;
  } 

  if (sensorVal17 == HIGH) {
    highCounter = highCounter +1 ;
  } 

  if (sensorVal18 == HIGH) {
    highCounter = highCounter +1 ;
  } 

  // when more then 1 input is high its raining
  if (highCounter > 1) {
    digitalWrite(44, HIGH);
  } else {
    digitalWrite(44, LOW);
  }

  // blink led output for alive signal 1 Hz
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(500);                       // wait for a second
}
