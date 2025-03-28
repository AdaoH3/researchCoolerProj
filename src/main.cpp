#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>

//Relay Pin Definitions from my Arduino Nano
#define TEPIN 2
#define PUMPPIN 3
#define FANPIN 4

//OneWire Temp Sensor Read Definition
#define ONE_WIRE_BUS 12

//Below are my sensors addresses, you can find yours by running the example sketch in the DallasTemperature library
//Sensor 1 Address: 28 15 B8 46 D4 79 49 6F 
//Sensor 2 Address: 28 51 8D 46 D4 DB 24 77 
DeviceAddress radiatorTemp = {0x28, 0x15, 0xB8, 0x46, 0xD4, 0x79, 0x49, 0x6F};
DeviceAddress roomTemp = {0x28, 0x51, 0x8D, 0x46, 0xD4, 0xDB, 0x24, 0x77};

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

bool freezeToggle;

int radTemp;
int romTemp;

unsigned long lastSensorRead = 0;  // To keep track of when to read sensors
const unsigned long sensorInterval = 15000;  // 15 seconds interval for reading sensors

String command = "";  

void setup() {
  Serial.begin(115200); //Start serial communication baud rate for Arduino Nano
  sensors.begin();
  
  pinMode(TEPIN, OUTPUT);
  pinMode(PUMPPIN, OUTPUT);
  pinMode(FANPIN, OUTPUT);

  //All pins initalized as HIGH (off in this scenario)
  digitalWrite(TEPIN, HIGH);
  digitalWrite(PUMPPIN, HIGH);
  digitalWrite(FANPIN, HIGH);

  Serial.println("Setup Complete");
}

void loop() {
  unsigned long currentMillis = (unsigned long) millis();
  
  //Reads sensors every sensorInterval seconds
  if (currentMillis % sensorInterval == 0) {
    sensors.requestTemperatures();
    
    //Example functions to detect if temperature sensors are disconnected/faulty
    radTemp = sensors.getTempC(radiatorTemp);
    if (radTemp == DEVICE_DISCONNECTED_C) {
      Serial.println("Error: Sensor 1 disconnected");
    }
    
    romTemp = sensors.getTempC(roomTemp);
    if (romTemp == DEVICE_DISCONNECTED_C) {
      Serial.println("Error: Sensor 2 disconnected");
    }
    
    // Print sensor readings -> Logged by Python Script
    Serial.print("RADIATOR,");
    Serial.println(radTemp);
    
    Serial.print("ROOM,");
    Serial.println(romTemp);
    
  }

  //Manual control of cooling system using Serial commands from laptop
  if (Serial.available() > 0) {
    char incomingChar = Serial.read(); 
    
    if (incomingChar == '\n') {  
      command.trim();  
      if (command == "TEPIN ON") {
        digitalWrite(TEPIN, LOW);
      } 
      else if (command == "TEPIN OFF") {
        digitalWrite(TEPIN, HIGH);
      } 
      else if (command == "PUMPPIN ON") {
        digitalWrite(PUMPPIN, LOW);
      } 
      else if (command == "PUMPPIN OFF") {
        digitalWrite(PUMPPIN, HIGH);
      } 
      else if (command == "FANPIN ON") {
        digitalWrite(FANPIN, LOW);
      } 
      else if (command == "FANPIN OFF") {
        digitalWrite(FANPIN, HIGH);
        Serial.println("FANPIN turned OFF");
      } 
      else {
        Serial.println("Invalid");
      }
      
      command = ""; 
    } 
    else {
      command += incomingChar; 
    }
  }

  
  //Greater than 5 C
  if((int)radTemp >= 5 && !freezeToggle)
  {
    //turn cooling system on
    Serial.println("FREEZEON");
    digitalWrite(TEPIN, LOW);
    digitalWrite(PUMPPIN, LOW);
    freezeToggle = true;
  }
  else if ((int) radTemp < 0 && freezeToggle)
  {
    //Turn cooling system off
    Serial.println("FREEZEOFF");
    digitalWrite(TEPIN, HIGH);
    digitalWrite(PUMPPIN, HIGH);
    freezeToggle = false;
  }
}
