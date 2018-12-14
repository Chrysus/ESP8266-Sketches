/******************************************************************************* 
 *  Simple ESP8266 Sketch to demonstrate promiscuous mode and serial output
 *  
 *  This Sketch is designed for use with the ESP8266 microcontroller.  It sets
 *  the WiFi to promiscuous mode (meaning, it will simply "listen" to all WiFi
 *  traffic, including datagrams intended for other devices).
 *  
 *  In this version of the Sniffer, we will scan through the available WiFi
 *  channels, "listening" for 1 minute on each channel.
 *  
 *  This uses the wifi_set_promiscuous_rx_cb function to set a callback; this
 *  callback will be called whenever a WiFi datagram is detected.  In this
 *  Sketch, we will keep track of the number of datagrams detected, as well
 *  as the length of each datagram.  This tracking data will be periodically
 *  output over the Serial connection.
 *  
 *  As a bonus, the user can also request the data output by sending a
 *  newline (\n) to the ESP8266 via the Serial connection.
 *  
 *  Speeial thanks to Ray Burnette for his post on using the ESP8266 in 
 *  promiscuous mode:
 *  
 *  https://www.hackster.io/rayburne/esp8266-mini-sniff-f6b93a
 *  
 ******************************************************************************/


#include <ESP8266WiFi.h>
#include <stdio.h>
#include "ESPDatagram.h"

// For some reason on my ESP8266 board, the builtin LED is off when the pin
// is set to HIGH, and on when the pin is set to LOW.  So...

#define LED_ON       LOW
#define LED_OFF      HIGH

#define MODE_DISABLE 0
#define MODE_ENABLE  1

/*
 *  Global Variables
 */
byte gMyMAC[6];
 
unsigned int gCurrentChannel = 1;  //<-- Channels range from 1 to 14

// The following arrays will store the datagram counts for each channel; to 
// keep things simple, the index into the array for the channel will be the
// channel number.  However, since arrays are zero based, but the range of 
// channels start on 1, we will use index 0 to keep track of the total counts
// regardless of the channel.
unsigned long gCallbackCount[15] = {0};

// I happen to know that most datagrams will be 12, 60, or 128 bytes
unsigned long gDatagramCount_012[15] = {0};
unsigned long gDatagramCount_060[15] = {0};
unsigned long gDatagramCount_128[15] = {0};
unsigned long gDatagramCount_Other[15] = {0};

unsigned long gDatagramTypeMgmtCount[15] = {0};
unsigned long gDatagramTypeCntlCount[15] = {0};
unsigned long gDatagramTypeDataCount[15] = {0};
unsigned long gDatagramTypeRsrvCount[15] = {0};

unsigned long gDatagramAssocCount[15] = {0};
unsigned long gDatagramProbeCount[15] = {0};
unsigned long gDatagramBeaconCount[15] = {0};
unsigned long gDatagramDisassocCount[15] = {0};



/*
 *  Forward declarations
 */
void promiscuous_callback(uint8_t *buf, uint16_t len);
void print_report();

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LED_OFF);
  
  Serial.begin(115200);
  Serial.printf("\n\nSDK version:%s\n\r", system_get_sdk_version());
  Serial.println(F("ESP8266- ScanSniffer Sketch"));

  WiFi.macAddress(gMyMAC);
  Serial.print("\n -- MAC ADDRESS: ");
  print_mac_address(gMyMAC);
  Serial.print("\n");

  // Promiscuous only works with STATION_MODE
  wifi_set_opmode(STATION_MODE);
  wifi_set_channel(gCurrentChannel);

  print_channel_switch(gCurrentChannel);
  
  wifi_promiscuous_enable(MODE_DISABLE);
  wifi_set_promiscuous_rx_cb(promiscuous_callback);
  wifi_promiscuous_enable(MODE_ENABLE);
}

/* 
 *  In the loop(), we blink the LED (on for 10 milliseconds) each time we
 *  detect (at least) 5 more datagrams.
 *  
 *  Note - a static variable within a function will retain its value between
 *  function calls.  Here we use prevCallbackCount to "remember" what the 
 *  callback count was the last time the LED was blinked.
 */
 
void loop() {
  static unsigned long prevBlinkCallbackCount = 0;
  static unsigned long prevPrintCallbackCount = 0;

  static unsigned long prevSwitchTime = millis();
  const unsigned long kScanTimeMillis = 1000 * 60;  //<-- 1000(ms) * 60(s) = 1(m)

  /*
   * Channel Scanning Logic
   */
   unsigned long currentTime = millis();
   unsigned long timeDelta = currentTime - prevSwitchTime;

   if (timeDelta >= kScanTimeMillis) {
     gCurrentChannel++;
     if (gCurrentChannel > 14) {
       gCurrentChannel = 1;
     }

     wifi_set_channel(gCurrentChannel);
     prevSwitchTime = currentTime;

     print_channel_switch(gCurrentChannel);
   }
   
  /*
   * LED Blink Logic
   */
  long packetsDetectedSinceLastBlink = gCallbackCount[0] - prevBlinkCallbackCount;
  
  if (packetsDetectedSinceLastBlink >= 5) {
    digitalWrite(LED_BUILTIN, LED_ON);
    delay(10);  // delay in microseconds
    digitalWrite(LED_BUILTIN, LED_OFF);

    prevBlinkCallbackCount = gCallbackCount[0];
  }

  /*
   * Report Print Logic
   */
  bool bReportRequestd = false;
  bool bFullReportRequestd = false;

  // Did the user request a report?
  if ((Serial.available() > 0)) {
    char serial_input = Serial.read();
    if (serial_input == '\n') {
      bReportRequestd = true;
    } else if (serial_input == 'F' || serial_input == 'f') {
      bFullReportRequestd = true;
    }
  }
  
  long packetsDetectedSinceLastPrint = gCallbackCount[0] - prevPrintCallbackCount;
  
  if ((packetsDetectedSinceLastPrint >= 100) || bReportRequestd) {
    print_report();
    prevPrintCallbackCount = gCallbackCount[0];
  }

  // Print Full Report
  if (bFullReportRequestd) {
    for (unsigned int i = 0; i < 15; i++) {
      print_channel_report(i);
    }
  }
}

/*
 * Promiscuous Mode Callback
 */
 
void promiscuous_callback(uint8_t *buf, uint16_t len) {
  gCallbackCount[0]++;
  gCallbackCount[gCurrentChannel]++;

  switch(len) {
    case 12:
    {
      gDatagramCount_012[0]++;
      gDatagramCount_012[gCurrentChannel]++;
    }
    break;

    case 60:
    {
      gDatagramCount_060[0]++;
      gDatagramCount_060[gCurrentChannel]++;
    }
    break;

    case 128:
    {
      gDatagramCount_128[0]++;
      gDatagramCount_128[gCurrentChannel]++;
    }
    break;

    default:
    {
      gDatagramCount_Other[0]++;
      gDatagramCount_Other[gCurrentChannel]++;
    }
    break;
  }

  //struct ESPDatagramMACHeader *frame_header = (struct ESPDatagramMACHeader *)buf;
  //struct ESPDatagramFrameControl *frame_control = (struct ESPDatagramFrameControl *)frame_header->frame_control;

  if (len < sizeof(struct ESPDatagramMACHeader)) {
    return;
  }

  struct ESPDatagramMACHeader *frame_header = new ESPDatagramMACHeader();
  struct ESPDatagramFrameControl *frame_control = new ESPDatagramFrameControl();

  memcpy(frame_header, buf, sizeof(struct ESPDatagramMACHeader));
  memcpy(frame_control, frame_header->frame_control, sizeof(struct ESPDatagramFrameControl));

  if (frame_control->protocol != 0b00) {
    Serial.printf("ERROR - BAD PROTOCOL VALUE: %d", frame_control->protocol);
  }

  if (frame_control->type == kType_Management) {
    Serial.print(frame_control->subtype, BIN);
    Serial.printf("\tMGMT");
    gDatagramTypeMgmtCount[0]++;
    gDatagramTypeMgmtCount[gCurrentChannel]++;
    
    if (frame_control->subtype == kSubtype_Mgmt_AssocReq) {
      Serial.printf("\tAssoc\n");
      gDatagramAssocCount[0]++;
      gDatagramAssocCount[gCurrentChannel]++;
    } else if (frame_control->subtype == kSubtype_Mgmt_ProbeReq) {
      Serial.printf("\tProbe\n");
      gDatagramProbeCount[0]++;
      gDatagramProbeCount[gCurrentChannel]++;
    } else if (frame_control->subtype == kSubtype_Mgmt_Beacon) {
      Serial.printf("\tBeacon <-----------------------------\n");
      gDatagramBeaconCount[0]++;
      gDatagramBeaconCount[gCurrentChannel]++;
    } else if (frame_control->subtype == kSubtype_Mgmt_Disassoc) {
      Serial.printf("\tDisassoc");
      Serial.printf("\t");
      print_mac_address(frame_header->mac_address_1);
      Serial.printf("\t");
      print_mac_address(frame_header->mac_address_2);
      Serial.printf("\tMy MAC: ");
      print_mac_address(gMyMAC);
      Serial.printf("\n");
      gDatagramDisassocCount[0]++;
      gDatagramDisassocCount[gCurrentChannel]++;
    } else {
      Serial.printf("\tOther\n");
    }
  } else if (frame_control->type == kType_Control) {
    Serial.printf("\tCNTL\n");
      
    Serial.printf("\t");
    print_mac_address(frame_header->mac_address_1);
    Serial.printf("\t");
    print_mac_address(frame_header->mac_address_2);
    Serial.printf("\tMy MAC: ");
    print_mac_address(gMyMAC);
    Serial.printf("\n");
    
    gDatagramTypeCntlCount[0]++;
    gDatagramTypeCntlCount[gCurrentChannel]++;
  } else if (frame_control->type == kType_Data) {
    Serial.printf("\tDATA\n");

    Serial.printf("\t");
    print_mac_address(frame_header->mac_address_1);
    Serial.printf("\t");
    print_mac_address(frame_header->mac_address_2);
    Serial.printf("\tMy MAC: ");
    print_mac_address(gMyMAC);
    Serial.printf("\n");
    
    gDatagramTypeDataCount[0]++;
    gDatagramTypeDataCount[gCurrentChannel]++;
  } else if (frame_control->type == kType_Reserved) {
    Serial.printf("\tRSRV\n");

    Serial.printf("\t");
    print_mac_address(frame_header->mac_address_1);
    Serial.printf("\t");
    print_mac_address(frame_header->mac_address_2);
    Serial.printf("\tMy MAC: ");
    print_mac_address(gMyMAC);
    Serial.printf("\n");
    
    gDatagramTypeRsrvCount[0]++;
    gDatagramTypeRsrvCount[gCurrentChannel]++;
  }
}


/*
 * Print Functions
 */

 void print_channel_switch(unsigned int channel_index) {
   Serial.println("\n-------------------------------------------------------------------------------------");
   Serial.printf("SCANNING CHANNEL: %d\n", channel_index);
   Serial.println("-------------------------------------------------------------------------------------\n\n");
 }

 void print_report() {
   Serial.println("\n-------------------------------------------------------------------------------------");

   for (unsigned int channel_index = 1; channel_index < 15; channel_index++) {
     Serial.printf("CHANNEL %2d:\t%7lu\n", channel_index, gCallbackCount[channel_index]);
   }

   Serial.println("---------------------------");
   Serial.printf("TOTAL     :\t%7lu\n", gCallbackCount[0]);
   Serial.println("-------------------------------------------------------------------------------------");
 }

 void print_channel_report(unsigned int channel_index) {
   Serial.println("\n-------------------------------------------------------------------------------------");
   Serial.printf("CHANNEL: %d\n", channel_index);
   Serial.println("-------------------------------------------------------------------------------------");
   
   Serial.println("\n-------------------------------------------------------------------------------------\n");
   Serial.printf("Datagram    12:    %7lu\n", gDatagramCount_012[channel_index]);
   Serial.printf("Datagram    62:    %7lu\n", gDatagramCount_060[channel_index]);
   Serial.printf("Datagram   128:    %7lu\n", gDatagramCount_128[channel_index]);
   Serial.printf("Datagram Other:    %7lu\n", gDatagramCount_Other[channel_index]);
   Serial.println("---------------------------");
   Serial.printf("Type      Mgmt:    %7lu\n", gDatagramTypeMgmtCount[channel_index]);
   Serial.printf("Type      Cntl:    %7lu\n", gDatagramTypeCntlCount[channel_index]);
   Serial.printf("Type      Data:    %7lu\n", gDatagramTypeDataCount[channel_index]);
   Serial.printf("Type      Rsrv:    %7lu\n", gDatagramTypeRsrvCount[channel_index]);
   Serial.println("---------------------------");
   Serial.printf("Type     Assoc:    %7lu\n", gDatagramAssocCount[channel_index]);
   Serial.printf("Type    Beacon:    %7lu\n", gDatagramBeaconCount[channel_index]);
   Serial.printf("Type     Probe:    %7lu\n", gDatagramProbeCount[channel_index]);
   Serial.printf("Type  DisAssoc:    %7lu\n", gDatagramDisassocCount[channel_index]);
   Serial.println("---------------------------");
   Serial.printf("Datagram Total:    %7lu\n", gCallbackCount[channel_index]);
   Serial.println("\n-------------------------------------------------------------------------------------\n\n");
 }
