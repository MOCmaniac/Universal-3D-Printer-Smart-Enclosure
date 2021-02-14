#define FAN_RPM_PIN 2 // Only on pin 2 or 3
#define FAN_PWM_PIN 9 // Only on pin 9 or 10
#define MAX_PWM 320

// Fan variables
const byte PULSE_PER_REVOLUTION = 2;
const unsigned short fanRPMMin = 230;
unsigned long updateTimeMax = 60e6 / (fanRPMMin*PULSE_PER_REVOLUTION); // Expressed in microseconds
unsigned long lastRPMUpdate = 0; // overflows after approximately 70 minutes using micros()
int volatile counter = 0;
int fanRPM = 0;

byte fanMode = 1;  // to save
int fanPWMMin = 35;  // to save
int fanPWMMax = MAX_PWM;  // to save
int fanPowerManual = 0;  // to save
int fanPower = 0;

void setupFan() {
  // Configure Timer 1 for PWM @ 25 kHz.
  TCCR1A = 0;           // undo the configuration done by...
  TCCR1B = 0;           // ...the Arduino core library
  TCNT1  = 0;           // reset timer
  TCCR1A = _BV(COM1A1)  // non-inverted PWM on ch. A
           | _BV(COM1B1)  // same on ch; B
           | _BV(WGM11);  // mode 10: ph. correct PWM, TOP = ICR1
  TCCR1B = _BV(WGM13)   // ditto
           | _BV(CS10);   // prescaler = 1
  ICR1   = 320;         // TOP = 320

  pinMode(FAN_PWM_PIN, OUTPUT);
  analogWrite25k(FAN_PWM_PIN, 0);

  pinMode(FAN_RPM_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(FAN_RPM_PIN), RPMInterrupt, FALLING);
}

void setFanPower(int newPower) {
  fanPowerManual = round((float) newPower * MAX_PWM / 100);
  EEPROM.put(19, fanPowerManual);
}

void setPWMMin(int newMin) {
  fanPWMMin = newMin * MAX_PWM / 100;
  EEPROM.put(15, fanPWMMin);
}

void setPWMMax(int newMax) {
  fanPWMMax = newMax * MAX_PWM / 100;
  EEPROM.put(17, fanPWMMax);
}

void setFanMode(byte newMode) {
  fanMode = newMode;
  EEPROM.put(14, fanMode);
  setPIDMode(fanMode);
}

void setFanPower() {
  if (fanMode) {
    fanPower = constrain(temperatureErrorFeedback(), 0, MAX_PWM);
    if (fanPower >= 1) {
      fanPower = map(fanPower, 0, MAX_PWM, fanPWMMin, fanPWMMax);
    }
  } else {
    fanPower = fanPowerManual;
  }
  analogWrite25k(FAN_PWM_PIN, fanPower);
}

void computeRPM() {
  if (counter >= 6 || (micros() - lastRPMUpdate) > (updateTimeMax * 6.5)) { // No overflow problem using this expression
    //Update RPM every 6 counts, increase this for better RPM resolution,
    //decrease for faster update
    fanRPM = (counter * 1e6 * 60) / ((micros() - lastRPMUpdate) * PULSE_PER_REVOLUTION);
    lastRPMUpdate = micros();
    counter = 0;
  }
}

// PWM output @ 25 kHz, only on pins 9 and 10.
// Output value should be between 0 and 320, inclusive.
void analogWrite25k(int pin, int value) {
  switch (pin) {
    case 9:
      OCR1A = constrain(value, 0, MAX_PWM);
      break;
    case 10:
      OCR1B = constrain(value, 0, MAX_PWM);
      break;
    default:
      // no other pin will work
      break;
  }
}

void sendFanValues() {
  // Fan values
  int power = round((float) fanPower * 100 / 320);
  Serial.print(F("fanSlider.val="));
  Serial.print(power);
  writeFF();
  Serial.print(F("fanPower.txt=\""));
  Serial.print(power);
  Serial.print(F("%\""));
  writeFF();
  Serial.print(F("fanRPM.txt=\""));
  Serial.print(fanRPM);
  Serial.print(F(" RPM\""));
  writeFF();
  fanDisplay();
}

void fanDisplay() {
  if (fanRPM != 0) {
    Serial.print(F("fanRotate.en=1"));
    writeFF();
    fanIncrement();
  } else {
    Serial.print(F("fanRotate.en=0"));
    writeFF();
  }
}

void fanIncrement() {
  char* increment = "1";
  if (fanPower < 65) {
    increment = "1";
  } else if (fanPower < 129) {
    increment = "2";
  } else if (fanPower < 193) {
    increment = "3";
  } else if (fanPower < 257) {
    increment = "4";
  } else {
    increment = "6";
  }
  Serial.print(F("fanIncrement.val="));
  Serial.print(increment);
  writeFF();
}

void sendFanSettings() {
  // Fan page
  if (fanMode) {
    Serial.print(F("Fan.fanMode.txt=\"auto\""));
  } else {
    Serial.print(F("Fan.fanMode.txt=\"manual\""));
  }
  writeFF();
  Serial.print(F("Fan.fanPWMMin.val="));
  Serial.print(round(fanPWMMin * 100 / 320));
  writeFF();
  Serial.print(F("Fan.fanPWMMax.val="));
  Serial.print(round(fanPWMMax * 100 / 320));
  writeFF();
  Serial.print(F("Fan.fanPowerManual.val="));
  Serial.print(round(fanPowerManual * 100 / 320));
  writeFF();
}

void RPMInterrupt() {
  counter++;
}

byte loadFanSettings(byte start) {
  byte address = start;
  EEPROM.get(address, fanMode);
  address++;
  EEPROM.get(address, fanPWMMin);
  address += 2;
  EEPROM.get(address, fanPWMMax);
  address += 2;
  EEPROM.get(address, fanPowerManual);
  address += 2;
  return address;
}