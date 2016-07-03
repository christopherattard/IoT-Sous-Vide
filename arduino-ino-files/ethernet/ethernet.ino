
#include <Ethernet.h>
#include <PubSubClient.h>


double logTime = 0;
int logInterval = 30000; // millis interval between mqtt reports
double output; // PID output for relay control
unsigned long logTimer; // timer for log updates
String string; // temporary var for data type conversion
char buffer[256]; // temporary var for data type conversion


// Update these with values suitable for your network.
byte mac[]    = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xDF };
byte server[] = { 192, 168, 0, 11 };

// start ethernet client
EthernetClient ethClient;
// start mqtt client
PubSubClient mqttClient(server, 1883, ethClient);

void setup()
{
  Serial.begin(9600); // disable Serial, to regain use of pins 0,1
  
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("Failed to get IP Address");
  }
  else
  {
    Serial.println("Ethernet connection established");
  }
  
  mqttConnect();
  
  logTimer = millis();  
    
  mqttClient.publish("stdout","restart"); // restart the log file
}

void loop()
{
  Serial.println("loop(): sleeping 3000ms...");  
  delay(3000);

  logUpdate(); // publish temperature data to mqtt channel "data"
  
  if (mqttClient.loop() == false) mqttConnect();  // reconnect if I've lost connection to mqtt
}

// this is what to do whenever I'm disconnected from MQTT
void mqttConnect() 
{
  Serial.println("mqttConnect");
  if (mqttClient.connect("arduinoClient")) 
  {    
    Serial.println("mqttConnect OK");        
  }
  else
  {
    Serial.println("mqttConnect FAILED");    
  }
}

void logUpdate() 
{
    logTime+=0.5;
    logTimer+=logInterval; 
    dtostrf(logTime,2,1,buffer);
    size_t len=strlen(buffer);
    buffer[len++] = ',';
    dtostrf(23,2,1,&buffer[len]);
    len = strlen(buffer);
    buffer[len++] = ',';
    dtostrf(30,2,0,&buffer[len]);
    len = strlen(buffer);
    buffer[len++] = ',';
    dtostrf(22,2,0,&buffer[len]);
    mqttClient.publish("data",buffer);  
}
