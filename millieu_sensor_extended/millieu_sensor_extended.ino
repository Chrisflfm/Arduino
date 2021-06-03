#include <OneWire.h>
#include <DallasTemperature.h>
#include <Pinger.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <elapsedMillis.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
 
#define ONE_WIRE_PIN D3
// DEFINE HERE THE KNOWN NETWORKS
const char* KNOWN_SSID[] = {"WiFi-2.4-09A8 Accespoint", "WiFi-2.4-09A8"};
const char* KNOWN_PASSWORD[] = {"d95uGXkD72jY", "d95uGXkD72jY"};
const int   KNOWN_SSID_COUNT = sizeof(KNOWN_SSID) / sizeof(KNOWN_SSID[0]); // number of known networks

OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWire);
DeviceAddress Thermometer;

// Network SSID
const char* ssid = "WiFi-2.4-09A8 Accespoint";
const char* ssid2 = "WiFi-2.4-09A8";
const char* password = "d95uGXkD72jY";

// global var 
int NetwerkModus = 0; //0 geen netwerk, 1 1WIfi SSID,2 meerdere SSID
int deviceCount = 0;
float tempC1;
float tempC2;
char buffer[12];         //the ASCII of the integer will be stored in this char array
int beat = 0;
int connectionAtemped = 0;
int loopCounter = 0;
Adafruit_BME680 bme; // I2C
WiFiClient WIFI_CLIENT;
int period = 600;// 150;//150 seconden = 2.5 min
int FirstLoop = 0;
int mobSensHoldPreBreath = 140;
int mobSensBreath = 30;
int mobSensHoldPostBreath = 1;
int Flush = 1;
int mobSensBaseTimeMultiplier = 1;

// Function prototypes
void subscribeReceive(char* topic, byte* payload, unsigned int length);

// Ethernet and MQTT related objects
EthernetClient ethClient;
PubSubClient mqttClient(ethClient);
Pinger pinger;
int pingResult;


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
  Serial.println("incomming MQTT");
  Serial.print("Topic: ");
  Serial.println(topic);
 
  // Print the message
  Serial.print("Message: ");
  for(int i = 0; i < length; i ++)
  {
    Serial.print(char(payload[i]));
  }
  String strTopic(*topic);
  if (strTopic == "a"){
    Serial.println("mobSensHoldPreBreath ontvangen");
    mobSensHoldPreBreath = payload[0]-48;
    Serial.println(mobSensHoldPreBreath);
  }
   if (strTopic == "b"){
    Serial.println("mobSensBreath ontvangen");
    mobSensBreath = payload[0]-48;
    Serial.println(mobSensBreath);
  }
  if (strTopic == "c"){
    Serial.println("mobSensHoldPostBreath ontvangen");
    mobSensHoldPostBreath = payload[0]-48;
    Serial.println(mobSensHoldPostBreath);
  }
  if (strTopic == "d"){
    Serial.println("mobSensBaseTimeMultiplier ontvangen");
    mobSensBaseTimeMultiplier = payload[0]-48;
    Serial.println(mobSensBaseTimeMultiplier);
  }
 
  // Print a newline
  Serial.println("");
  
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("Sens1")) {
      Serial.println("connected");
      connectionAtemped = 0;
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      connectionAtemped = connectionAtemped +1;
      if (connectionAtemped > 3){
        Serial.println("Atempting restart");
        ESP.restart();
      }
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
      //mqttClient.subscribe("aap");
      //mqttClient.subscribe("beer");
      //mqttClient.subscribe("cat");
}

 
void setup() {
  boolean wifiFound = false;
  int i, n;
 Serial.begin(115200);
 delay(10);
 // put your setup code here, to run once:
  pinMode(A0, INPUT);
  //pinMode(vent_pin, OUTPUT);
  pinMode(D5, OUTPUT);    // sets the digital pin D5 vent
  Serial.begin(115200);
  while (!Serial);
  Serial.println(F("BME680 async test"));

  if (!bme.begin()) {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
  
  delay(10);

// ----------------------------------------------------------------
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  // ----------------------------------------------------------------
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  Serial.println("Setup done");

  // ----------------------------------------------------------------
  // WiFi.scanNetworks will return the number of networks found
  // ----------------------------------------------------------------
  Serial.println(F("scan start"));
  int nbVisibleNetworks = WiFi.scanNetworks();
  Serial.println(F("scan done"));
  if (nbVisibleNetworks == 0) {
    Serial.println(F("no networks found. Reset to try again"));
    while (true); // no need to go further, hang in there, will auto launch the Soft WDT reset
  }

  // ----------------------------------------------------------------
  // if you arrive here at least some networks are visible
  // ----------------------------------------------------------------
  Serial.print(nbVisibleNetworks);
  Serial.println(" network(s) found");

  // ----------------------------------------------------------------
  // check if we recognize one by comparing the visible networks
  // one by one with our list of known networks
  // ----------------------------------------------------------------
  for (i = 0; i < nbVisibleNetworks; ++i) {
    Serial.println(WiFi.SSID(i)); // Print current SSID
    for (n = 0; n < KNOWN_SSID_COUNT; n++) { // walk through the list of known SSID and check for a match
      if (strcmp(KNOWN_SSID[n], WiFi.SSID(i).c_str())) {
        Serial.print(F("\tNot matching "));
        Serial.println(KNOWN_SSID[n]);
      } else { // we got a match
        wifiFound = true;
        break; // n is the network index we found
      }
    } // end for each known wifi SSID
    if (wifiFound) break; // break from the "for each visible network" loop
  } // end for each visible network

  if (!wifiFound) {
    Serial.println(F("no Known network identified. Reset to try again"));
    while (true); // no need to go further, hang in there, will auto launch the Soft WDT reset
  }

  // ----------------------------------------------------------------
  // if you arrive here you found 1 known SSID
  // ----------------------------------------------------------------
  Serial.print(F("\nConnecting to "));
  Serial.println(KNOWN_SSID[n]);

  // ----------------------------------------------------------------
  // We try to connect to the WiFi network we found
  // ----------------------------------------------------------------
  WiFi.begin(KNOWN_SSID[n], KNOWN_PASSWORD[n]);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  // ----------------------------------------------------------------
  // SUCCESS, you are connected to the known WiFi network
  // ----------------------------------------------------------------
  WiFi.hostname("Angua");
  Serial.println(F("WiFi connected, your IP address is "));
  Serial.println(WiFi.localIP());

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
   mqttClient.setCallback(subscribeReceive);
   mqttClient.setClient(WIFI_CLIENT);
   mqttClient.connect("Sens1");
   
  
   while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    Serial.print("connection state: ");
    Serial.println(mqttClient.state());
    delay(300);
    mqttClient.connect("Sens1");
    connectionAtemped = connectionAtemped +1;
      if (connectionAtemped > 0){
        Serial.println("Atempting restart");
        ESP.restart();
      }
   }
    if (mqttClient.connect("Sens1")) {
      // connection succeeded
      mqttClient.setCallback(subscribeReceive);
      Serial.println("Connected ");
      connectionAtemped = 0;
      
    } 
    else {
      // connection failed
      // mqttClient.state() will provide more information
      // on why it failed.
      Serial.println("Connection failed ");
      
    }
  
 
}
 
void loop() {
  // This is needed at the top of the loop!
          mqttClient.loop();
         if (FirstLoop < 1){
            mqttClient.publish("beat", "3",1);
            FirstLoop = 1;
             //mqttClient.subscribe("aap");
             //mqttClient.subscribe("beer");
             //mqttClient.subscribe("cat");
             //mqttClient.subscribe("deer"); 
            Serial.println("Flushing sensor box");
            digitalWrite(D5, HIGH); 
            delay(Flush*(1000*mobSensBaseTimeMultiplier));//6000
            mqttClient.publish("S1Status", "Booted");
            digitalWrite(D5, LOW); 
          }
 
  if (!mqttClient.connected()) {
   reconnect();  
   //mqttClient.setCallback(subscribeReceive);
    //mqttClient.subscribe("aap");
    //mqttClient.subscribe("beer");
    //mqttClient.subscribe("cat"); 
    //mqttClient.subscribe("deer"); 
 }

  // one wire temp sensors
  sensors.requestTemperatures();
  tempC1 = sensors.getTempCByIndex(0);
  Serial.print("Internal sensor: ");
  Serial.println(tempC1);
  tempC2 = sensors.getTempCByIndex(1);
  Serial.print("external Sensor: ");
  Serial.println(tempC2);
  itoa(tempC1*100,buffer,10); //(integer, yourBuffer, base)
  mqttClient.publish("intTemp", buffer,1);
  itoa(tempC2*100,buffer,10); //(integer, yourBuffer, base)
  mqttClient.publish("extTemp", buffer,1);

  //VOC via anlog in
  int sensorValue = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V) by " * (5.0 / 1023.0)":
  float voltage = sensorValue;
  // print out the value you read:
  Serial.print("VOC value = ");
  Serial.print(voltage);
  Serial.println(" units");
  itoa(voltage,buffer,10); //(integer, yourBuffer, base)
  mqttClient.publish("S1VOC", buffer,1);

  //BME 680
  // Tell BME680 to begin measurement.
  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println(F("Failed to begin reading :("));
    mqttClient.publish("S1Status", "Failed to begin reading BME680");
    return;
  }
 
  
  if (!bme.endReading()) {
    Serial.println(F("Failed to complete reading :("));
    return;
  }

  Serial.print(F("Temperature = "));
  Serial.print(bme.temperature);
  Serial.println(F(" *C"));
  itoa(bme.temperature*100,buffer,10); //(integer, yourBuffer, base)
  mqttClient.publish("S1temperature", buffer,1);

  Serial.print(F("Pressure = "));
  Serial.print(bme.pressure / 100.0);
  Serial.println(F(" hPa"));
  itoa(bme.pressure,buffer,10); //(integer, yourBuffer, base)
  mqttClient.publish("S1pressure", buffer,1);

  Serial.print(F("Humidity = "));
  Serial.print(bme.humidity);
  Serial.println(F(" %"));
  itoa(bme.humidity,buffer,10); //(integer, yourBuffer, base)
  mqttClient.publish("S1humidity", buffer,1);
  
  Serial.print(F("Gas = "));
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(F(" KOhms"));
  Serial.println();
  itoa(bme.gas_resistance,buffer,10); //(integer, yourBuffer, base)
  mqttClient.publish("S1gas", buffer,1);

  if (beat == 0){
  beat = 1;
  }
  beat = beat * -1;
  itoa(beat,buffer,10); //(integer, yourBuffer, base)
  mqttClient.publish("beat", buffer,1);
  digitalWrite(D5, LOW);
  while (loopCounter < 6){
    loopCounter = loopCounter +1;
    beat = beat * -1;
    itoa(beat,buffer,10); //(integer, yourBuffer, base)
    mqttClient.publish("beat", buffer,1);
    delay(1000); 
  }
  digitalWrite(D5, LOW);  
  loopCounter = 0;
   while (loopCounter < mobSensHoldPreBreath){
     // one wire temp sensors
    sensors.requestTemperatures();
    tempC1 = sensors.getTempCByIndex(0);
    Serial.print("Internal sensor: ");
    Serial.println(tempC1);
    tempC2 = sensors.getTempCByIndex(1);
    Serial.print("external Sensor: ");
    Serial.println(tempC2);
    itoa(tempC1*100,buffer,10); //(integer, yourBuffer, base)
    mqttClient.publish("intTemp", buffer,1);
    itoa(tempC2*100,buffer,10); //(integer, yourBuffer, base)
    mqttClient.publish("extTemp", buffer,1);
    loopCounter = loopCounter +1;
   beat = beat * -1;
   itoa(beat,buffer,10); //(integer, yourBuffer, base)
   mqttClient.publish("beat", buffer,1);
   Serial.print(loopCounter);
   Serial.print("/");
   Serial.println(mobSensHoldPreBreath);
   delay(1000*mobSensBaseTimeMultiplier);    
    }
    loopCounter = 0;
  digitalWrite(D5, HIGH); 
  while (loopCounter < mobSensBreath){
   loopCounter = loopCounter +1;
   beat = beat * -1;
   itoa(beat,buffer,10); //(integer, yourBuffer, base)
   mqttClient.publish("beat", buffer,1);
   Serial.print(loopCounter);
   Serial.print("/");
   Serial.println(mobSensBreath);
   delay(1000*mobSensBaseTimeMultiplier); 
   }
  
  loopCounter = 0;
  while (loopCounter < mobSensHoldPostBreath){
   loopCounter = loopCounter +1;
   beat = beat * -1;
   itoa(beat,buffer,10); //(integer, yourBuffer, base)
   mqttClient.publish("beat", buffer,1);
   delay(1000*mobSensBaseTimeMultiplier); 
   }
   mqttClient.publish("beat", "0",1);
}
