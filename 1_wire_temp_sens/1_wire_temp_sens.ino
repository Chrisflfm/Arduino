#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <elapsedMillis.h>
#include <Pinger.h>

// Data wire is plugged into digital pin 4 on the Arduino
#define ONE_WIRE_PIN D3


// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_PIN);  

// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

// variable to hold temp related info
DeviceAddress Thermometer;
int deviceCount = 0;
float tempC;


int connectionAtemped = 0;
WiFiClient WIFI_CLIENT;
int period = 600;// 150;//150 seconden = 2.5 min
int livePuls = 1;
int FirstLoop = 0;
char buffer[12];         //the ASCII of the integer will be stored in this char array
// Ethernet and MQTT related objects
EthernetClient ethClient;
PubSubClient mqttClient(ethClient);
Pinger pinger;
int pingResult;
int connectionAttempt = 0;
int maxconnectionAttempt = 20;

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
    if (mqttClient.connect("Sens2")) {
      Serial.println("connected");
      connectionAtemped = 0;
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      connectionAtemped = connectionAtemped +1;
      if (connectionAtemped > 0){
        Serial.println("Atempting restart");
        ESP.restart();
      }
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup(void)
{
  //pinMode(2, INPUT_PULLUP);
  //pinMode(0, INPUT_PULLUP);
  sensors.begin();  // Start up the library
  Serial.begin(115200);
  
  // locate devices on the bus
  delay(100);
  Serial.print("Locating devices...");
  Serial.print("Found ");
  deviceCount = sensors.getDeviceCount();
  Serial.print(deviceCount, DEC);
  Serial.println(" devices.");
  Serial.println("");
  delay(10);
  
  // Connect WiFi
  Serial.print("Connecting to ");
  WiFi.begin(ssid, password);
  
  connectionAttempt=0;
  while (WiFi.status() != WL_CONNECTED && connectionAttempt < maxconnectionAttempt) {
    delay(500);
    Serial.print(".");
    connectionAttempt =connectionAttempt + 1;
  }
  if (connectionAttempt >= maxconnectionAttempt){
    Serial.println(" No wifi connection restarting");
    ESP.restart();
  }
  Serial.println("");
  WiFi.hostname("Gaspode");
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
    mqttClient.publish("S2Status", "(re)connected");
    
  } 
  else {
    // connection failed
    // mqttClient.state() will provide more information
    // on why it failed.
    Serial.println("Connection failed ");
    
  }
  
}

void printAddress(DeviceAddress deviceAddress)
{ 
  for (uint8_t i = 0; i < 8; i++)
  {
    Serial.print("0x");
    if (deviceAddress[i] < 0x10) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    if (i < 7) Serial.print(", ");
  }
  Serial.println("");
}

void loop(void)
{ 
  // This is needed at the top of the loop!
          mqttClient.loop();
         if (FirstLoop < 1){
            FirstLoop = 1;
            mqttClient.publish("S2Status", "Booted");
            mqttClient.publish("sens2Puls", "0",1);
          }
   if (!mqttClient.connected()) {
    reconnect();   
   }    
  // Send command to all the sensors for temperature conversion
  sensors.requestTemperatures(); 
  
  // Display temperature from each sensor
  for (int i = 0;  i < deviceCount;  i++)
  {
    tempC = sensors.getTempCByIndex(i)*100;
    itoa(tempC,buffer,10); //(integer, yourBuffer, base)
    char tempSens[3] = {'t','e',i+48};
    mqttClient.publish(tempSens, buffer,1);
    Serial.print("sensor code: ");
    Serial.print(tempSens);
    Serial.print(" ");
    Serial.println(tempC);
//    sensors.getAddress(Thermometer, i);
//    printAddress(Thermometer);
  }
  livePuls = livePuls * -1;
  if (deviceCount < 3){
    livePuls = 3;
  }
  itoa(livePuls,buffer,10); //(integer, yourBuffer, base)
  mqttClient.publish("sens2Puls", buffer,1);
  Serial.println("**********************************************");
  delay(1000);
}
