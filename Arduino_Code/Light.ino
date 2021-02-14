const byte NUMBER_OF_LIGHTS = 2;
const byte LIGHT_PIN[NUMBER_OF_LIGHTS] = {5, 6};
const float expFilterValues[3] = {1, 0.4, 0.15};

byte lightDoorOpen = 1;  // to save
byte lightDoorClosed = 1;  // to save
byte lightTransition = 1;  // to save

int defaultBrightness = 255;  // to save
float targetBrightness[NUMBER_OF_LIGHTS] = {255, 255};
float currentBrightness[NUMBER_OF_LIGHTS] = {255, 255};

void setupLight() {
  for (int i = 0; i < NUMBER_OF_LIGHTS; i++) {
    pinMode(LIGHT_PIN[i], OUTPUT);
  }
}

void setBrightness(int newBrightness) {
  defaultBrightness = round((float) newBrightness * 255 / 100);
  EEPROM.put(24, defaultBrightness);
}

void setTransition(byte newTransition) {
  lightTransition = newTransition;
  EEPROM.put(23, lightTransition);
}

void setDoorOpen(byte newState) {
  lightDoorOpen = newState;
  EEPROM.put(21, lightDoorOpen);
}

void setDoorClosed(byte newState) {
  lightDoorClosed = newState;
  EEPROM.put(22, lightDoorClosed);
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
      brightnessChanging = 0;
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
  Serial.print(F("Home.lightPower.txt=\""));
  Serial.print(round((float) defaultBrightness * 100 / 255));
  Serial.print(F("%\""));
  writeFF();

  // Light page
  if (!lightDoorOpen) {
    Serial.print(F("Light.openCrop.picc=12"));
    writeFF();
  }
  if (!lightDoorClosed) {
    Serial.print(F("Light.closedCrop.picc=12"));
    writeFF();
  }
  Serial.print(F("Light.transition.val="));
  Serial.print(lightTransition);
  writeFF();
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
