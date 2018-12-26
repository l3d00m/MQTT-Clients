int loadIntFromEeprom(int pos) {
  byte low = EEPROM.read(pos);
  byte high = EEPROM.read(pos + 1);
  int value = word(high, low) - 1;
  Serial.print("Read from EEPROM: ");
  Serial.println(value);
  return value;
}

void writeIntToEeprom(int input, int pos) {
  // + 1 for validity check
  byte low = lowByte(input + 1);
  byte high = highByte(input + 1);
  Serial.print("Writing to EEPROM: ");
  Serial.println(input);
  EEPROM.write(pos, low);
  EEPROM.write(pos + 1, high);
  EEPROM.commit();
}

void loadSpeedFromEeprom() {
  int speed = loadIntFromEeprom(2);
  if (speed <= 0 || speed > 100) {
    Serial.println("Wrong value for speed conf in EEPROM, using default");
    rpm = INIT_RPM;
  } else {
    rpm = speed;
    Serial.print("Read speed config from EEPROM: ");
    Serial.println(speed);
  }
}

void writeSpeedToEeprom() {
  writeIntToEeprom(rpm, 2);
}

void loadDefaultStepsFromEeprom() {
  int steps = loadIntFromEeprom(4);
  if (steps <= 0 || steps > 200000) {
    Serial.println("Wrong value for default steps in EEPROM, using default");
  } else {
    STEPS = steps;
    Serial.print("Read step config from EEPROM: ");
    Serial.println(steps);
  }
}

void writeDefaultStepsToEeprom() {
  writeIntToEeprom(STEPS, 4);
}

void writeCurrentStepsToEeprom() {
  writeIntToEeprom(current_steps, 0);
}

void loadCurrentStepsFromEeprom() {
  int steps = loadIntFromEeprom(0);
  if (steps < 0 || steps > STEPS) {
    Serial.println("Wrong value for current steps in EEPROM, using 0!!");
  } else {
    current_steps = steps;
    Serial.print("Read current steps from EEPROM: ");
    Serial.println(steps);
  }
}
