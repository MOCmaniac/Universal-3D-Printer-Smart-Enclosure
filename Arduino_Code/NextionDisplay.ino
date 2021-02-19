void handleDisplayInput(char* string) {
  char command;
  int value;
  sscanf(string, "%c-%d", &command, &value);

  switch (command) {
    case 'b':
      setBrightness(value);
      break;
    case 'd':
      setDoorOpen(value);
      break;
    case 'D':
      setDoorClosed(value);
      break;
    case 'f':
      setFanMode(value);
      break;
    case 'F':
      setFanPower(value);
      break;
    case 'g':
      graphActive = 0;
      break;
    case 'G':
      graphActive = 1;
      break;
    case 'h':
      homeScreenActive = 0;
      break;
    case 'H':
      homeScreenActive = 1;
      break;
    case 'i':
      sendSettings();
      break;
    case 'l':
      setTransition(value);
      break;
    case 'm':
      setPWMMin(value);
      break;
    case 'M':
      setPWMMax(value);
      break;
    case 't':
      setTempMode(value);
      break;
    case 'T':
      setTemp(value);
      break;
    case 'u':
      setUnit(value);
      break;
    default:
      //Serial.print(F("default");
      break;
  }
}

void sendSettings() {
  Serial.print(F("Welcome.initialization.en=0"));
  writeFF();

  sendTempSettings();
  sendFanSettings();
  sendLightSettings();

  // Enable timer to switch to page Home
  Serial.print(F("toHome.en=1"));
  writeFF();
}

void goToPage(char* pageName) {
  Serial.print(F("page "));
  Serial.print(pageName);
  writeFF();
}

void printAdd(byte channel, byte value) {
  Serial.print(F("add 4,"));
  Serial.print(channel);
  Serial.print(F(","));
  Serial.print(value);
  writeFF();
}

void printTxt(const __FlashStringHelper* name, float value, byte precision, char* unit) {
  Serial.print(name);
  Serial.print(F(".txt=\""));
  Serial.print(value, precision);
  Serial.print(unit);
  Serial.print(F("\""));
  writeFF();
}

void printVal(const __FlashStringHelper* name, float value, byte precision) {
  Serial.print(name);
  Serial.print(F(".val="));
  Serial.print(value, precision);
  writeFF();
}

void writeFF() {
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}
