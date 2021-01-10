// informatie:
// http://www.esp8266learning.com/wemos-d1-esp8266-based-board.php
// https://components101.com/wireless/esp8266-pinout-configuration-features-datasheet
// https://www.addicore.com/D1-DevBoard-p/aD286.htm
// https://uc806aae2d18cf758f8ba76518d1.dl.dropboxusercontent.com/cd/0/inline2/BF9yEz6sIsHM9vme9zQ8RShLkSgNQPLYeopJQR-j-m1w9GqcDF30GrCAGV2rH8OA0xr1RE-wSdaHvOglP9USAGcTgqxsp4lRkwkN8s9sWkeHGz3RmEoKLGuUBKUQ3N3va3eQj201n4yCgdZ-SqM51CTDvPWT5yESVgVBKFxbu9-tPRqzaoSjYayFdc7_9lRtBxhuAdgHA2tBVz4QRNv55CmYxsA4nRURdVoqpAfMFSKO6N7Ag7-fKBd6gSHruJ8FQNem5WdjqiDKRp64EhrrwH7335KPZAXXnTKL2f30e23Va3ScERr3z4Hjee8rdGa2aC9_hwnkAb67D9hNLMo95oPh/file
// https://uc6299d5e11d4bea5565a586aa47.dl.dropboxusercontent.com/cd/0/inline2/BF_7lOPOSVU5A-_KUrVNlNjILJyjz4BV_4hEALFaAWu9SSpPXlvF8-VR8NP-ObekPFW7fdjHswiMIc8DVlQwe77JcM1KimG6lsB1kccAnVLl8Y3WoGEVQVe48vyo7UJM7ARL6izpm3QQO0-2nhguFTsClnQtZ5G3yG8O_PDxZTnAHGTYrDUNNJp8It0Zd6Uy3U5OcUtqLeL1Ox96fAnjO7skrL_dgQqPBp5F3LO6VkTbax8_IJUf1vo6q-gkVviChSkkW0R7YyIG-4ULOtr-1efo-3sI9gex5EUvj-dkM__jK_IL68Or2FCzX9MtUYz0ox9NORST3r5BHLs8k2j43gdR/file
// https://averagemaker.com/2018/04/how-to-set-up-wifi-on-a-wemos.html
// https://www.digikey.com/en/maker/blogs/2018/how-to-use-basic-mqtt-on-arduino

#include <Pinger.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <elapsedMillis.h>


// global var
bool resetting = false;
WiFiClient WIFI_CLIENT;
int period = 150;// 150;//150 seconden = 2.5 min
int period2 = 60;// 60;//60 seconden = 1 min
float rest;
int LedState;
int ResetCounter = 1;
int LoopCounter = 0;
int FirstLoop = 0;
int voltage;
int AnaIn;
int duur;
int ResetSpanning = 210;
int BedrijfsSpaning = 90;
int WaitKetelUit = 300;//hoe lang nul volt voordat we zeggen vlam uit
int KetelUit = WaitKetelUit;
char buffer[12];         //the ASCII of the integer will be stored in this char array
int Status;
// Function prototypes
void subscribeReceive(char* topic, byte* payload, unsigned int length);

// Ethernet and MQTT related objects
EthernetClient ethClient;
PubSubClient mqttClient(ethClient);
Pinger pinger;
int pingResult;

// Network SSID
const char* ssid = "WiFi-2.4-09A8 Accespoint";
const char* password = "d95uGXkD72jY";

// Make sure to leave out the http and slashes!
const char* server = "192.168.1.43";

void wait(int seconds)
{
  elapsedMillis waiting;     // "waiting" starts at zero
  waiting = 0; 
    while (waiting < 1000 * seconds){ 
      //Serial.println(waiting);
      yield();
    }
    waiting = 0; 
}

void subscribeReceive(char* topic, byte* payload, unsigned int length)
{
  // Print the topic
  Serial.print("Topic: ");
  Serial.println(topic);
 
  // Print the message
  Serial.print("Message: ");
  for(int i = 0; i < length; i ++)
  {
    Serial.print(char(payload[i]));
  }
 
  // Print a newline
  Serial.println("");
}
 
void setup() {
  Serial.begin(9600);
  delay(10);
  
  // Connect WiFi
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  WiFi.hostname("Name");
  Serial.println(ssid);
  Serial.println("WiFi connected");
  Serial.println("");
 
  // Print the IP address
  Serial.print("my IP address: ");
  Serial.print(WiFi.localIP());
  Serial.println("");
  // test WiFi
  pingResult = pinger.Ping("192.168.1.43");

  if (pingResult >= 0) {
    Serial.print("PING SUCCESS! RTT = ");
    Serial.print(pingResult);
    Serial.println(" ms");
  } else {
    Serial.print("PING FAILED! Error code: ");
    Serial.println(pingResult);
  }
 

 mqttClient.setServer("192.168.1.43", 1883);
 mqttClient.setClient(WIFI_CLIENT);
 mqttClient.connect("arduino-1");

 while (!mqttClient.connected()) {
  Serial.print("Connecting to MQTT...");
  Serial.print("connection state: ");
  Serial.println(mqttClient.state());
  delay(300);
  mqttClient.connect("arduino-1");
 }
  if (mqttClient.connect("arduino-1")) {
    // connection succeeded
    Serial.println("Connected ");
    
  } 
  else {
    // connection failed
    // mqttClient.state() will provide more information
    // on why it failed.
    Serial.println("Connection failed ");
  }

  // IO config:
 pinMode(A0, INPUT);
 pinMode(D5, OUTPUT);    // sets the digital pin D5 as KETEL
 pinMode(D6, OUTPUT);    // sets the digital pin D6 as LED
 Serial.println("Starting loop");
 
}

void loop() {
          // This is needed at the top of the loop!
          mqttClient.loop();
          // Ensure that we are subscribed to the topic "MakerIOTopic"
          if (FirstLoop < 1){
            Status = 0;
            ResetCounter = 1;
            LoopCounter = 0;
            mqttClient.subscribe("CVSpanning");
            mqttClient.subscribe("RestTijd");
            mqttClient.subscribe("CVStatus");
            mqttClient.subscribe("KetelUit");
            FirstLoop = 1;
            mqttClient.publish("CVStatus", "Booting");
          }
          AnaIn = analogRead(A0);
          voltage = (AnaIn * 0.3225) - 90.042;
          if (voltage < 0){voltage = 0;}
          Serial.print("Spanning: ");
          Serial.println(voltage);
          itoa(voltage,buffer,10); //(integer, yourBuffer, base)
          mqttClient.publish("CVSpanning", buffer );

         //detectie ketel uit
         if (voltage == 0 && Status == 3){
          KetelUit = KetelUit + 1; 
         }
         if (voltage > 0){
          KetelUit = 0;
         }
         //Restart   
         if (Status == 2){
                digitalWrite(D5, HIGH);
                LoopCounter = LoopCounter + 1;
                rest = duur - LoopCounter;///60.0;//in minuten
                mqttClient.publish("CVStatus", "Restarting");
                itoa(rest,buffer,10); //(integer, yourBuffer, base)
                mqttClient.publish("RestTijd", buffer);
                if (rest <= 0){
                      Status = 3;
                      LoopCounter = 0; 
                }
         }
         
         //Wachten voor restart
          if (Status == 1 ){
                LoopCounter = LoopCounter + 1;
                rest = duur - LoopCounter;///60.0;//in minuten
                itoa(rest,buffer,10); //(integer, yourBuffer, base)
                mqttClient.publish("RestTijd", buffer);
                if (rest <= 0){
                      Status = 2;
                      LoopCounter = 0; 
                      duur = period2;
               }
         } 
          
          // Veiligheids blok reset
          if ((Status == 0 || Status == 3) && voltage > ResetSpanning){
              digitalWrite(D5, LOW);
              digitalWrite(D6, HIGH);
              mqttClient.publish("CVStatus", "Resetting");
              duur = period * ResetCounter;
              ResetCounter = ResetCounter + 1;
              LoopCounter = 0;
              Status = 1;
          }    
          
         
         //normal oparation
          if (Status == 3 ){
               digitalWrite(D6, LOW); 
               digitalWrite(D5, HIGH);
               mqttClient.publish("CVStatus", "Normal operation");
               ResetCounter = 1;
          }
         //Flame out
          if (KetelUit >= WaitKetelUit ){
                     //digitalWrite(D6, LOW); 
                     mqttClient.publish("CVStatus", "Burner off");
                     ResetCounter = 1;
                     Status = 0;
          }
         //LED knipperen
         if (Status >0 && Status <= 2){
              if (LedState > 0){
                  digitalWrite(D6, LOW);
                  LedState = 0;
                }
              else {
                  digitalWrite(D6, HIGH);
                  LedState = 1;
                }
        }
         // Dont overload the server!
         int countDouwnKetelUit = WaitKetelUit - KetelUit;
         if (countDouwnKetelUit < 0){
                countDouwnKetelUit = WaitKetelUit;
         }
         itoa(countDouwnKetelUit,buffer,10); //(integer, yourBuffer, base)
         mqttClient.publish("KetelUit", buffer);
         Serial.print("Status: ");
         Serial.println(Status);
         wait(1);
}
