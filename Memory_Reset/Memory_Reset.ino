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
  EEPROM.put(2, 22.0); // tempRequestedC
  EEPROM.put(6, 22.0); // tempRequestedF
  EEPROM.put(10, 5.0); // tempDelta
  EEPROM.put(14, 1); // fanMode
  EEPROM.put(15, 35); // fanPWMMin
  EEPROM.put(17, 320); // fanPWMMax
  EEPROM.put(21, 1); // lightDoorOpen
  EEPROM.put(22, 1); // lightDoorClosed
  EEPROM.put(23, 1); // lightTransition
  EEPROM.put(24, 0); // defaultBrightness
}
