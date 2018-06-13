void writeStepsToEeprom() {
  // + 1 for validity check
  byte low = lowByte(current_steps + 1);
  byte high = highByte(current_steps + 1);
  Serial.print("Writing to EEPROM: ");
  Serial.println(current_steps);
  EEPROM.write(0, low);
  EEPROM.write(1, high);
  EEPROM.commit();
}

void loadStepsFromEeprom() {
  byte low = EEPROM.read(0);
  byte high = EEPROM.read(1);
  int steps = word(high, low) + 1;
  if (steps < 0 || steps > STEPS) {
    Serial.println("Wrong value in EEPROM, not using it");
  } else {
    current_steps = steps;
    Serial.print("Read from EEPROM: ");
    Serial.println(steps);
  }
}
