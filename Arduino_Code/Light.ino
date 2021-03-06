const byte NUMBER_OF_LIGHTS = 2;
const byte LIGHT_PIN[NUMBER_OF_LIGHTS] = {5, 6};
const float expFilterValues[3] = {1, 0.4, 0.15};

byte lightDoorOpen = 1;  // to save
byte lightDoorClosed = 1;  // to save
byte lightTransition = 1;  // to save

int defaultBrightness = 0;  // to save
float targetBrightness[NUMBER_OF_LIGHTS] = {0, 0}; // Must be 0 to avoid a sudden change from 255 to 0
float currentBrightness[NUMBER_OF_LIGHTS] = {0, 0}; // Must be 0 to avoid a sudden change from 255 to 0

void setupLight() {
  for (int i = 0; i < NUMBER_OF_LIGHTS; i++) {
    pinMode(LIGHT_PIN[i], OUTPUT);
    analogWrite(LIGHT_PIN[i], LOW);
  }
}

void setBrightness(int newBrightness) {
  defaultBrightness = roundInt(newBrightness, 255 , 100);
  EEPROM.put(20, defaultBrightness);
}

void setTransition(byte newTransition) {
  lightTransition = newTransition;
  EEPROM.put(19, lightTransition);
}

void setDoorOpen(byte newState) {
  lightDoorOpen = newState;
  EEPROM.put(17, lightDoorOpen);
}

void setDoorClosed(byte newState) {
  lightDoorClosed = newState;
  EEPROM.put(18, lightDoorClosed);
}

void updateBrightness() {
  for (byte i = 0; i < NUMBER_OF_LIGHTS; i++) {
    if (doorIsOpen(i)) {
      if (lightDoorOpen) {
        targetBrightness[i] = defaultBrightness;
      } else {
        targetBrightness[i] = 0;
      }
    } else {
      if (lightDoorClosed) {
        targetBrightness[i] = defaultBrightness;
      } else {
        targetBrightness[i] = 0;
      }
    }
  }
}

void incrementBrightness() {
  for (byte i = 0; i < NUMBER_OF_LIGHTS; i++) {
    if (abs(currentBrightness[i] - targetBrightness[i]) > 1) {
      currentBrightness[i] = (1 - expFilterValues[lightTransition]) * currentBrightness[i] + expFilterValues[lightTransition] * targetBrightness[i];
    } else {
      currentBrightness[i] = targetBrightness[i];
    }
  }
}

void writeBrightness() {
  for (byte i = 0; i < NUMBER_OF_LIGHTS; i++) {
    analogWrite(LIGHT_PIN[i], currentBrightness[i]);
  }
}

void sendLightSettings() {
  // Home page
  printFloatTxt(F("Home.lightPower"), roundInt(defaultBrightness , 100 , 255), 0, "%");

  // Light page
  // Default is OFF on the GUI
  if (lightDoorOpen) {
    Serial.print(F("Light.open.picc=Light.picON.val"));
    writeFF();
  }
  if (lightDoorClosed) {
    Serial.print(F("Light.closed.picc=Light.picON.val"));
    writeFF();
  }
  printVal(F("Light.transition"), lightTransition, 0);
}

void loadLightSettings(byte start) {
  byte address = start;
  EEPROM.get(address, lightDoorOpen);
  address++;
  EEPROM.get(address, lightDoorClosed);
  address++;
  EEPROM.get(address, lightTransition);
  address++;
  EEPROM.get(address, defaultBrightness);
}
