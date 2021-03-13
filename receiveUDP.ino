

void InputUdpAsGateway() {
  /*
     command received for the gateway function
  */
#define commandBytePosition 7
#define crcLen 2
  int packetSize = UdpAsGateway.parsePacket();  // message from UDP
  if (packetSize)
  {
    IPAddress remote = UdpAsGateway.remoteIP();
#if defined(debugOn)
    Serial.print("Master Udp RSize ");
    Serial.print(packetSize);
    Serial.print(": ");
    for (int i = 0; i < 4; i++)
    {
      Serial.print(remote[i], DEC);
      if (i < 3)
      {
        Serial.print(".");
      }
    }

    Serial.print(",port ");
    Serial.print(UdpAsGateway.remotePort());
    Serial.print(" ");
#endif
    UdpAsGateway.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    int packetBufferLen = packetSize;
    byte inputCRC = packetBuffer[packetSize - 1];
    byte computeCRC = CRC8(&packetBuffer[commandBytePosition], packetSize - commandBytePosition - crcLen);
    if (inputCRC != computeCRC)
    {
      bitWrite(diagByte, diagServerConnexion, 1);
#if defined(debugOn)
      Serial.println("crcError");
      Serial.print("in CRC:");
      Serial.print(inputCRC, HEX);
      Serial.print("compute CRC:");
      Serial.println(computeCRC, HEX);
#endif
      return;
    }
    else {
      bitWrite(diagByte, diagServerConnexion, 0);
      bitWrite(diagByte, serverTimeout, 0);
      lastUDPreceivedTime = millis();
    }
    switch (packetBuffer[commandBytePosition]) {
      case statusRequest :             //
        {
#if defined(debugOn)
          Serial.println("req status");
#endif
          SendStatus();
          break;
        }
      case serviceInfoRequest :             //
        {
#if defined(debugOn)
          Serial.print("service info:");
#endif
          unsigned port = GetUnsignedValue(packetBuffer[9], packetBuffer[10]);
          if (packetBuffer[8] == 0x00)
          {
            RemotePortForStation = port;
            RemoteIPForStation = UdpAsGateway.remoteIP(); // update remote IP address
#if defined(debugOn)
            Serial.print(RemoteIPForStation);
            Serial.print("/");
            Serial.println(RemotePortForStation);
#endif
          }
          if (packetBuffer[8] == 0x01)
          {
            RemotePortForGateway = port;
            RemoteIPForGateway = UdpAsGateway.remoteIP(); // update remote IP address
#if defined(debugOn)
            Serial.print(RemoteIPForGateway);
            Serial.print("/");
            Serial.println(RemotePortForGateway);
#endif
          }
          break;
        }
    }
  }
}
int GetValue(byte one, byte two) {
  return ((one << 8) & 0x7fff) + (int(two) & 0x00ff);
}
unsigned int GetUnsignedValue(byte one, byte two) {
  return ((one << 8) ) + (int(two) & 0x00ff);
}

void InputUdpAsStation() {
  /*
     command received for a station
  */
#define commandBytePosition 7
#define crcLen 2
  int packetSize = UdpAsStation.parsePacket();  // message from UDP
  if (packetSize)
  {
    IPAddress remote = UdpAsStation.remoteIP();
#if defined(debugOn)
    Serial.print("Station Udp RSize ");
    Serial.print(packetSize);
    Serial.print(": ");
    for (int i = 0; i < 4; i++)
    {
      Serial.print(remote[i], DEC);
      if (i < 3)
      {
        Serial.print(".");
      }
    }
    Serial.print(",port ");
    Serial.print(UdpAsStation.remotePort());
    Serial.print(" ");
#endif
    UdpAsStation.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    int packetBufferLen = packetSize;
    byte inputCRC = packetBuffer[packetSize - 1];
    byte computeCRC = CRC8(&packetBuffer[commandBytePosition], packetSize - commandBytePosition - crcLen);
    if (inputCRC != computeCRC)
    {
      bitWrite(diagByte, diagServerConnexion, 1);
#if defined(debugOn)
      Serial.println("crcError");
      Serial.print("in CRC:");
      Serial.print(inputCRC, HEX);
      Serial.print("compute CRC:");
      Serial.println(computeCRC, HEX);
#endif
      return;
    }
    else {
      bitWrite(diagByte, diagServerConnexion, 0);
#if defined(debugOn)
      Serial.print("forward len:");
      Serial.println(packetBufferLen - rfStartPosition - 1);
#endif

      SendData(packetBuffer[rfStationPosition], PRfpacketBuffer, packetBufferLen - rfStartPosition - crcLen - 1);
    }
#if defined(debugOn)
    Serial.print("cmde:");
    Serial.println(packetBuffer[commandBytePosition], HEX);
#endif

  }
}
