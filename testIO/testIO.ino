void setup() {
  // put your setup code here, to run once:
 pinMode(A0, INPUT);
 //pinMode(8, OUTPUT);    // sets the digital pin D1 as output
 pinMode(D5, OUTPUT);    // sets the digital pin D2 as output
 pinMode(D6, OUTPUT);    // sets the digital pin D2 as output
 pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
 Serial.begin(9600);
 Serial.println("Starting loop");
 
}

void loop() {
  // put your main code here, to run repeatedly:
   static int counter = 0;
  
  digitalWrite(D6, HIGH);
  digitalWrite(LED_BUILTIN, LOW);
  delay(2000);
  counter = counter +1;
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println(counter);
  digitalWrite(D6, LOW);
  
  delay(2000);
 
}
