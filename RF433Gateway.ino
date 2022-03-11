/*
   version 3
   ajout de 2 LED
   buzzer si alerte (switchable)
   service reception d'alerte des stations

   version 4
   ajout timeout receive UDP
   ajout timeout receive RF

   version 5
   suppression envoi liststation et ajout dans status
   version 6 retablissement lecture udp as station
   version 7 modification resolution DNS debug et non bloquante au demarrage
   version 8 print dns
   version 9 frames missed count improvment
   version 10 modif print rf
   version 11 ajout option preprocesseur pour gestion des config et modif update eeprom
   version 12 ajout option DHCP
   version 13 modif diagbyte cas dns et red led
*/
#define Version 13

/*
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  Written by Jean Cuiller
*/

/*
   This software run on Arduino microcontroller (typically an UNO)equiped with an Ethernet shield and RF transceivers
   It acts as a gateway between RF equipments and UDP server
   Can deal with up to 12 RF equipments
   The gateway is identified with 2 bytes address: 0xab-0x00 and the stations addresses are 0xab-0x01, 0xab-0x02 and so on
    for instance: 0xef00 for the gateway and 0xefe01 for the first station

*** UDP/IP ***
    The gateway listens to 2 x (port, IP): the gateway one to receive messages directed to the gateway as itself and the station to receive messagee to forward to the stations thru RF
    Services messsages received on the gateway port are used to locate the server(s) ready to interact with the gateway

*** Radio Frequence ***
    The software is permanently listening to the RF433 network using the VirtualWire.h library
    RF message format: one byte addresses (0xef for instance) for the gateway and one for the station (0x01 for instance)
    message format is: sender address, receiver address, 0x00, frameCount, data
    for instance 0x01 0xef 0x00 0x09 0x010203: frame number 9, from station 1 to the gateway 254 with 0x010203 as data
    From RF to UDP the addresses are translated: gateway 0xef:254 becomes 254+256=65024 and the station 0x01:1 becomes 254*256+1=65025)
    RF frames are sent 3 time to improve reliability - duplicated frames are detected with frameCount and ignored

*** Data ***
    The software regurarly send to the server
      status information
      active stations list
      active stations RF statistics

*** Eeprom ***
    MAC address manualy defined is stored in eeprom
    IP configuration (address, mask, router, dns server) manualy defined is stored in eeprom
    remove ethernet shield and set MOSI PIN high to initialize eeprom
*/

/*
   depending on wheter the gateway is on the same local network the server DNS has to be activated or not using the define flag: #define useDns
   if defined dns address must have been stored in eeprom
    if use set the server name inside RequestDns()
*/
//#define debugOn // uncomment for debuging
//#define printStat // uncomment for debuging
//#define initEeprom
//#define useDns  // uncomment for using DNS resolution to find the server by name over internet - otherwise use stored server IP - automatic for TLS18
//#define useDHCP

//#include "LH107.h" // config GW le havre
#include "TLS13.h" // config GW Toulouse

#include <HomeAutomationBytesCommands.h> // commands specifications
#include <SPI.h>
#include <VirtualWire.h>
#include <Ethernet.h>
EthernetClient client;
#ifdef useDns
#include <Dns.h>
DNSClient dnsClient;

char hubServer[] = "cuillerj.chickenkiller.com";
#endif
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <EEPROM.h>

byte valueEeprom;
int addrEeprom = 0;
#define addrid 0
#define addrmac 2
#define addrip 8
#define addrmask 12
#define addrgateway 16
#define addrdynDns 20

#define maxStationNumber 12
#define buzzPin 9
#define greenLEDPin 7
#define redLEDPin 8
#define sendPin 3
#define receivePin 5
uint8_t ListStations[10] = {  // up to 12 stations
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a
};
uint8_t ActiveStations[sizeof(ListStations)]; //
unsigned int TrafficStationsIn[sizeof(ListStations)];  // index idem ActiveStations - a partir activation
unsigned int TrafficStationsOut[sizeof(ListStations)]; // Index idem ListStations
uint8_t lastSentNumber[sizeof(ListStations)]; // last sent number
uint8_t frameCountStation[sizeof(ListStations)];
unsigned  int IgnoredTrame; // index trames ignorees
unsigned int RFTraffic;
unsigned int RFinputFrame;
unsigned int RFstationError;
unsigned int RFframeMissed[sizeof(ListStations)];

uint8_t DataIn[25];
uint8_t addrS = 0x01; // station address
uint8_t addrE = 0x01; // hex 01 master
//uint8_t cmde = 0x00;
//uint8_t byt3 = 0x00;
uint8_t dlen = 0x00;

//uint8_t mailBootSent = 0x00; // flag mail reboot envoye
//uint8_t ss;
String data = "";
String Err01 = "??ID:";

// ethernet et SD card
// On the Ethernet Shield, CS is pin 4. Note that even if it's not
// used as the CS pin, the hardware CS pin (10 on most Arduino boards,
// 53 on the Mega) must be left as an output or the SD library
// functions will not work.

//define PROGMEM __ATTR_PROGMEM__
//PROGMEM const int chipSelect = 4;
PROGMEM const int chipSelectEth = 10; // Pin 10,11,12,13 reserve pour ethernet
byte mac[6];
uint8_t dynDns = 0xff;
#ifdef useDns
IPAddress RemoteIPForGateway = {0, 0, 0, 0};
IPAddress RemoteIPForStation = {0, 0, 0, 0};
#endif
#ifndef useDns
IPAddress RemoteIPForGateway = {192, 168, 1, 197};
IPAddress RemoteIPForStation = {192, 168, 1, 197};
#endif
unsigned int portAsGateway = 8889;      // local port to listen on
unsigned int portAsStation = 8888;      // local port to listen on
unsigned int RemotePortForStation = 49164;      // remote port en ecoute
unsigned int RemotePortForGateway = 49166;      // remote port modifie dynamiquement
#define minUdpSize 28

unsigned long timeStatusSent;
#define statusCycleDuration 300000
//unsigned long timeStatusListSent = 5000;
//#define statusListCycleDuration 180000
#define readyDurationAfterBoot 60000
unsigned long timeStatisticsSent;
#define statisticsCycleDuration 60*60000
unsigned long lastUDPreceivedTime = 0;
unsigned long lastRFreceivedTime[sizeof(ListStations)];


uint8_t statisticsFlag = 0xff;
#ifdef useDns
unsigned long timeRequested;
unsigned long lastDNSupdateTime = 0;
#endif
// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,
uint8_t *PpacketBuffer = &packetBuffer[0]; // pointer to the input UDP data zone
#define rfStationPosition 8
#define rfStartPosition 12
char *PRfpacketBuffer = &packetBuffer[rfStartPosition]; // pointer to the input UDP data zone
char  ReplyBuffer[] = "ack";       // a string to send back
byte dataBin[50];  // min data en out UDP pour un max trame a 30 + 6
uint8_t ShiftUdp = 0;

uint8_t _stationAddress;
uint8_t _gatewayAddress;
//uint8_t _senderPin;
//uint8_t _reveiverPin;
int _speedLink;
uint8_t _outMsg[25];
uint8_t _outFrameNumber = 0x00;
uint8_t _nbRetry = 4;
uint8_t _retry;
uint8_t _outLen;
unsigned long _lastSentTime;
#define _maxDataLen 20
#define _headerLen 5

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP UdpAsGateway;
EthernetUDP UdpAsStation;
// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
// EthernetServer server(80);
boolean justReboot = true;
#define diagReboot 0
#define diagServerConnexion 1
#define diagGatewayReady 2
#define serverTimeout 3
#define stationTimeout 4
#define diagDNS 5

#ifdef useDns
byte diagByte = 0b00100101;
#else
byte diagByte = 0b00000111;
#endif

byte prevStationsAlert[2] = {0x00, 0x00};
byte stationsAlert[2] = {0x00, 0x00};
byte stationsTimeout[2] = {0x00, 0x00};

/*
   to init eeprom remove the ethernet shied and set to high the MOSI PIN with a switch on the ICSP connector
   the first time configuration must be adjusted in the eeprom folder
   station ID and IP configuration are stored once in eeprom in order - later the software can be download keeping the station configuration

*/
//#define configPIN MOSI

void setup() {
  pinMode(greenLEDPin, OUTPUT);
  digitalWrite(greenLEDPin, HIGH);
  delay(1000);
  digitalWrite(greenLEDPin, LOW);
  pinMode(redLEDPin, OUTPUT);
  digitalWrite(redLEDPin, HIGH);
  pinMode(buzzPin, OUTPUT);
  digitalWrite(buzzPin, 1);
  delay(100);
  digitalWrite(buzzPin, 0);
  //digitalWrite(buzzPin, LOW);
  Serial.begin(38400);
  delay(500);
  Serial.print("RF433 Gateway V");
  Serial.println(Version);
  Serial.print("station Id:");
  //  pinMode(configPIN, INPUT_PULLUP);
#if defined initEeprom
  InitEeprom();
#endif
  PrintEeprom();
#define addrid 0
  valueEeprom = EEPROM.read(addrid);
  if (valueEeprom == 0xff) {
    Serial.println("eeprom to be initialized");
    InitEeprom();
    delay(1000);
  }
  else {
    addrS = valueEeprom;
#ifndef useDHCP
    IPAddress IP(EEPROM.read(addrip), EEPROM.read(addrip + 1), EEPROM.read(addrip + 2), EEPROM.read(addrip + 3));
    IPAddress gateway(EEPROM.read(addrgateway), EEPROM.read(addrgateway + 1), EEPROM.read(addrgateway + 2), EEPROM.read(addrgateway + 3));
    IPAddress subnet(EEPROM.read(addrmask), EEPROM.read(addrmask + 1), EEPROM.read(addrmask + 2), EEPROM.read(addrmask + 3));
    //#ifdef useDns
    IPAddress dnsAddr(EEPROM.read(addrdynDns), EEPROM.read(addrdynDns + 1), EEPROM.read(addrdynDns + 2), EEPROM.read(addrdynDns + 3));
#endif
    //#endif
    for (int i = 0; i < 6; i++)
    {
      mac[i] = EEPROM.read(addrmac + i);
      Serial.print(mac[i], HEX);
      Serial.print("-");
    }
    Serial.print(addrS);
    Serial.print(" RF:");
    Serial.println(SpeedNetw);
#ifndef useDHCP
    Serial.print(IP);
    Serial.print("-");
    Serial.print(gateway);
    Serial.print("-");
    Serial.print(subnet);
    Serial.print("-");
    Serial.print(dnsAddr);
    Serial.println("-");
#endif
    pinMode(chipSelectEth, OUTPUT);
    digitalWrite(chipSelectEth, HIGH);

    // start the Ethernet connection and the server:
    Serial.println("start eth");

    //  Ethernet.begin(mac, IP, dnsAddr, gateway, subnet);
    Ethernet.begin(mac);
#ifndef useDHCP
    Ethernet.setGatewayIP(gateway);
    Ethernet.setLocalIP(IP);
    Ethernet.setSubnetMask(subnet);
#endif
    delay(1000);
    Serial.println("connecting...");
    Serial.println(Ethernet.localIP());
    Serial.println(Ethernet.gatewayIP());
    Serial.println(Ethernet.subnetMask());
    UdpAsGateway.begin(portAsGateway);

    Serial.print("IP:");
    Serial.print(Ethernet.localIP());
    Serial.print(": ");
    Serial.print(portAsGateway);

    UdpAsStation.begin(portAsStation);
#ifdef useDns
    dnsClient.begin(Ethernet.dnsServerIP());
#endif
    Serial.print(": ");
    Serial.println(portAsStation);
    Serial.print("rf nb:");
    Serial.println(sizeof(ActiveStations));

    vw_set_tx_pin(sendPin);   // choisir la broche 3 pour emettre


    vw_set_rx_pin(receivePin);     // choisir la broche 5 pour recevoir
    vw_set_ptt_pin(6);     // choisir la broche 6 pour ptt - ajoute suite bug ecriture carte SD
    vw_setup(SpeedNetw);       // vitesse de reception
    vw_rx_start();        // demarrer la reception
    Serial.print("rf at speed:");
    Serial.println(SpeedNetw);
    for (int i = 0; i < sizeof(ActiveStations); i++) {
      ActiveStations[i] = 0x00;
    }
#ifdef useDns
    RequestDns();
#endif

  }
}

void loop() {
  delay(1);

  if (vw_have_message())        // message from RF434
  {
    TrameAnalyzeRFInput();
  }
  RetrySend();
  RefreshLED();
  RefreshBuzzer();
  if (justReboot && millis() > readyDurationAfterBoot) {
    SendStatus() ;
    justReboot = false;
  }
  if (bitRead(diagByte, diagReboot) && (millis() > 4 * readyDurationAfterBoot) ) {  // let enough time for the server to discover the reboot
    bitWrite(diagByte, diagReboot, 0);
  }

  if ( !bitRead(diagByte, diagServerConnexion)) {
    bitWrite(diagByte, diagGatewayReady, 0);
  }

  if (millis() > timeStatusSent + statusCycleDuration) {
    SendStatus() ;
  }
  /*
    if (millis() > timeStatusListSent + statusListCycleDuration) {
    SendStationsList();
    }
  */
  if (millis() > timeStatisticsSent + statisticsCycleDuration) {
#if defined(printStat)
    PrintStatistics();
#endif
    SendRFGatewayStatistics();
  }
  if (statisticsFlag != 0xff && millis() > timeStatisticsSent + 2000) {
    SendRFStationStatistics();
  }
  InputUdpAsGateway();        // test data en input UDP
  InputUdpAsStation();
  if ((millis() > readyDurationAfterBoot + 10000) && (stationsAlert[0] != prevStationsAlert[0]) || (stationsAlert[1] != prevStationsAlert[1])) {
    timeStatisticsSent = millis() + statusCycleDuration -  30000;
    prevStationsAlert[0] = stationsAlert[0];
    prevStationsAlert[1] = stationsAlert[1];
  }
  if (millis() - lastUDPreceivedTime > 30 * 60000) {
    bitWrite(diagByte, serverTimeout, 1);
  }
  for (int i = 0; i < sizeof(ListStations); i++) {
    if (ActiveStations[i] != 0x00) {
      //    int station = ActiveStations[i];
      if (millis() - lastRFreceivedTime[i] > 30 * 60000) {
        if (i <= 7) {
          bitWrite(stationsTimeout[0], i, 1);
        }
        else {
          bitWrite(stationsTimeout[1], i, 1);
        }
      }
      else {
        if (i <= 7) {
          bitWrite(stationsTimeout[0], i, 0);
        }
        else {
          bitWrite(stationsTimeout[1], i, 0);
        }
      }
    }

  }
#ifdef useDns
  if (!bitRead(diagByte, diagDNS) && (millis() - lastDNSupdateTime) > 15 * 60000) {
    RequestDns();
  }
  if (bitRead(diagByte, diagDNS) && (millis() - lastDNSupdateTime) >  60000) {
    RequestDns();
  }
#endif
}

#ifdef useDns
boolean RequestDns() {
  IPAddress newIP;
  boolean gotDns = (dnsClient.getHostByName(hubServer, newIP) == 1);
  lastDNSupdateTime = millis();
  bitWrite(diagByte, diagDNS, !gotDns);
  if (gotDns) {
    RemoteIPForGateway = newIP;
    Serial.print("DNS ok ip:");
    Serial.println( RemoteIPForGateway);
    RemoteIPForStation = RemoteIPForGateway;
  }
  else {
    Serial.println("DNS failed");
  }
  timeRequested = millis();
  return gotDns;
}
#endif
