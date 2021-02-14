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
      newFanPower(value);
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

void sendValues() {
  sendFanValues();
  sendTempValues();
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

void writeFF() {
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}
