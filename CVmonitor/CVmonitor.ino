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
int period = 600;// 150;//150 seconden = 2.5 min
int period2 = 600;// 60;//60 seconden = 1 min
float rest;
int LedState;
int InReset = 0;
int ResetCounter = 1;
int LoopCounter = 0;
int FirstLoop = 0;
int voltage;
int AnaIn;
int duur;
int ResetSpanning = 200;
int BedrijfsSpaning = 90;
int WaitKetelUit = 120;//hoe lang nul volt voordat we zeggen vlam uit
int KetelUit = WaitKetelUit;
int SpanningPreReset = ResetSpanning;
int ForceOff = 120;//na deze tijd met brander aan ga ik er vanuit dat die detectie niet klopt en doe ik een reset
int ForceOffCounter = 0;
char buffer[12];         //the ASCII of the integer will be stored in this char array
int Status;
int Delco = 0;
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
  String strTopic(*topic);
  if (strTopic == "r"){
    Serial.println("topic ontvangen");
    if (payload[0] =='1'){
      Serial.println("Reset ontvangen");
      Serial.println(payload[0]);
      digitalWrite(D5, LOW);
      mqttClient.publish("CVStatus", "Manueel rust");
      InReset = 1;
    }
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
 mqttClient.setCallback(subscribeReceive);
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
         if (FirstLoop < 1){
            Status = 0;
            ResetCounter = 1;
            LoopCounter = 0;
            FirstLoop = 1;
            digitalWrite(D5, HIGH);
            mqttClient.subscribe("reset");
            mqttClient.publish("CVStatus", "Brander vrij wacht op vraag");
            mqttClient.publish("ForceOffCounter","-");
            mqttClient.publish("RestTijd","-");
            mqttClient.publish("KetelUit", "-" );
          }
          //Delco 0 Ain
          //Delco 1 rust
          //Delco 2 MQTT
          //Delco 3 rust
                   
          if (Delco==0){ 
              AnaIn = analogRead(A0);
              voltage = (AnaIn * 0.3225) - 90.042;
              if (voltage < 0){voltage = 0;}
              //Serial.print("Spanning: ");
              //Serial.println(voltage);
          }
          //Serial.print("Status: ");
          //Serial.println(Status);
          itoa(voltage,buffer,10); //(integer, yourBuffer, base)
          if (Delco==2){mqttClient.publish("CVSpanning", buffer );}
          //itoa(SpanningPreReset,buffer,10); //(integer, yourBuffer, base)
          //if (Delco==2){mqttClient.publish("SpanningPreReset", buffer );}
         
         //Teller detectie ketel uit
         if (voltage == 0 && Status > 0){
          KetelUit = KetelUit + 1; 
          
         }
         if (voltage > 0){
          // ketel is niet meer uit
          KetelUit = 0;
         }
         itoa(WaitKetelUit - KetelUit,buffer,10); //(integer, yourBuffer, base)
         if (Delco==2 && Status == 2){mqttClient.publish("KetelUit", buffer );}
         if (Delco==2 && Status != 2){mqttClient.publish("KetelUit", "-" );}
         
         //Brandtijd   
         if (Status == 2){
                LoopCounter = LoopCounter + 1;
                rest = duur - LoopCounter;///60.0;//in minuten
                if (Delco==2 && InReset == 0){mqttClient.publish("CVStatus", "Brander vrij periode");}
                itoa(rest,buffer,10); //(integer, yourBuffer, base)
                if (Delco==2){mqttClient.publish("RestTijd", buffer);}
                if (rest <= 0){
                      if (Delco==2){mqttClient.publish("CVStatus", "Brander vrij");}
                      digitalWrite(D6, LOW);
                      Status = 3; //cool down
                      LoopCounter = 0; 
                      InReset = 0;
                }
         }
         
         //Rusttijd
          if (Status == 1 ){
                digitalWrite(D6, LOW); 
                LoopCounter = LoopCounter + 1;
                rest = duur - LoopCounter;///60.0;//in minuten
                itoa(rest,buffer,10); //(integer, yourBuffer, base)
                if (Delco==2){mqttClient.publish("ForceOffCounter", buffer);}
                if (rest <= 0){
                      Status = 2;
                      LoopCounter = 0; 
                      duur = period2;
                      SpanningPreReset = voltage;
                      if (SpanningPreReset < BedrijfsSpaning){
                        SpanningPreReset = ResetSpanning;
                      }
                      ForceOffCounter = 0;
                      Status = 0;//Wacht op ontsteken
                      digitalWrite(D5, HIGH);
               }
         } 
          
          //Ontsteken
          if (Status == 0 && voltage > BedrijfsSpaning){
              digitalWrite(D6, HIGH);
              digitalWrite(D5, HIGH);
              duur = period * ResetCounter;
              ResetCounter = ResetCounter + 1;
              LoopCounter = 0;
              mqttClient.publish("ForceOffCounter","-");
              InReset = 0;
              Status = 2;
          }    
          
         
         //Cool down
          if (Status == 3 ){
               digitalWrite(D6, LOW); 
               digitalWrite(D5, LOW);
               mqttClient.publish("CVStatus", "Afkoel periode");
               mqttClient.publish("RestTijd", "-");
               ResetCounter = 1;
               Status = 1;//Rusttijd
           } 
         //Flame out
          if (Status > 0 && KetelUit >= WaitKetelUit ){
                     ResetCounter = 1;
                     Status = 0;
                     LoopCounter = 10000;//als ketel ketel uit alles terug naar start positie
                     mqttClient.publish("ForceOffCounter","-");
                     mqttClient.publish("RestTijd","-");
                     mqttClient.publish("CVStatus", "Brander uit");
                     SpanningPreReset = ResetSpanning;
          }
         //LED knipperen
         if (Status == 1){
              if (LedState > 0){
                  digitalWrite(D6, LOW);
                  LedState = 0;
                }
              else {
                  digitalWrite(D6, HIGH);
                  LedState = 1;
                }
        }
         Delco = Delco + 1;
         if (Delco > 3){Delco = 0;}
         wait(1);
         yield();
}
