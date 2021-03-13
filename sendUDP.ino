//   GatewayLink.PendingDataReqSerial[frameLen + 1] = CRC8(GatewayLink.PendingDataReqSerial , frameLen);

void dataUDPStation(byte *msg2, int mlen) {
  Serial.print("send IP:");
  Serial.print(RemoteIPForStation);
  Serial.print("/");
  Serial.print(RemotePortForStation);
  Serial.print(">");
  Serial.println(mlen);
  UdpAsStation.beginPacket(RemoteIPForStation, RemotePortForStation);
  mlen = max(mlen, minUdpSize);
  UdpAsStation.write(msg2, mlen);
  // UdpAsStation.endPacket();
  if (!UdpAsStation.endPacket()) {
    Serial.print(" error");
  }
  else {
    Serial.println();
  }
}
void dataUDPStation2(int mlen) {
  Serial.print("send IP:");
  Serial.print(RemoteIPForStation);
  Serial.print("/");
  Serial.print(RemotePortForStation);
  Serial.print(">");
  Serial.println(mlen);
  UdpAsStation.beginPacket(RemoteIPForStation, RemotePortForStation);
  mlen = max(mlen, minUdpSize);
  UdpAsStation.write(dataBin, mlen);
  // UdpAsStation.endPacket();
  if (!UdpAsStation.endPacket()) {
    Serial.print(" error");
  }
  else {
    Serial.println();
  }
}
void dataUDPGateway(byte *msg2, int mlen ) {
  mlen = max(mlen, minUdpSize);
  Serial.print("send IP:");
  Serial.print(RemoteIPForGateway);
  Serial.print("/");
  Serial.print(RemotePortForGateway);
  Serial.print(">");
  Serial.print(mlen);
  UdpAsGateway.beginPacket(RemoteIPForGateway, RemotePortForGateway);
  UdpAsGateway.write(msg2, mlen);
  if (!UdpAsGateway.endPacket()) {
    Serial.print(" error");
  }
  else {
    Serial.println();
  }
}

void SendStatus() {
#if defined(debugOn)
  Serial.println("status");
#endif
  dlen = 34;
  dataBin[0] = 0x4D; // M master
  dataBin[1] = 0x00;
  dataBin[2] = uint8_t(dlen);
  dataBin[3] = 0x00;
  dataBin[4] = addrS;
  dataBin[5] = 0x00;
  dataBin[6] = statusResponse; // statut
  dataBin[7] =  diagByte; // statut actif
  dataBin[8] = 0xff; //
  dataBin[9] = Version;
  unsigned int minuteSinceReboot = (millis() / 60000);
  dataBin[10] = uint8_t(minuteSinceReboot / 256);
  dataBin[11] = uint8_t(minuteSinceReboot);
  dataBin[12] = uint8_t(RemoteIPForGateway[0]);
  dataBin[13] = uint8_t(RemoteIPForGateway[1]);
  dataBin[14] = uint8_t(RemoteIPForGateway[2]);
  dataBin[15] = uint8_t(RemoteIPForGateway[3]);
  dataBin[16] = 0x00;
  dataBin[17] = stationsAlert[0] | stationsTimeout[0];
  dataBin[18] = stationsAlert[1] | stationsTimeout[1];
  dataBin[19] = 0x00;

  int idx = 20;
  for (int i = 0; i < min(sizeof(ActiveStations), 12); i++) {
    dataBin[idx] = ActiveStations[i];
    idx++;
  }
  for (int i = idx; i < dlen ; i++) {
    dataBin[i] = 0x00;
  }

  dataBin[dlen] = CRC8(dataBin, dlen);
  dataUDPGateway(dataBin, dlen + 1); // service 0
  // dataUDPGateway2( dlen + 1); // service 0
  timeStatusSent = millis();
}
/*
void SendStationsList() {
  timeStatusListSent = millis();
#if defined(debugOn)
  Serial.println("list");
#endif

  dlen = 21;
  dataBin[0] = 0x4D; // M master
  dataBin[1] = 0x00;
  dataBin[2] = uint8_t(dlen);
  dataBin[3] = 0x00;
  dataBin[4] = addrS;
  dataBin[5] = 0x00;
  dataBin[6] = indicatorsRequest; // statut
  int idx = 7;
  for (int i = 0; i < min(sizeof(ActiveStations), 12); i++) {
    dataBin[idx] = ActiveStations[i];
    idx++;
  }
  for (int i = idx; i < dlen ; i++) {
    dataBin[i] = 0x00;
  }
  dataBin[dlen] = CRC8(dataBin, dlen);
  return;
  // dataUDPGateway(dataBin, dlen + 1); // service 0
  dataUDPGateway2( dlen + 1); // service 0
}
*/
void SendRFGatewayStatistics() {
#if defined(debugOn)
  Serial.println("G stat");
#endif
  dlen = 24;
  dataBin[0] = 0x4D; // M master
  dataBin[1] = 0x00;
  dataBin[2] = uint8_t(dlen);
  dataBin[3] = 0x00;
  dataBin[4] = addrS;
  dataBin[5] = 0x00;
  dataBin[6] = insertDataInDatabaseRequest; // statut
  dataBin[7] = 0x00; // type of mersurment
  dataBin[8] = 0x2b;
  dataBin[9] = 0x00;
  dataBin[10] = 0x00;
  dataBin[11] = 0x2b;
  dataBin[12] = uint8_t((RFTraffic & 0xff00) >> 8);
  dataBin[13] = uint8_t(RFTraffic & 0x00ff );
  dataBin[14] = 0x2b;
  dataBin[15] = uint8_t((RFinputFrame & 0xff00) >> 8);
  dataBin[16] = uint8_t(RFinputFrame & 0x00ff );
  dataBin[17] = 0x2b;
  dataBin[18] = uint8_t((IgnoredTrame & 0xff00) >> 8);
  dataBin[19] = uint8_t(IgnoredTrame & 0x00ff );
  dataBin[20] = 0x2b;
  dataBin[21] = uint8_t((RFstationError & 0xff00) >> 8);
  dataBin[22] = uint8_t(RFstationError & 0x00ff );
  dataBin[23] = 0x00;
  dataBin[dlen] = CRC8(dataBin, dlen);
  dataUDPGateway(dataBin, dlen + 1); // service 0
  timeStatisticsSent = millis();
  statisticsFlag = 0x00;
}
void SendRFStationStatistics() {
  Serial.println("S stat");
  if (statisticsFlag >= sizeof(ActiveStations)) {
    statisticsFlag = 0xff;
    return;
  }
  if (ActiveStations[statisticsFlag] == 0x00) {
    statisticsFlag++;
    return;
  }
  dlen = 21;
  dataBin[0] = 0x4D; // M master
  dataBin[1] = 0x00;
  dataBin[2] = uint8_t(dlen);
  dataBin[3] = 0x00;
  dataBin[4] = addrS;
  dataBin[5] = 0x00;
  dataBin[6] = insertDataInDatabaseRequest; // statut
  dataBin[7] = 0x01; // type of data
  dataBin[8] = 0x2b;
  dataBin[9] = 0x00;
  dataBin[10] = ActiveStations[statisticsFlag];
  dataBin[11] = 0x2b;
  dataBin[12] = uint8_t((TrafficStationsIn[statisticsFlag] & 0xff00) >> 8);
  dataBin[13] = uint8_t(TrafficStationsIn[statisticsFlag] & 0x00ff );
  dataBin[14] = 0x2b;
  dataBin[15] = uint8_t((TrafficStationsOut[statisticsFlag] & 0xff00) >> 8);
  dataBin[16] = uint8_t(TrafficStationsOut[statisticsFlag]  & 0x00ff );
  dataBin[17] = 0x2b;
  dataBin[18] = uint8_t((RFframeMissed[statisticsFlag] & 0xff00) >> 8);
  dataBin[19] = uint8_t(RFframeMissed[statisticsFlag] & 0x00ff );
  dataBin[20] = 0x00;
  dataBin[dlen] = CRC8(dataBin, dlen);
  dataUDPGateway(dataBin, dlen + 1); // service 0
  statisticsFlag++;
  timeStatisticsSent = millis();
}
