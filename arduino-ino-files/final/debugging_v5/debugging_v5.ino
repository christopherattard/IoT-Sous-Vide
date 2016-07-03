
// In this version only the keypad bugs were fixed. also filtration
//as regarding numbers entry via keypad were accomplished

//********************* The required libraries ****************

#include <PID_v1.h> 
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Ethernet.h>
#include <PubSubClient.h>

//********** Global variables ***********************************

// variables associated with PID controller

double setpoint = 0; // desired temperature
double actual_temp; // actual temperature
//double kp=675, ki=0.1, kd=0; // PID tuning
//double kp=943, ki=1.13, kd=1909; // PID tuning
double kp=600, ki=0.06, kd=2.4; // PID tuning values
double PID_OUTPUT; // PID output for relay control

// variables associated with PWM....
int pwmWindowSize = 5000; // PWM window for relay. 
unsigned long pwmWindowStartTime;

// variables associated with updates
double logTime = 0;
unsigned long logTimer; // timer for log updates
int logInterval = 30000; // millis interval between mqtt reports
int lcdInterval = 1000; // interval for LCD updates. 
unsigned long lcdTimer; // timer for LCD updates.

// variables associated with Ethernet connection
byte mac[]    = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xDF };
byte server[] = { 192, 168, 0, 11 }; //verify this IP address is correct
EthernetClient ethClient; // ethernet client
PubSubClient mqttClient(server, 1883, ethClient); //mqtt client

// Array for temporary storage of values
char temporary_storage[256]; // array of chars - temporary variables storage for data type conversion.

//*** Macros serving as constants ****************************************

#define SSRELAY 3 // Solid state Relay on pin 3
#define DS18B20 47 // Setup a onewire bus to accomodate the dallas temperature sensors 

//** Global Objects *************************************************************

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);//LiquidCrystal lcd(RS, E, D4, D5, D6, D7), initialize the library with the numbers of the interface pins
OneWire tempsen_1wire(DS18B20);// Pass the oneWire reference to Dallas Temperature sensors.
DallasTemperature temp_sensors(&tempsen_1wire);
DeviceAddress Probe01 = { 0x28, 0xFF, 0xC1, 0x3D, 0x04, 0x15, 0x03, 0x1D }; //Address for Sensor1
DeviceAddress Probe02 = { 0x28, 0xFF, 0xBF, 0x3B, 0x04, 0x15, 0x13, 0xDB }; //Address for Sensor2
PID heatPID(&actual_temp, &PID_OUTPUT, &setpoint, kp, ki, kd, DIRECT);// Setup PID

//****************** Keypad ********************************************************************

const byte numRows= 4; //number of rows on the keypad ie Four rows
const byte numCols= 4; //number of columns on the keypad ie Four columns
//keymap defines the key pressed according to the row and columns just as appears on the keypad
 char keymap[numRows][numCols]= 
 {
 {'1', '2', '3', 'A'}, 
 {'4', '5', '6', 'B'}, 
 {'7', '8', '9', 'C'},
 {'*', '0', '#', 'D'}
 };
//Code that shows the the keypad connections to the arduino terminals
 byte rowPins[numRows] = {21,14,15,17};  // Connect keypad ROW1, ROW2, ROW3 and ROW4 to these Arduino pins
 byte colPins[numCols]= {16,18,19,20};  // Connect keypad COL1, COL2 and COL3 to these Arduino pins
//initializes an instance of the Keypad class
 Keypad myKeypad= Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);
 //boolean variables associated with the entry of data via the keypad 
 bool IsAPressed = false; // when the letter A is pressed from the keypad all operations are interrupted 
 bool ResetSetpoint = true; // and the user is asked to enter a setpoint

//** Functions *******************************************************************

//********************** inherent arduino function***********************************

void setup() 
{
  Serial.begin(9600); // disable Serial, to regain use of pins 0,1
  lcd.begin(16,2);//16 columns and 2 rows in the lcd display
  lcd.clear();  
  //Establish an ethernet connection
  if (Ethernet.begin(mac) == 0) Serial.println("Failed to get IP Address");
  else Serial.println("Ethernet connection established");
  mqttConnect();  
  pwmWindowStartTime = millis();
  // initialise the temperature sensors bus
  temp_sensors.begin();
  //Set the resolution to 10 bit (Can be 9 to 12 bits .. lower is faster)
  temp_sensors.setResolution(Probe01, 10);
  temp_sensors.setResolution(Probe02, 10);
  //Setting the minimum and maximum range to vary the PID's PWM output
  heatPID.SetOutputLimits(0, pwmWindowSize);
  //Specifies the PID mode to be ON (Automatic)
  heatPID.SetMode(AUTOMATIC);
  pinMode(SSRELAY, OUTPUT); // disable Serial if using pin 0,1
  myKeypad.addEventListener(CheckIfAIsPressed); //add an event listener for this keypad
  myKeypad.setDebounceTime(200); 
  lcd.setCursor(0,0);
  lcd.print("system");
  lcd.setCursor(0,1);
  lcd.print("start");
  delay(2000);
  lcd.clear();
}

// inherent arduino function

void loop() 
{ 
 if (ResetSetpoint == true)
  {
    readSetpointFromKeypad();
    ResetSetpoint = false;
  }
  
  getTemperaturesAndPrint();
  print2lcd(); // update lcd
  logUpdate(); // publish information to mqtt channel "data"
  relayUpdate(); // implement pwm on relay
  heatPID.Compute(); // PID compute loop
  readAFromKeypad();  
  //delay(500);
  if (IsAPressed == true)
  {
    ResetSetpoint = true;
  }
  
  // reconnect if I've lost connection to mqtt
  if (mqttClient.loop() == false) mqttConnect();  
}

//************************ user defined functions*******************************

// this function establihses a connection to the MQTT Broker

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

// this function periodically prints the updated or rather new values to lcd. it Prints setpoint, temperature and output 
 
void print2lcd() 
{
  if(millis()>lcdTimer) 
  {    
    lcdTimer+=lcdInterval; //increment by interval for LCD updates
    lcd.clear();
    lcd.setCursor(0,0);
    
    lcd.print("Sp:");
    Serial.print("Sp:");
    //dtostrf() function converts a double value to string. 
    //Parameter1 is the double variable, Parameter2 is the minimum field width of the string, Parameter3 is the decimal precision, Parameter4 is the actual string.
    //For e.g. 34.66 should be dtostrf(setpoint,3,2,temporary_storage)
    dtostrf(setpoint,3,2,temporary_storage);    
    lcd.print(temporary_storage);// setpoint is stored in a temporary storage hence the setpoint is printed
    Serial.println(temporary_storage);
    
    lcd.print(" a:");
    Serial.print("Actual:");
    lcd.print(actual_temp);
    Serial.println(actual_temp);
    
    lcd.setCursor(0,1);
    lcd.print("Output:");
    dtostrf(PID_OUTPUT,1,0,temporary_storage);
    lcd.print(temporary_storage);
    Serial.print("Output:");
    Serial.println(temporary_storage);
    delay(100);
  }
}  

//this funtion is associated with the PWM to drive the relay

void relayUpdate() 
{
  if((millis()-pwmWindowStartTime) > pwmWindowSize) //determines the start of the duty cycle 
  { //time to shift the Relay Window
    pwmWindowStartTime += pwmWindowSize;
  }
  
  if(heatPID.GetMode()==MANUAL) PID_OUTPUT = 0;

  //Output is limited by the output limits, i.e., 0 and 5000
  if(PID_OUTPUT > (millis()-pwmWindowStartTime)) 
  {
    digitalWrite(SSRELAY,HIGH);
  } 
  else 
  {
    digitalWrite(SSRELAY,LOW);
  }
}

// this function is concerned with entering input data, ie the set point from the keypad which can only take two figures since it is
//  limited by a range of 28 degrees C to 40 degrees C. These figures represent the setpoint 

void readSetpointFromKeypad()
{
  bool correctInput = false; // local variable of boolean type
  
  do
  {
    lcd.clear();  
    lcd.print("Enter setpoint:");
    char key1 = myKeypad.waitForKey(); //Read first key pressed, deleration of local variables of data type char
    lcd.setCursor(0,1);
    lcd.print(key1);
    char key2 = myKeypad.waitForKey(); //Read second key pressed
    lcd.setCursor(1,1);
    lcd.print(key2);
    lcd.clear();  
    lcd.setCursor(0,0);
    //lcd.print("sp="+key1+key2);
  
    if (key1 != NO_KEY && key2 != NO_KEY)
    {
      if (key1 == '*' || key1 == '#' || key1 == 'A' || key1 == 'B' || key1 == 'C' || key1 == 'D' ||
          key2 == '*' || key2 == '#' || key2 == 'A' || key2 == 'B' || key2 == 'C' || key2 == 'D')
      {
        Serial.println("Invalid character");// filtration in order to omitt the entry of these keys
        lcd.print("Invalid character");
        delay(1000);
      }
      else
      {
        char tempsetpoint[3]; // buffer which temporarily stores the values of the pressed keys
        sprintf(tempsetpoint, "%c%c", key1, key2); //concatenate   
        setpoint = atoi(tempsetpoint); //conversion from string to integer  
        
        if (setpoint < 21 || setpoint > 40)// filtration as regarding the temperature range
        {
          Serial.println("Invalid entry");
          lcd.print("Invalid entry");
          delay(1000);
        }
        else
        {
          Serial.print("sp:");        
          Serial.println(tempsetpoint);
          lcd.print("sp=");
          lcd.print(tempsetpoint);
          correctInput = true;
        }
      }
    }
    else //NO_KEY
    {
      Serial.println("Input error");
      lcd.print("Input error");
      //delay(1000);
    }
  } //do
  while (correctInput == false);
}

// this function is concerned with the entry of letter A from the keypad 
void readAFromKeypad()
{
  Serial.println("Press letter A to exit:");
  //delay(1000);
  char keyA = myKeypad.getKey();
  //delay(100);
}

// this function enables the user that when the letter A is pressed on the keypad the program exits all events and compuations
// and asks the user for a setpoint

void CheckIfAIsPressed(KeypadEvent key)
{
  if (/*myKeypad.getState() == RELEASED &&*/ key == 'A')
  {
    IsAPressed = true;
    lcd.setCursor(0,1);
    lcd.print("A is pressed");
    Serial.println("Callback: A is pressed");
    //delay(2000);   
  }
  else
  {
    IsAPressed = false;  
  } 
}

//To obtain temperature from 2 sensors and calculate their average

void getTemperaturesAndPrint()
{
  bool device_1 = true;
  bool device_2 = true;
  // Send the command to get temperatures
  temp_sensors.requestTemperatures(); // read DS18B20 temperature sensor
  
  //Get temperature from Probe1
  float temp1 = temp_sensors.getTempC(Probe01);  
  if (temp1 == -127.00) // Measurement failed or no device found
  {
    Serial.println("Probe1 Temperature Error");
    device_1 = false;
  } 
  else
  { 
    Serial.print("temp1: ");  
    Serial.println(temp1);
    delay(1000);
  }

  //Get temperature from Probe2
  float temp2 = temp_sensors.getTempC(Probe02);  
  if (temp2 == -127.00) // Measurement failed or no device found
  {
    Serial.println("Probe2 Temperature Error");
    device_2 = false;
  } 
  else
  { 
    Serial.print("temp2: ");
    Serial.println(temp2);
    delay(1000);
  }
  if (device_1 == false && device_2 == false)
  {
    Serial.println("temperature cannot be obtained due to device malfuntion");
    lcd.setCursor(0,1);
    lcd.print("sensor");
    lcd.setCursor(1,1);
    lcd.print("malfunction");
  }
  else if (device_1 == true && device_2 == true)
  {
  actual_temp = (temp1 + temp2) / 2;
  Serial.print("avgtemp: ");
  Serial.println(actual_temp);
  delay(1000);
  }
  else
  {
    if (device_1 == true) 
    {actual_temp = temp1;} 
    else 
    {actual_temp = temp2;}
  } 
  Serial.print("actual temp: ");
  Serial.println(actual_temp);
  delay(1000);
}

void logUpdate() 
{
  char buffer[256]; // temporary var for data type conversion
  
  if(millis()>logTimer) 
  {
    logTime+=0.5;
    logTimer+=logInterval; 
    dtostrf(logTime,2,1,buffer);
    size_t len=strlen(buffer);
    buffer[len++] = ',';
    dtostrf(actual_temp,2,1,&buffer[len]);
    len = strlen(buffer);
    buffer[len++] = ',';
    dtostrf(setpoint,2,0,&buffer[len]);
    len = strlen(buffer);
    buffer[len++] = ',';
    dtostrf(PID_OUTPUT/pwmWindowSize*100,2,0,&buffer[len]);
    mqttClient.publish("data",buffer);
  }
}
