
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
#ifndef useDHCP
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
#endif
  Serial.println("reload without initEeprom !");
  while (true) {};
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
#ifndef useDHCP
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
    Serial.print(EEPROM.read(addrdynDns + i));
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
#endif
}
