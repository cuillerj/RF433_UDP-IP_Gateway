/*
   configuration to set once in the eeprom
*/


uint8_t newID[2] = {0xfb, 0x00};
uint8_t macaddr[6] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x05
}; // must be unique on the LAN
uint8_t ipaddr[4] = {
  0xC0, 0xA8, 0x01, 0xfa
};       //    must be unique on the network
uint8_t ipmask[4] = {
  0xFF, 0xFF, 0xFF, 0x00
};
uint8_t ipgateway[4] = {
  0xC0, 0xA8, 0x00, 0x01
};
uint8_t dnsAddr[4] = {
  0xC0, 0xA8, 0x00, 0x01
};

void InitEeprom() {
  PrintEeprom();
  Serial.print("new ID:");
  Serial.print(newID[0], HEX);
  Serial.print("-");
  Serial.print(newID[1], HEX);
  Serial.print(" ");
  unsigned stationID = int(256 * newID[0]) +  int(newID[1]);
  Serial.println(stationID);
  Serial.println("mise a jour eeprom dans 30 secondes");
  delay(30000);

  for (int i = 0; i < sizeof(newID); i++) {
    EEPROM.update(addrid + i, newID[i]);
  }
  for (int i = 0; i < sizeof(macaddr); i++) {
    EEPROM.update(addrmac + i, macaddr[i]);
  }
  for (int i = 0; i < sizeof(ipaddr); i++) {
    EEPROM.update(addrip + i, ipaddr[i]);
  }
  for (int i = 0; i < sizeof(ipmask); i++) {
    EEPROM.update(addrmask + i, ipmask[i]);
  }
  for (int i = 0; i < sizeof(ipgateway); i++) {
    EEPROM.update(addrgateway + i, ipgateway[i]);
  }
    for (int i = 0; i < sizeof(dnsAddr); i++) {
    EEPROM.update(addrdynDns + i, dnsAddr[i]);
  }
  Serial.println("eeprom updated > power off the UNO, remove MOSI PIN on ICSP, add the ethernet shield and reboot");
  delay(60 * 60000);
}

void PrintEeprom() {
  int startAddress = 0;
  Serial.println("eeprom conf");
  Serial.print("ID: 0x");
  unsigned int stationID = EEPROM.read(startAddress);
  Serial.print(EEPROM.read(addrid), HEX);
  Serial.print("-");
  Serial.print(EEPROM.read(addrid + 1), HEX);
  Serial.print(" ");
  stationID = stationID * 256 + EEPROM.read(startAddress + 1);
  Serial.println(stationID);

  Serial.print("MAC addr:");

  for (int i = 0; i < 6; i++)
  {
    Serial.print(EEPROM.read(addrmac + i), HEX);
    Serial.print("-");
  }
  Serial.println();
  Serial.print("IP addr:");
  for (int i = 0; i < 4; i++)
  {
    Serial.print(EEPROM.read(addrip + i));
    Serial.print("-");
  }
  Serial.println();
  Serial.print("IP mask:");
  for (int i = 0; i < 4; i++)
  {
    Serial.print(EEPROM.read(addrmask + i));
    Serial.print("-");
  }
  Serial.println();
  Serial.print("IP gw:");
  for (int i = 0; i < 4; i++)
  {
    Serial.print(EEPROM.read(addrdynDns+ i));
    Serial.print("-");
  }
  Serial.println();
  Serial.print("IP dns:");
  for (int i = 0; i < 4; i++)
  {
    Serial.print(EEPROM.read(addrdynDns + i));
    Serial.print("-");
  }
  Serial.println();
}
