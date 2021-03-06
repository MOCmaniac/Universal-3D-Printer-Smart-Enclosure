#define FAN_RPM_PIN 2 // Only on pin 2 or 3 (interrupt)
#define FAN_PWM_PIN 9 // Only on pin 9 or 10 (25kHz pwm)
#define MAX_PWM 320 // max pwm at 25kHz

// Fan variables
const float coeff = 0.97;
const byte PULSE_PER_REVOLUTION = 2;
const unsigned short fanRPMMin = 450;
unsigned long updateTimeMax = 60e6 / (fanRPMMin*PULSE_PER_REVOLUTION); // Expressed in microseconds
unsigned long lastRPMUpdate = 0; // overflows after approximately 70 minutes using micros()
int volatile counter = 0;
int fanRPM = 0;

byte fanMode = 1;  // to save
int fanPWMMin = 35;  // to save
int fanPWMMax = MAX_PWM;  // to save
int fanPowerManual = 0;  // to save
int fanPower = 0;

byte safety = 0;

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
  fanPowerManual = roundInt(newPower, MAX_PWM, 100);
  EEPROM.put(15, fanPowerManual);
}

void setPWMMin(int newMin) {
  fanPWMMin = roundInt(newMin, MAX_PWM, 100);
  EEPROM.put(11, fanPWMMin);
}

void setPWMMax(int newMax) {
  fanPWMMax = roundInt(newMax, MAX_PWM, 100);
  EEPROM.put(13, fanPWMMax);
}

void setFanMode(byte newMode) {
  fanMode = newMode;
  EEPROM.put(10, fanMode);
  setPIDMode(fanMode);
}

void fanSafety(byte on) {
  static byte toPageSafety;
  if (on) {
    safety = 1;
    setPIDMode(0);
    if (!toPageSafety) {
      goToPage("Safety");
      toPageSafety = 1;
    }
  } else {
    safety = 0;
    setPIDMode(fanMode);
    toPageSafety = 0;
  }
}

void updateFanPower() {
  if (fanMode == 1 && !safety) {
    fanPower = constrain(temperatureErrorFeedback(), 0, MAX_PWM);
    if (fanPower >= 1) {
      fanPower = map(fanPower, 0, MAX_PWM, fanPWMMin, fanPWMMax);
    }
  } else if (!safety) {
    fanPower = fanPowerManual;
  } else {
    fanPower = MAX_PWM;
  }
  analogWrite25k(FAN_PWM_PIN, fanPower);
}

void computeRPM() {
  if (counter >= 6 || (micros() - lastRPMUpdate) > (updateTimeMax * 6.5)) { // No overflow problem using this expression
    //Update RPM every 6 counts, increase this for better RPM resolution,
    //decrease for faster update
    float newRPM = (counter * 1e6 * 60) / ((micros() - lastRPMUpdate) * PULSE_PER_REVOLUTION);
    // fan RPM Smoothing
    if (abs(newRPM / fanRPM - 1) < 0.1) {
      fanRPM = round(coeff * newRPM + (1 - coeff) * fanRPM);
    } else {
      fanRPM = newRPM;
    }
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
  static int lastValue[2];
  int power = roundInt(fanPower, 100, 320);
  if (power != lastValue[0]) {
    lastValue[0] = power;
    printVal(F("Home.fanSlider"), power, 0);
    printFloatTxt(F("Home.fanPower"), power, 0, "%");
  }
  if (fanRPM != lastValue[1]) {
    lastValue[1] = fanRPM;
    printFloatTxt(F("Home.fanRPM"), fanRPM, 0, " RPM");
    fanDisplay();
  }
}

void fanDisplay() {
  if (fanRPM != 0) {
    printEn(F("Home.fanRotate"), 1);
    fanIncrement();
  } else {
    printEn(F("Home.fanRotate"), 0);
  }
}

void fanIncrement() {
  byte increment = 1;
  if (fanPower < 65) {
    increment = 1;
  } else if (fanPower < 129) {
    increment = 2;
  } else if (fanPower < 193) {
    increment = 3;
  } else if (fanPower < 257) {
    increment = 4;
  } else {
    increment = 6;
  }
  printVal(F("Home.fanIncrement"), increment, 0);
}

void sendFanSettings() {
  // Fan page
  printVal(F("Fan.mode"), fanMode, 0);
  printVal(F("Fan.min"), roundInt(fanPWMMin, 100, 320), 0);
  printVal(F("Fan.max"), roundInt(fanPWMMax, 100, 320), 0);
  printVal(F("Fan.power"), roundInt(fanPowerManual, 100, 320), 0);
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
