#include <Pinger.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <elapsedMillis.h>
#include <Wire.h>
#include <SPI.h>
//#include <OneWire.h>
//#include <DallasTemperature.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

/*#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
#define SEALEVELPRESSURE_HPA (1013.25)
*/

//#define ONE_WIRE_PIN D3
//OneWire oneWire(ONE_WIRE_PIN);
//DallasTemperature sensors(&oneWire);

// global var
//int vent_pin = 14;//GPIO 14
float tempC1;
float tempC2;
int beat = 0;
int connectionAtemped = 0;
int mobSensHoldPreBreath = 1;
int mobSensBreath = 10;
int mobSensHoldPostBreath = 5;
Adafruit_BME680 bme; // I2C
WiFiClient WIFI_CLIENT;
WiFi setHostname("Angua");

int period = 600;// 150;//150 seconden = 2.5 min
int FirstLoop = 0;
char buffer[12];         //the ASCII of the integer will be stored in this char array

// Function prototypes
void subscribeReceive(char* topic, byte* payload, unsigned int length);

// Ethernet and MQTT related objects
EthernetClient ethClient;
PubSubClient mqttClient(ethClient);
Pinger pinger;
int pingResult;

// Network SSID
const char* ssid = "WiFi-2.4-09A8 Accespoint";
const char* ssid2 = "WiFi-2.4-09A8";
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
  if (strTopic == "a"){
    Serial.println("topic ontvangen");
    Serial.println(payload[0]);
    mobSensHoldPreBreath = payload[0];  
    mobSensBreath = payload[1]; 
    mobSensHoldPostBreath = payload[2]; 
    }
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
}


void setup() {
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
   mqttClient.connect("Sens1");
  
   while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    Serial.print("connection state: ");
    Serial.println(mqttClient.state());
    delay(300);
    mqttClient.connect("Sens1");
    connectionAtemped = connectionAtemped +1;
      if (connectionAtemped > 3){
        Serial.println("Atempting restart");
        ESP.restart();
      }
   }
    if (mqttClient.connect("Sens1")) {
      // connection succeeded
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
  // put your main code here, to run repeatedly:
   // This is needed at the top of the loop!
          //mqttClient.loop();
         if (FirstLoop < 1){
            FirstLoop = 1;
            Serial.println("Flushing sensor box");
            digitalWrite(D5, HIGH); 
            delay(60000);//6000
            mqttClient.publish("S1Status", "Booted");
          }
//   if (!mqttClient.connected()) {
//    reconnect();   
//   }
//  // one wire temp sensors
//  sensors.requestTemperatures();
//  tempC1 = sensors.getTempCByIndex(0);
//  Serial.print("Internal sensor: ");
//  Serial.println(tempC1);
//  tempC2 = sensors.getTempCByIndex(1);
//  Serial.print("external Sensor: ");
//  Serial.println(tempC2);
//  
//  itoa(tempC1,buffer,10); //(integer, yourBuffer, base)
//  //mqttClient.publish("intTemp", buffer,1);
//  
//  itoa(tempC2,buffer,10); //(integer, yourBuffer, base)
//  //mqttClient.publish("extTemp", buffer,1);
//      
  //analogWrite(vent_pin, 254); // analog value 0-255 127 = 50%
  digitalWrite(D5, LOW); 
  int sensorValue = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V) by " * (5.0 / 1023.0)":
  float voltage = sensorValue;
  // print out the value you read:
  Serial.print("VOC value = ");
  Serial.print(voltage);
  Serial.println(" units");
  itoa(voltage,buffer,10); //(integer, yourBuffer, base)
  mqttClient.publish("S1VOC", buffer,1);
    
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
  itoa(bme.temperature,buffer,10); //(integer, yourBuffer, base)
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
  itoa(bme.gas_resistance,buffer,10); //(integer, yourBuffer, base)
  mqttClient.publish("S1gas", buffer,1);

//  Serial.print(F("Approx. Altitude = "));
//  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
//  Serial.println(F(" m"));

//  itoa(millis(),buffer,10); //(integer, yourBuffer, base)
//  mqttClient.publish("S1Status", buffer,1);

if (beat == 0){
  beat = 1;
}
beat = beat * -1;
itoa(beat,buffer,10); //(integer, yourBuffer, base)
  mqttClient.publish("beat", buffer,1);
  digitalWrite(D5, LOW);
  delay(6000);
  digitalWrite(D5, HIGH); 
  delay(3000);//6000
  
  
  //wait(60);

}
