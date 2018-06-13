void step(int current_step) {
  int i = current_step % 4;
  digitalWrite(motorPin1, (i == 0) ? HIGH : LOW);
  digitalWrite(motorPin2, (i == 1) ? HIGH : LOW);
  digitalWrite(motorPin3, (i == 2) ? HIGH : LOW);
  digitalWrite(motorPin4, (i == 3) ? HIGH : LOW);
}

void turn_off_motor() {
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
  digitalWrite(motorPin3, LOW);
  digitalWrite(motorPin4, LOW);
}

void setup_motor() {
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);
}
