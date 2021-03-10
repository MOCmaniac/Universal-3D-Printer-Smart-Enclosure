#include <EEPROM.h>

void setup() {
  Serial.begin(9600); // Default baudrate of the serial monitor
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("Resetting the EEPROM");
  resetMemory();
  Serial.println("Done !\n");
  Serial.println("You can now upload the \"Arduino_Code\" to the Arduino");
}

void loop() {
  byte t = 100;
  // Blink to indicate that the memory reset is done
  digitalWrite(LED_BUILTIN, HIGH);
  delay(t);
  digitalWrite(LED_BUILTIN, LOW);
  delay(t);

  digitalWrite(LED_BUILTIN, HIGH);
  delay(t);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}

void resetMemory() {
  EEPROM.put(0, 1); // tempMode
  EEPROM.put(1, 1); // tempUnit
  EEPROM.put(2, 25.0); // tempRequested
  EEPROM.put(6, 5.0); // tempDelta
  EEPROM.put(10, 1); // fanMode
  EEPROM.put(11, 35); // fanPWMMin
  EEPROM.put(13, 320); // fanPWMMax
  EEPROM.put(15, 0); // fanPowerManual
  EEPROM.put(17, 1); // lightDoorOpen
  EEPROM.put(18, 1); // lightDoorClosed
  EEPROM.put(19, 1); // lightTransition
  EEPROM.put(20, 0); // defaultBrightness
}
