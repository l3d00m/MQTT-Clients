// EEPROM addresses, only use
#define current_steps_address 0
#define up_speed_address 2
#define max_steps_address 4
#define down_speed_address 6

// Number to divide the steps before saving to eeprom (steps are usually at around 40k, which is high enough that the exact result doesn't matter)
// This allows to store bigger values in EEPROM with just 16 bytes
#define eeprom_steps_resolution 10

#define default_up_rpm 10 // default value for up speed
#define default_down_rpm default_up_rpm + 4 // the default down rpm (down speed should be faster by default)
#define default_max_steps 40000

int loadIntFromEeprom(int pos) {
  byte low = EEPROM.read(pos);
  byte high = EEPROM.read(pos + 1);
  int value = word(high, low) - 1;
  Serial.print("Read from EEPROM: ");
  Serial.println(value);
  return value;
}

void writeIntToEeprom(int input, int pos) {
  // + 1 to make validity checks with > 0 possible as we can't store negative values if it's invalid
  input = input + 1;
  byte low = lowByte(input);
  byte high = highByte(input);
  Serial.print("Writing to EEPROM (+1 for validity is added): ");
  Serial.println(input);
  EEPROM.write(pos, low);
  EEPROM.write(pos + 1, high);
  EEPROM.commit();
}

void clear_all_eeprom() {
  // Fill all of our eeprom with zero, this function isn't called anywhere yet
  EEPROM.write(current_steps_address, 0);
  EEPROM.write(current_steps_address + 1, 0);
  EEPROM.write(up_speed_address, 0);
  EEPROM.write(up_speed_address + 1, 0);
  EEPROM.write(down_speed_address, 0);
  EEPROM.write(down_speed_address + 1, 0);
  EEPROM.write(max_steps_address, 0);
  EEPROM.write(max_steps_address + 1, 0);
  EEPROM.commit();
}

void loadUpSpeedFromEeprom() {
  int speed_loaded = loadIntFromEeprom(up_speed_address);
  if (speed_loaded <= 0 || speed_loaded > 100) {
    Serial.println("Wrong value for speed conf in EEPROM, using default");
    up_rpm = default_up_rpm;
  } else {
    up_rpm = speed_loaded;
    Serial.print("Read speed config from EEPROM: ");
    Serial.println(speed_loaded);
  }
}

void writeUpSpeedToEeprom() {
  writeIntToEeprom(up_rpm, up_speed_address);
}

void loadDownSpeedFromEeprom() {
  int speed_loaded = loadIntFromEeprom(down_speed_address);
  if (speed_loaded <= 0 || speed_loaded > 100) {
    Serial.println("Wrong value for down speed conf in EEPROM, using normal speed");
    down_rpm = default_down_rpm;
  } else {
    down_rpm = speed_loaded;
    Serial.print("Read speed config from EEPROM: ");
    Serial.println(speed_loaded);
  }
}

void writeDownSpeedToEeprom() {
  writeIntToEeprom(down_rpm, down_speed_address);
}

void loadMaxStepsFromEeprom() {
  int steps = loadIntFromEeprom(max_steps_address) * eeprom_steps_resolution;
  if (steps <= 100 || steps > 400000) {
    MAX_STEPS = default_max_steps;
    Serial.println("Wrong value for default steps in EEPROM, using default");
  } else {
    MAX_STEPS = steps;
    Serial.print("Read step config from EEPROM: ");
    Serial.println(steps);
  }
}

void writeMaxStepsToEeprom() {
  writeIntToEeprom(MAX_STEPS / eeprom_steps_resolution, max_steps_address);
}

void writeCurrentStepsToEeprom() {
  writeIntToEeprom(current_steps / eeprom_steps_resolution, current_steps_address);
}

void loadCurrentStepsFromEeprom() {
  int steps_loaded = loadIntFromEeprom(current_steps_address) * eeprom_steps_resolution;
  if (steps_loaded < 0 || steps_loaded > MAX_STEPS) {
    Serial.println("Wrong value for current steps in EEPROM, using max steps!!");
    current_steps = MAX_STEPS;
  } else {
    current_steps = steps_loaded;
    Serial.print("Read current steps from EEPROM: ");
    Serial.println(steps_loaded);
  }
}
