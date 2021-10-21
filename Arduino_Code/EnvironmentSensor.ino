#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Environment variables
const float temperatureThreshold = -0.3;
byte tempMode = 1; // to save
byte tempUnit = 1; // to save
float tempRequested = 25;  // to save
float tempDelta = 5; // to save
float tempTarget;

float tempIn;
float tempOut;
float humidityIn;
float humidityOut;

Adafruit_BME280 bmeIn; // I2C 0x77
Adafruit_BME280 bmeOut; // I2C 0x66
byte beginSuccessIn = 0; // Must be 0 to initialize it once
byte beginSuccessOut = 0;
byte errorSensorIn = 1; // Must be 1
byte errorSensorOut = 1;

double Kp = 100, Ki = 8.0, Kd = 0.0;
PID_v2 myPID(Kp, Ki, Kd, PID::Direct);

byte setupEnvironmentSensor() {
  // begin() returns true on success, false otherwise
  if (!beginSuccessIn) {
    beginSuccessIn = bmeIn.begin(0x77);
    // humidity sensing (suggested parameters from datasheet)
    // suggested rate is 1Hz (1s)
    if (beginSuccessIn) {
      bmeIn.setSampling(Adafruit_BME280::MODE_FORCED,
                        Adafruit_BME280::SAMPLING_X1,   // temperature
                        Adafruit_BME280::SAMPLING_NONE, // pressure
                        Adafruit_BME280::SAMPLING_X1,   // humidity
                        Adafruit_BME280::FILTER_OFF);
    }
  }

  if (!beginSuccessOut) {
    beginSuccessOut = bmeOut.begin(0x76);
    if (beginSuccessOut) {
      bmeOut.setSampling(Adafruit_BME280::MODE_FORCED,
                         Adafruit_BME280::SAMPLING_X1,   // temperature
                         Adafruit_BME280::SAMPLING_NONE, // pressure
                         Adafruit_BME280::SAMPLING_X1,   // humidity
                         Adafruit_BME280::FILTER_OFF);
    }
  }

  if (beginSuccessIn && beginSuccessOut) {
    return 1;
  } else {
    return 0;
  }

}

void setupPID() {
  myPID.SetOutputLimits(0, 320);
  myPID.SetMode(AUTOMATIC);
  myPID.Start(0, 0, 0);  // input, current output, setpoint
}

void setTemp(float value) {
  if (tempMode) {
    if (tempUnit) {
      tempDelta = value;
    } else {
      tempDelta = fahrenheitToCelsius(value);
    }
    EEPROM.put(6, tempDelta);
  } else {
    if (tempUnit) {
      tempRequested = value;
    } else {
      tempRequested = fahrenheitToCelsius(tempRequested);
    }
    EEPROM.put(2, tempRequested);
  }
}

void setUnit(byte newUnit) {
  tempUnit = newUnit;
  EEPROM.put(1, tempUnit);
}

void setTempMode(byte newTempMode) {
  tempMode = newTempMode;
  EEPROM.put(0, tempMode);
}

void setPIDMode(byte automatic) {
  if (automatic) {
    myPID.SetMode(AUTOMATIC);
  } else {
    myPID.SetMode(MANUAL);
  }
}

void readEnvironmentSensor() {
  errorSensorIn = !bmeIn.takeForcedMeasurement(); // has no effect in normal mode
  if (!errorSensorIn) {
    tempIn = bmeIn.readTemperature();
    humidityIn = bmeIn.readHumidity();
  } else {
    tempIn = 0.0;
    humidityIn = 0.0;
  }

  errorSensorOut = !bmeOut.takeForcedMeasurement(); // has no effect in normal mode
  if (!errorSensorOut) {
    tempOut = bmeOut.readTemperature();
    humidityOut = bmeOut.readHumidity();
  } else {
    tempOut = 0.0;
    humidityOut = 0.0;
  }
}

byte checkError() {
  // Only send values if it changes
  static byte lastErrorIn;
  static byte lastErrorOut;

  if (errorSensorIn != lastErrorIn) {
    printVal(F("Home.errorSensorIn"), errorSensorIn, 0);
    lastErrorIn = errorSensorIn;
  }
  if (errorSensorOut != lastErrorOut) {
    printVal(F("Home.errorSensorOut"), errorSensorOut, 0);
    lastErrorOut = errorSensorOut;
  }

  if (errorSensorIn || errorSensorOut) {
    fanSafety(1);
    return 1;
  } else {
    fanSafety(0);
    return 0;
  }
}

void setTempTarget() {
  if (tempMode) {
    tempTarget = tempOut + tempDelta;
  } else {
    tempTarget = tempRequested;
  }
}

int temperatureErrorFeedback() {
  // Stop a bit above ambiant
  if (tempIn > tempOut + temperatureThreshold) {
    return myPID.Run(tempTarget - tempIn);
  } else {
    return 0;
  }
}

float celsiusToFahrenheit(float val) {
  return (val * 1.8) + 32;
}

float fahrenheitToCelsius(float val) {
  return (val - 32) * 5 / 9;
}

void sendEnvironmentValues() {
  // Initialize to an impossible value to send it once
  static float lastValue[4] = {-100.0, -100.0, -100.0, -100.0};
  float tIn;
  float tOut;
  if (tempUnit) {
    tIn = tempIn;
    tOut = tempOut;
  } else {
    tIn = celsiusToFahrenheit(tempIn);
    tOut = celsiusToFahrenheit(tempOut);
  }

  // Compare to a tenth
  if (round(10 * tIn) != round(10 * lastValue[0])) {
    lastValue[0] = tIn;
    printFloatTxt(F("Home.tempIn"), tIn, 1, "");
  }
  if (round(10 * tOut) != round(10 * lastValue[1])) {
    lastValue[1] = tOut;
    printFloatTxt(F("Home.tempOut"), tOut, 1, "");
  }
  // Comapre to the unit
  if (round(humidityIn) != round(lastValue[2])) {
    lastValue[2] = humidityIn;
    printFloatTxt(F("Home.humidityIn"), humidityIn, 0, "%");
    printVal(F("Home.humidityInSli"), humidityIn, 0);
  }
  if (round(humidityOut) != round(lastValue[3])) {
    lastValue[3] = humidityOut;
    printFloatTxt(F("Home.humidityOut"), humidityOut, 0, "%");
    printVal(F("Home.humidityOutSli"), humidityOut, 0);
  }
}

void sendEnvironmentSettings() {
  // Temperature page
  printVal(F("Temperature.mode"), tempMode, 0);
  printVal(F("Temperature.unit"), tempUnit, 0);
  printVal(F("Temperature.tempRequested"), tempRequested, 0);
  printVal(F("Temperature.tempDelta"), tempDelta, 0);
  // Home page
  // If not in Celsius (default on screen) change it to Fahrenheit 
  if(!tempUnit){
    // Impossible to send "°" with ASCII
    Serial.print(F("Home.tempUnit.txt=Home.degreeF.txt"));
    writeFF();
  }
}

byte loadTempSettings(byte start) {
  byte address = start;
  EEPROM.get(address, tempMode);
  address++;
  EEPROM.get(address, tempUnit);
  address++;
  EEPROM.get(address, tempRequested);
  address += 4;
  EEPROM.get(address, tempDelta);
  address += 4;
  return address;
}
