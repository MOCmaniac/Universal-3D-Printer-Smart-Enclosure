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
float tempIn;
float tempOut;
float humidityIn;
float humidityOut;

Adafruit_BME280 bmeIn; // I2C 0x77
Adafruit_BME280 bmeOut; // I2C 0x66
byte errorSensorIn = 1; // Must be 1
byte errorSensorOut = 1; // Must be 1

double Kp = 20, Ki = 0.1, Kd = 0.0;
PID_v2 myPID(Kp, Ki, Kd, PID::Direct);

/*
   After this function, the sensor MUST be ready to take temperature and humidity measurements
*/
void setupEnvironmentSensor() {
  myPID.SetOutputLimits(0, 320);
  myPID.SetMode(AUTOMATIC);
  myPID.Start(0, 0, 0);  // input, current output, setpoint

  // begin() returns true on success, false otherwise
  unsigned status = bmeIn.begin(0x77);
  if (status) {
    errorSensorIn = 0;
  }

  status = bmeOut.begin(0x76);
  if (status) {
    errorSensorOut = 0;
  }
  // humidity sensing (suggested parameters from datasheet)
  // suggested rate is 1Hz (1s)
  bmeIn.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1,   // temperature
                    Adafruit_BME280::SAMPLING_NONE, // pressure
                    Adafruit_BME280::SAMPLING_X1,   // humidity
                    Adafruit_BME280::FILTER_OFF);

  bmeOut.setSampling(Adafruit_BME280::MODE_FORCED,
                     Adafruit_BME280::SAMPLING_X1,   // temperature
                     Adafruit_BME280::SAMPLING_NONE, // pressure
                     Adafruit_BME280::SAMPLING_X1,   // humidity
                     Adafruit_BME280::FILTER_OFF);
}

void setUnit(byte newUnit) {
  tempUnit = newUnit;
  EEPROM.put(1, tempUnit);
}

void setTempMode(byte newTempMode) {
  tempMode = newTempMode;
  EEPROM.put(0, tempMode);
}

void setPIDMode(byte automatic){
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

void checkError(){
  static byte lastErrorIn;
  static byte lastErrorOut;
  
  if(errorSensorIn != lastErrorIn){
    Serial.print(F("Home.errorSensorIn.val="));
    Serial.print(errorSensorIn);
    writeFF();
    lastErrorIn = errorSensorIn;
  }
   if(errorSensorOut != lastErrorOut){
    Serial.print(F("Home.errorSensorOut.val="));
    Serial.print(errorSensorOut);
    writeFF();
    lastErrorOut = errorSensorOut;
  }
}

int temperatureErrorFeedback() {
  if (tempIn > tempOut + temperatureThreshold) {
    float t;
    if (tempMode) {
      t = tempOut + tempDelta;
    } else {
      if (tempUnit) {
        t = tempRequestedC;
      } else {
        t = tempRequestedF;
      }
    }
    return myPID.Run(t - tempIn);
  } else {
    return 0;
  }
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

float celsiusToFahrenheit(float val) {
  return (val * 1.8) + 32;
}

float fahrenheitToCelsius(float val) {
  return (val - 32) * 5 / 9;
}

void sendTempValues() {
  Serial.print(F("tempOut.txt=\""));
  if (tempUnit) {
    Serial.print(tempOut, 1);
  } else {
    Serial.print(celsiusToFahrenheit(tempOut), 1);
  }
  Serial.print(F("\""));
  writeFF();

  Serial.print(F("humidityOut.txt=\""));
  Serial.print(humidityOut, 0);
  Serial.print(F("%\""));
  writeFF();
  Serial.print(F("humidityOutSli.val="));
  Serial.print(humidityOut, 0);
  writeFF();

  Serial.print(F("tempIn.txt=\""));
  if (tempUnit) {
    Serial.print(tempIn, 1);
  } else {
    Serial.print(celsiusToFahrenheit(tempIn), 1);
  }
  Serial.print(F("\""));
  writeFF();

  Serial.print(F("humidityIn.txt=\""));
  Serial.print(humidityIn, 0);
  Serial.print(F("%\""));
  writeFF();
  Serial.print(F("humidityInSli.val="));
  Serial.print(humidityIn, 0);
  writeFF();
}

void sendTempSettings() {
  // Temperature page
  if (tempMode) {
    Serial.print(F("Temperature.tempMode.txt=\"Relative\""));
  } else {
    Serial.print(F("Temperature.tempMode.txt=\"Absolute\""));
  }
  writeFF();
  Serial.print(F("Temperature.tempRequestedC.val="));
  Serial.print(tempRequestedC, 0);
  writeFF();
  Serial.print(F("Temperature.tempRequestedF.val="));
  Serial.print(celsiusToFahrenheit(tempRequestedF), 0);
  writeFF();
  Serial.print(F("Temperature.tempDelta.val="));
  Serial.print(tempDelta, 0);
  writeFF();
  if (tempUnit) {
    Serial.print(F("Temperature.tempUnit.txt=Temperature.degreeC.txt"));
  } else {
    Serial.print(F("Temperature.tempUnit.txt=Temperature.degreeF.txt"));
  }
  writeFF();

  checkError();
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
