unsigned long RedLedLastTime = 0;
unsigned long GreenLedLastTime = 0;
unsigned int waitGreenLed = 0;
void RefreshLED() {
  if (bitRead(diagByte, diagReboot)) {
    waitGreenLed = 500;
  }
  if (bitRead(diagByte, diagServerConnexion)) {
    waitGreenLed = 1000;
  }
  if ((diagByte & 0b11111110) == 0x00 ) { // ignore reboot bit
    digitalWrite(greenLEDPin, HIGH);
  }
  else if (millis() - GreenLedLastTime > waitGreenLed) {
    GreenLedLastTime = millis();
    digitalWrite(greenLEDPin, !digitalRead(greenLEDPin));
  }

  if (stationsAlert[0] == 0x00 && stationsAlert[1] == 0x00 && stationsTimeout[0]==0x00 && stationsTimeout[1]==0x00) {
    digitalWrite(redLEDPin, LOW);
  }
  else if (millis() - RedLedLastTime > 2000) {
    RedLedLastTime = millis();
    digitalWrite(redLEDPin, !digitalRead(redLEDPin));
  }
}
