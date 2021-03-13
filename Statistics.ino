#ifdef printStat
void PrintStatistics() {
  Serial.print("total RF traffic:");
  Serial.print(RFTraffic);
  Serial.print(" Rf received:");
  Serial.print(RFinputFrame);
  Serial.print(" Ignored:");
  Serial.print(IgnoredTrame);
  Serial.print(" station error:");
  Serial.println(RFstationError);

  for (int i = 0; i < sizeof(ListStations); i++) {
    Serial.print("station:");
    Serial.print(ListStations[i]);
    Serial.print(" In:");
    Serial.print(TrafficStationsIn[i]);
    Serial.print(" Out:");
    Serial.print(TrafficStationsOut[i]);

    Serial.print(" Missed:");
    Serial.println(RFframeMissed[i]);
  }
}
#endif
