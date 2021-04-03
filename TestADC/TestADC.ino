//info
// Intern 3.3/1 spannings deler maakt dat op pin A0 max3.3V mag en dat die dan gelezen word
//volgens https://forum.arduino.cc/index.php?topic=649032.0
// moet je per volt boven 3.3 100K in serie met A0 zetten (voor 10V --> 680K)
// ook wordt er velmeld dat je niet tegelijk de Wifi en de ADC kan gebruiken
//Delay laat ook de WiFi slapen zodat die de ADC niet kan gebruiken tijdens de wachttijd
//gebruik millis() om de loop tepauzeren om de WiFi de tijd te geven om zijn ding te doen.

// global var
bool resetting = false;
int WaitMultiplyer = 0;
unsigned long  myTime = 0;
unsigned long  Counter = 0;
// maak een NOP operatie
//#define NOP __asm__ __volatile__ ("nop\n\t")
//#define NOP Counter = Counter + 1


//while (millis() < myTime + 5000) {
//    // do something repetitive 200 times
//      NOP; // delay 62.5ns on a 16MHz AtMega
//      }

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(A0, INPUT);
  pinMode(D2, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  float voltage = analogRead(A0) * (3.3 / 1023);
  Serial.println(voltage);
   // Veiligheids blok uit
  if (voltage < 2) {
  digitalWrite(D2, LOW);
  }
   // Veiligheids blok reset
  if (voltage >= 3 && !resetting) {
    Serial.println("Error gedetecteerd, resetting");
    resetting = true;
    WaitMultiplyer = 1;
  while(resetting){
    digitalWrite(D2, LOW);
    Serial.print("Veiligheidsblok uit wacht: ");
    Serial.print(5 * WaitMultiplyer);
    Serial.println(" seconden");
    delay (5000 * WaitMultiplyer);
    digitalWrite(D2, HIGH);
    Serial.println("veiligheidsblok aan");
    delay (5000);
    voltage = analogRead(A0) * (3.3 / 1023);
    Serial.println(voltage);
  if (voltage >= 2 && voltage <= 3 ) {
    resetting = false;    
    Serial.println("Reset gelukt");    
      }
  else {
    Serial.println("Reset faalt wachttijd verhoogd");
    WaitMultiplyer = WaitMultiplyer + 1;
  }
  }
  }
  // Veiligheids blok aan
  if (voltage >= 2 && voltage <= 3 && !resetting) {
    digitalWrite(D2, HIGH);
  }
delay (5000);
}
