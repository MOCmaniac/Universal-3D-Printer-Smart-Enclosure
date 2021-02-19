#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Environment variables
const float temperatureThreshold = 0.3;
byte tempMode = 1; // to save
byte tempUnit = 1; // to save
float tempRequestedC = 22;  // to save
float tempRequestedF = 72;  // to save
float tempDelta = 3; // to save
float tempRequested;

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

double Kp = 20, Ki = 0.1, Kd = 0.0;
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
    tempDelta = value;
    EEPROM.put(10, tempDelta);
  } else {
    if (tempUnit) {
      tempRequestedC = value;
      EEPROM.put(2, tempRequestedC);
    } else {
      tempRequestedF = fahrenheitToCelsius(value);
      EEPROM.put(6, tempRequestedF);
    }
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

void checkError() {
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
  } else {
    fanSafety(0);
  }
}

void tempTarget() {
  if (tempMode) {
    tempRequested = tempOut + tempDelta;
  } else {
    if (tempUnit) {
      tempRequested = tempRequestedC;
    } else {
      tempRequested = tempRequestedF;
    }
  }
}

int temperatureErrorFeedback() {
  if (tempIn > tempOut + temperatureThreshold) {
    return myPID.Run(tempRequested - tempIn);
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
  float tIn;
  float tOut;
  if (tempUnit) {
    tIn = tempIn;
    tOut = tempOut;
  } else {
    tIn = celsiusToFahrenheit(tempIn);
    tOut = celsiusToFahrenheit(tempOut);
  }

  printTxt(F("Home.tempIn"), tIn, 1, "");
  printTxt(F("Home.tempOut"), tOut, 1, "");
  printTxt(F("Home.humidityIn"), humidityIn, 0, "%");
  printTxt(F("Home.humidityOut"), humidityOut, 0, "%");
  printVal(F("Home.humidityInSli"), humidityIn, 0);
  printVal(F("Home.humidityOutSli"), humidityOut, 0);
}

void sendTempSettings() {
  // Temperature page
  printVal(F("Temperature.mode"), tempMode, 0);
  printVal(F("Temperature.tempRequestedC"), tempRequestedC, 0);
  printVal(F("Temperature.tempRequestedF"), celsiusToFahrenheit(tempRequestedF), 0);
  printVal(F("Temperature.tempDelta"), tempDelta, 0);
}

byte loadTempSettings(byte start) {
  byte address = start;
  EEPROM.get(address, tempMode);
  address++;
  EEPROM.get(address, tempUnit);
  address++;
  EEPROM.get(address, tempRequestedC);
  address += 4;
  EEPROM.get(address, tempRequestedF);
  address += 4;
  EEPROM.get(address, tempDelta);
  address += 4;
  return address;
}
