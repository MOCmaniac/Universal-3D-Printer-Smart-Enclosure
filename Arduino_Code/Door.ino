const byte DOOR_PIN[2][2] = {{3, 4}, {7, 8}};

byte opened = false;

void setupDoor() {
  for (int i = 0; i < 2; i++) {
    pinMode(DOOR_PIN[i][0], INPUT_PULLUP);
    pinMode(DOOR_PIN[i][1], INPUT_PULLUP);
  }
}

byte doorIsOpen(byte shelf) {
  if (!digitalRead(DOOR_PIN[shelf][0]) || !digitalRead(DOOR_PIN[shelf][1])) {
    return 1;
  }
  return 0;
}

void checkClosing() {
  if (!doorIsOpen(0) && opened) {
    opened = false;
    goToPage("Home");
  }
  opened = doorIsOpen(0);
}
