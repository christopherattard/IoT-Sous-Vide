#include <SPI.h>
#include <Ethernet.h>

// the media access control (ethernet hardware) address for the shield:
//byte mac[] = { 0xDE, 0x1D, 0x4F, 0x24, 0x04, 0xD4 };
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xDF };
  
//the IP address for the shield:
byte ip[] = { 127, 0, 0, 1 };    

void setup()
{
  Serial.begin(9600);
  while (!Serial) 
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  Serial.print("DHCP tester");
  delay(1000);
  
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("Kazzata Pazzeska");  
  }
  else
  {
      Serial.print("OK");
  }
}

void loop () 
{
  
}
