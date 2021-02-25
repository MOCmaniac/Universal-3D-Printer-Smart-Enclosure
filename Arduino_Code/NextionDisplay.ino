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
      graphPageActive = value;
      break;
    case 'h':
      homePageActive = value;
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
  printEn(F("Startup.initialization"),0);
  printEn(F("timerLoading"), 0);

  sendEnvironmentSettings();
  sendFanSettings();
  sendLightSettings();

  // Enable timer to switch to page Home
  printEn(F("toHome"), 1);
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

void printEn(const __FlashStringHelper* name, byte enable){
  Serial.print(name);
  Serial.print(F(".en="));
  Serial.print(enable);
  writeFF();
}

void writeFF() {
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}
