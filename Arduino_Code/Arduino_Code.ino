#include <PID_v2.h>
#include <EEPROM.h>

#define PRINTER_RELAY_PIN 11

// Door variables
const int DOOR_SENSOR_UPDATE_RATE = 100; // Expressed in milliseconds
unsigned long lastDoorSensorUpdate = 0;

// Environment variables
const int ENVIRONMENT_SENSOR_SAMPLING_RATE = 1500; // Expressed in milliseconds
unsigned long lastEnvironmentSensorSampling = 0;
const int SAVE_DATA_RATE = 2500;
unsigned long lastDataSave = 0;
const int SENSOR_I2C_BEGIN = 3000; // Expressed in milliseconds
unsigned long lastI2CBegin = 0;
byte i2cBeginOk = 0;

// Fan variables
const int FAN_VALUE_UPDATE_RATE = 400; // Expressed in milliseconds
unsigned long lastFanValueUpdate = 0;

// Light variables
const int LIGHT_UPDATE_RATE = 25; // Expressed in milliseconds
unsigned long lastLightUpdate = 0;

// General variables
byte homePageActive = 0;
byte graphPageActive = 0;

// Serial input variables
const byte numChars = 10;
char receivedChars[numChars];
boolean newData = false;


void setup() {
  Serial.begin(115200);
  
  setupLight();
  setupDoor();
  i2cBeginOk = setupEnvironmentSensor();
  setupPID();
  setupFan();

  loadSettings();

  goToPage("Startup");
}

void loop() {

  recvWithStartEndMarkers();
  computeRPM();
  updateFanPower();

  if (newData) {
    handleDisplayInput(receivedChars);
    newData = false;
  }

  if (millis() - lastI2CBegin > SENSOR_I2C_BEGIN && !i2cBeginOk) {
    i2cBeginOk = setupEnvironmentSensor();
    lastI2CBegin = millis();
  }

  if (millis() - lastEnvironmentSensorSampling > ENVIRONMENT_SENSOR_SAMPLING_RATE) {
    readEnvironmentSensor();
    byte error = checkError();
    if (!error) {
      setTempTarget();
    }
    if (homePageActive) {
      sendEnvironmentValues();
    }
    lastEnvironmentSensorSampling = millis();
  }

  if (millis() - lastDataSave > SAVE_DATA_RATE && i2cBeginOk) {
    saveData();
    if (graphPageActive) {
      sendGraph();
    }
    lastDataSave = millis();
  }

  if (millis() - lastFanValueUpdate > FAN_VALUE_UPDATE_RATE && homePageActive) {
    sendFanValues();
    lastFanValueUpdate = millis();
  }

  if (millis() - lastDoorSensorUpdate > DOOR_SENSOR_UPDATE_RATE) {
    checkClosing();
    lastDoorSensorUpdate = millis();
  }

  if (millis() - lastLightUpdate > LIGHT_UPDATE_RATE) {
    updateBrightness();
    incrementBrightness();
    writeBrightness();
    lastLightUpdate = millis();
  }
}

// Code from Robin2 on arduino's website : Serial Input Basics
void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

// Read saved settings from the EEPROM
void loadSettings() {
  byte address = loadTempSettings(0);
  address = loadFanSettings(address);
  loadLightSettings(address);
}

/*void checkSetting(float value, float low, float high, float def){
  if(value >= low && value <= high){
    return value;
  }
  return def;
  }*/

int roundInt(int value, int multiply, int divide) {
  return round((float) value * multiply / divide);
}

byte mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return round((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}
