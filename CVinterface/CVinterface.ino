
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <elapsedMillis.h>

//Declarations
int FirstLoop = 0;
char buffer[255];  
int connectionCounter = 0;
int LoopCounter = 0;

int BlKeuk = 22; 
int GrKeuk = 24;
int RoKeuk = 26;
int BlBad = 28;  
int GrBad = 30;
int RoBad = 32;
int BlBur = 34;  
int GrBur = 36;
int RoBur = 38;
//int MqttError = 40;
int interfaceStatus = 1;

int oBlKeuk = 0; 
int oGrKeuk = 0;
int oRoKeuk = 0;
int oBlBad = 0;  
int oGrBad = 0;
int oRoBad = 0;
int oBlBur = 0;  
int oGrBur = 0;
int oRoBur = 0;

// Ethernet and MQTT related objects
byte mac[] = { 0xDC, 0x52, 0x2A, 0x52, 0xFE, 0xED };
IPAddress ip(192,168,1,25);
EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

// Function prototypes
void subscribeReceive(char* topic, byte* payload, unsigned int length);

// Functions
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

String MQTTconnctionState(int state){
  if (state == -4){
    return "MQTT_CONNECTION_TIMEOUT";// - the server didn't respond within the keepalive time
  }
  if (state == -3){
    return "MQTT_CONNECTION_LOST";// - the network connection was broken
  }
  if (state == -2){
    return "MQTT_CONNECT_FAILED";// - the network connection failed
  }
  if (state == -1){
    return "MQTT_DISCONNECTED";// - the client is disconnected cleanly
  }
  if (state == 0){
    return "MQTT_CONNECTED";// - the client is connected
  }
  if (state == 1){
    return "MQTT_CONNECT_BAD_PROTOCOL";// - the server doesn't support the requested version of MQTT
  }
  if (state == 2){
    return "MQTT_CONNECT_BAD_CLIENT_ID";// - the server rejected the client identifier
  }
  if (state == 3){
    return "MQTT_CONNECT_UNAVAILABLE";// - the server was unable to accept the connection
  }
  if (state == 4){
    return "MQTT_CONNECT_BAD_CREDENTIALS";// - the username/password were rejected
  }
  if (state == 5){
    return "MQTT_CONNECT_UNAUTHORIZED";// - the client was not authorized to connect
  }
}
void subscribeToMqtt(){
  mqttClient.subscribe("LEDarray");
  mqttClient.subscribe("CvMonLamptest");
//   if( subscriptCounter < 10){
//    digitalWrite(MqttError, HIGH);
//   }
}
void subscribeReceive(char* topic, byte* payload, unsigned int length)
{
  // Print the topic
  //Serial.print("Topic: ");
  //Serial.println(topic);
 
  // Print the message
  //Serial.print("Message: ");
  String strTopic =String(topic);
  String strpayload ="";
  for(int i = 0; i < length; i ++)
  {
    //Serial.print(char(payload[i]));
    strpayload = strpayload + char(payload[i]);
  }
   Serial.println(" ");
//   Serial.print("strTopic: ");
//   Serial.println(strTopic);
//   Serial.print("strpayload: ");
//   Serial.println(strpayload);
//   
  mqttClient.publish("CVMonitorLastExecutingCommand", topic); 
  if (strTopic == "CvMonLamptest" && strpayload == "1")
  {
    Serial.println("Lamp test triggerd");
    digitalWrite(GrKeuk, HIGH);
    digitalWrite(BlKeuk, HIGH);
    digitalWrite(RoKeuk, HIGH);
    digitalWrite(GrBad, HIGH);
    digitalWrite(BlBad, HIGH);
    digitalWrite(RoBad, HIGH);
    digitalWrite(GrBur, HIGH);
    digitalWrite(BlBur, HIGH);
    digitalWrite(RoBur, HIGH);
    wait(30); //delay 30 sec
    digitalWrite(GrKeuk, LOW);
    digitalWrite(BlKeuk, LOW);
    digitalWrite(RoKeuk, LOW);
    digitalWrite(GrBad, LOW);
    digitalWrite(BlBad, LOW);
    digitalWrite(RoBad, LOW);
    digitalWrite(GrBur, LOW);
    digitalWrite(BlBur, LOW);
    digitalWrite(RoBur, LOW);
    Serial.println("Lamp test done");
  }
  if (strTopic == "LEDarray")
  {
    Serial.print("Message: ");
    strpayload = strpayload.substring(1);
    strpayload = strpayload.substring(0, strpayload.length() -1);
    Serial.println(strpayload);

    int i;
  char delimiter[] = ",";
  char *p;
  char string[128];
  String test = strpayload;
  String words[12];

  test.toCharArray(string, sizeof(string));
  i = 0;
  p = strtok(string, delimiter);
  while(p && i < 12)
  {
    words[i] = p;
    p = strtok(NULL, delimiter);
    ++i;
  }

  for(i = 0; i < 12; ++i)
  {
    Serial.print("words[");
    Serial.print(i);
    Serial.print("] ");
    Serial.println(words[i]);
  }
  if (words[6] == "true"){
    digitalWrite(GrBur, HIGH);
    oGrBur = 1;
  }
  else{
   digitalWrite(GrBur, LOW);
   oGrBur = 0;
  }
  if (words[7] == "true"||words[11] == "true" ){
    digitalWrite(BlBur, HIGH);
    oBlBur = 1;
  }
  if(words[7] != "true" && words[11] != "true" ){
   digitalWrite(BlBur, LOW);
   oBlBur = 0;
  }
  if (words[8] == "true"){
    digitalWrite(RoBur, HIGH);
    oRoBur = 1;
  }
  else{
   digitalWrite(RoBur, LOW);
   oRoBur = 0;
  }
  Serial.print("] ");
    Serial.println(words[i]);
 }
  Serial.print("oBlBur ");
  Serial.println(oBlBur);
  Serial.print("oGrBur ");
  Serial.println(oGrBur);
  Serial.print("oRoBur ");
  Serial.println(oRoBur);
}

void softReset(){
 Serial.println("ESP.restart()");
}

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("chrisFLFM")) {
      Serial.println("connected");
      subscribeToMqtt();
//      digitalWrite(MqttError, LOW);
    } else {
//      digitalWrite(MqttError, HIGH);
      Serial.print("failed, rc= ");
      Serial.print(MQTTconnctionState(mqttClient.state()));
      Serial.println(" try again in 5 seconds");
      connectionCounter = connectionCounter + 1;
      if (connectionCounter >= 5){
        connectionCounter = 0;
        softReset();
      }
      wait(5);
    }
  }
  Serial.println(" ");  
}

void setup() {
  Serial.begin(115200);

  // disable SD card if one in the slot
  pinMode(4,OUTPUT);
  digitalWrite(4,HIGH);

  Serial.println("Starting Ethernet shield");
  Ethernet.begin(mac,ip);
  wait(1);   //starting ethernet
  Serial.println(Ethernet.localIP());

  mqttClient.setServer("192.168.1.43", 1883);
  mqttClient.setCallback(subscribeReceive);
  mqttClient.setClient(ethClient);
  mqttClient.connect("chrisFLFM");  

  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    Serial.print("connection state: ");
    Serial.println(mqttClient.state());
    mqttClient.connect("chrisFLFM");
    wait(1);
   }
    if (mqttClient.connect("chrisFLFM")) {
      // connection succeeded
      Serial.println("Connected to MQTT server");
      
    } 
    else {
      // connection failed
      // mqttClient.state() will provide more information
      // on why it failed.
      Serial.println("Connection failed ");
  }
   // IO config:
 //pinMode(A0, INPUT);
 pinMode(GrKeuk, OUTPUT);    // sets the digital pin D5 as keuken groen
 pinMode(BlKeuk, OUTPUT);    // sets the digital pin D6 as keuken blauw
 pinMode(RoKeuk, OUTPUT);    // sets the digital pin D6 as keuken rood
 pinMode(GrBad, OUTPUT);    // sets the digital pin D5 as badkamer groen
 pinMode(BlBad, OUTPUT);    // sets the digital pin D6 as badkamer blauw
 pinMode(RoBad, OUTPUT);    // sets the digital pin D6 as badkamer rood
 pinMode(GrBur, OUTPUT);    // sets the digital pin D5 as bureel groen
 pinMode(BlBur, OUTPUT);    // sets the digital pin D6 as bureel blauw
 pinMode(RoBur, OUTPUT);    // sets the digital pin D6 as bureel rood
}

void loop() {
    // This is needed at the top of the loop!
  if (!mqttClient.connected()) {
    Serial.println("connection to MQTT server lost");
    reconnect();
  }
  mqttClient.loop();
  if (FirstLoop < 1){
    mqttClient.publish("CVMonitorStatus", "3");
    Serial.println("looping");
    digitalWrite(GrKeuk, LOW);
    digitalWrite(BlKeuk, LOW);
    digitalWrite(RoKeuk, LOW);
    digitalWrite(GrBad, LOW);
    digitalWrite(BlBad, LOW);
    digitalWrite(RoBad, LOW);
    digitalWrite(GrBur, LOW);
    digitalWrite(BlBur, LOW);
    digitalWrite(RoBur, LOW);
 //   digitalWrite(MqttError, LOW);
    subscribeToMqtt();
   mqttClient.publish("CVMonitorStatus", "0");
    FirstLoop = 10;
  }
  wait(1);
  yield();
  if (LoopCounter >=2){ //beat every 10 loops = 10 x wait time
  itoa(interfaceStatus,buffer,10); //(integer, yourBuffer, base)
  mqttClient.publish("CVMonitorStatus", buffer);
  interfaceStatus = interfaceStatus * -1;
  LoopCounter = -1;
  }
  LoopCounter = LoopCounter + 1;
}
