
void TrameAnalyzeRFInput() {
#define RFheaderLen 5
#define UdpheaderLen 4

  char c = 0;
  uint8_t station = 0x00;
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  uint8_t RFaddrE = 0x00;
  uint8_t RFaddrR = 0x00;
  uint8_t RFcmde = 0x00;
  uint8_t RFdlen = 0x00;
  uint8_t RFcount = 0x00;

  if (vw_get_message(buf, &buflen)) {
    RFTraffic++;
    for (int i = RFheaderLen; i < buflen; i++) {
      DataIn[i - RFheaderLen] = buf[i];
    }
    RFaddrE = buf[0];
    RFaddrR = buf[1];
    RFcmde = buf[2];
    RFcount = buf[3];
    RFdlen = buf[4];
    if (RFaddrR == addrS) { // is this gateway the destination ?
      RFinputFrame++;
#if defined(debugOn)
      for (int i = 0; i < RFdlen + RFheaderLen; i++)
      {
        Serial.print(buf[i], HEX);
        Serial.print("-");
      }
      Serial.println("");
#endif
      //    station = uint8_t(sizeof(ListStations) + 1);
      boolean foundStation = false;
      uint8_t station = FindStationIndex(RFaddrE);
      if (station != 0xff) {
        foundStation = true;              // known station
        if(ActiveStations[station]==0x00){
          frameCountStation[station]=RFcount-1;  // to synchronise count with station
        }
        ActiveStations[station] = RFaddrE;  // store station address in active list
        lastRFreceivedTime[station]=millis();
        TrafficStationsIn[station] = TrafficStationsIn[station] + 1;
      }
      if (!foundStation) {                  // station not found
        RFstationError++;
//#if defined(debugOn)
        Serial.print("not found:");
        Serial.println(RFaddrE, HEX);
//#endif
        return;
      }
      else {
//#if defined(debugOn)
        Serial.print("found:");
        Serial.println(RFaddrE, HEX);
//#endif
        lastRFreceivedTime[station]=millis();
        if (frameCountStation[station] == RFcount)
        {
#if defined(debugOn)
          Serial.print(" ignored:");
          Serial.println(RFcount, HEX);
#endif
          return;
        }
        else {
          if (frameCountStation[station] + 1 != RFcount) {
            if(frameCountStation[station] + 1 < RFcount)
            {
              RFframeMissed[station]=RFframeMissed[station]+RFcount-frameCountStation[station] ;
            }
              else{
                RFframeMissed[station]++;
              }
#if defined(debugOn)
            Serial.print(" missed:");
            Serial.print(frameCountStation[station] + 1, HEX);
#endif
          }
          frameCountStation[station] = RFcount;
          Serial.println();
        }
      }

#if defined(debugOn)
      Serial.print("RF received:");
#endif

      if (ActiveStations[station] != 0x00 ) {
#if defined(debugOn)
        Serial.print("Id:");
        Serial.print(ActiveStations[station]);

        for (uint8_t j = 0; j < dlen; ++j) {
          Serial.print("-0x:");
          Serial.print(DataIn[j], HEX);
        }
        Serial.println("");
#endif
        dlen = 7;
        dataBin[0] = 0x4D; // M master
        dataBin[1] = 0x00;
        dataBin[3] = 0x00;
        dataBin[4] = addrS;
        dataBin[5] = ActiveStations[station];
        dataBin[6] = RFcmde; //
        for (uint8_t j = 0; j < RFdlen; ++j) {
          dataBin[j + 6] = DataIn[j];
          dlen++;
        }
      }
      else {
        ActiveStations[station] = addrE;
#if defined(debugOn)
        Serial.print(Err01);
        Serial.println(addrE, HEX);
#endif
      }
      dataBin[2] = uint8_t(RFdlen + 2);
      dataBin[dataBin[2] + UdpheaderLen] = 0x00;
      byte *Pdata = dataBin;
      dataBin[dataBin[2] + UdpheaderLen + 1] = CRC8((Pdata + UdpheaderLen), dataBin[2]);
      dataUDPStation(dataBin, dataBin[2] + UdpheaderLen + 2);
      if (RFcmde != 0x00) {  // receive a specific flag
        if (RFaddrE <= 8) {
          bitWrite(stationsAlert[0], RFaddrE - 1, 1);
        }
        else {
          bitWrite(stationsAlert[1], RFaddrE - 9, 1);
        }
#if defined(debugOn)
        Serial.print("alert on from:");
        Serial.println(RFaddrE);
#endif
      }
      else {
        if (RFaddrE <= 8) {
          bitWrite(stationsAlert[0], RFaddrE - 1, 0);
        }
        else {
          bitWrite(stationsAlert[1], RFaddrE - 9, 0);
        }
#if defined(debugOn)
        Serial.print("alert off from:");
        Serial.println(RFaddrE);
#endif
      }
    }
    else {
#if defined(debugOn)
      Serial.print("ignored from:");
      Serial.print(RFaddrE);
      Serial.print(" to:");
      Serial.println(RFaddrR);
      for (int i = 0; i < dlen + 5; i++)
      {
        Serial.print(buf[i], HEX);
        Serial.print("-");
      }
      Serial.println("");
#endif
      IgnoredTrame++;
    }
  }
#if defined(debugOn)
  else {
    Serial.println("#");
  }
#endif
}
void SendData(uint8_t station, uint8_t *PData, uint8_t dataLen) {
  uint8_t stationIndex = FindStationIndex(station);
  if (stationIndex == 0xff) {
    RFstationError++;
#if defined(debugOn)
    Serial.print("station not found:");
    Serial.println(station);
#endif
    return;
  }
  #if defined(debugOn)
    Serial.print("send rf:");
    Serial.println(station);
#endif
  dataLen = min(dataLen, _maxDataLen);
  lastSentNumber[stationIndex]++;
  TrafficStationsOut[stationIndex]++;
  _outMsg[0] = addrS;
  _outMsg[1] = station;
  _outMsg[2] = 0x00;
  _outMsg[3] = lastSentNumber[stationIndex];
  for (int i = 0; i < dataLen; i++) {
    PData++;
    _outMsg[i + 5] = *PData;
  }
  _outMsg[4] = dataLen;
  _outLen = dataLen + 5;
  vw_send((uint8_t *)_outMsg, dataLen + 5);
  vw_wait_tx();
  _lastSentTime = millis();
  _retry = 0;
}
void RetrySend() {
  if (millis() > _lastSentTime + random(1500, 2500) && _retry < _nbRetry ) {
    //   Serial.println(_retry);
    _retry++;
    vw_send((uint8_t *)_outMsg, _outLen);
    vw_wait_tx();
    _lastSentTime = millis();
  }
}
uint8_t FindStationIndex(uint8_t address) {
  uint8_t station = 0xff;
  for (int idx = 0; idx < sizeof(ListStations); idx++) {
    if (ListStations[idx] == address) { // is the station an allowaed one
      station = idx;                    // set idx station
    }
  }
  return station;
}
