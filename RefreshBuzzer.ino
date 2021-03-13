unsigned long BuzzLastTime = 0;
unsigned long buzzDelay = 5 * 60000;
boolean buzzOn = false;
uint8_t alert = 0x00;
#define buzzDuration 40
void RefreshBuzzer() {
  if (millis() - BuzzLastTime < buzzDuration) {
    return;
  }
  else {
    alert =  stationsAlert[0] | stationsAlert[1] | stationsTimeout[0] | stationsTimeout[1];
    //
  }
  if (alert != 0x00 ) {

    if (!buzzOn && (millis() - BuzzLastTime > buzzDelay)) {
#ifdef debugOn
 //    Serial.println(alert, HEX);
      Serial.println("buzz on");
#endif
      digitalWrite(buzzPin, 1);
      BuzzLastTime = millis();
      buzzOn = true;
    }
    if (buzzOn && (millis() - BuzzLastTime >= buzzDuration)) {
#ifdef debugOn
 //     Serial.println(alert, HEX);
      Serial.println("buzz off");
#endif
      digitalWrite(buzzPin, 0);
      buzzOn = false;
    }
  }
  else {
    digitalWrite(buzzPin, 0);
  }
}
