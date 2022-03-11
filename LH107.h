/*
   configuration to set once in the eeprom
*/
#ifdef netConfig
#error "duplicate eeprom config"
#else netConfig
#warning "LH107 config"
#define netConfig
#define SpeedNetw  2000 // vitessse reception
uint8_t newID[2] = {0xfe, 0x00};
uint8_t macaddr[6] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x05
}; // must be unique on the LAN
#ifndef useDHCP
uint8_t ipaddr[4] = {
  0xC0, 0xA8, 0x01, 0xc9
};       //    must be unique on the network
uint8_t ipmask[4] = {
  0xFF, 0xFF, 0xFF, 0x00
};
uint8_t ipgateway[4] = {
  0xC0, 0xA8, 0x01, 0x01
};
uint8_t dnsAddr[4] = {
  0xC0, 0xA8, 0x01, 0x01
#endif
};

#endif
