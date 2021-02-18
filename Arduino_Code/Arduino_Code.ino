#include <PID_v2.h>
#include <EEPROM.h>

// Door variables
const int DOOR_SENSOR_UPDATE_RATE = 100; // Expressed in milliseconds
unsigned long lastDoorSensorUpdate = 0;

// Environment variables
const int ENVIRONMENT_SENSOR_SAMPLING_RATE = 1500; // Expressed in milliseconds
unsigned long lastEnvironmentSensorSampling = 0;
const int SENSOR_I2C_BEGIN = 3000; // Expressed in milliseconds
unsigned long lastI2CBegin = 0;
byte i2cBeginOk = 0;

// Fan variables
const int FAN_RPM_UPDATE_RATE = 200; // Expressed in milliseconds
unsigned long lastFanRPMUpdate = 0;

// Light variables
const int LIGHT_UPDATE_RATE = 25; // Expressed in milliseconds
unsigned long lastLightUpdate = 0;
byte brightnessChanging = 0;

// General variables
byte homeScreenActive = 0;

// Serial input variables
const byte numChars = 10;
char receivedChars[numChars];
boolean newData = false;


void setup() {
  Serial.begin(115200);

  setupDoor();
  i2cBeginOk = setupEnvironmentSensor();
  setupPID();
  setupFan();
  setupLight();

  loadSettings();

  goToPage("Welcome");
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

  if (millis() - lastEnvironmentSensorSampling > ENVIRONMENT_SENSOR_SAMPLING_RATE && homeScreenActive) {
    readEnvironmentSensor();
    checkError();
    sendEnvironmentValues();
    lastEnvironmentSensorSampling = millis();
  }

  if (millis() - lastFanRPMUpdate > FAN_RPM_UPDATE_RATE && homeScreenActive) {
    sendFanValues();
    lastFanRPMUpdate = millis();
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

void loadSettings() {
  byte address = loadTempSettings(0);
  address = loadFanSettings(address);
  loadLightSettings(address);
}

int roundInt(int value, int multiply, int divide) {
  return round((float) value * multiply / divide);
}
