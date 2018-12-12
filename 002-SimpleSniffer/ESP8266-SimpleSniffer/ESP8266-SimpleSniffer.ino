/******************************************************************************* 
 *  Simple ESP8266 Sketch to demonstrate promiscuous mode and serial output
 *  
 *  This Sketch is designed for use with the ESP8266 microcontroller.  It sets
 *  the WiFi to promiscuous mode (meaning, it will simply "listen" to all WiFi
 *  traffic, including datagrams intended for other devices).
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

// For some reason on my ESP8266 board, the builtin LED is off when the pin
// is set to HIGH, and on when the pin is set to LOW.  So...

#define LED_ON       LOW
#define LED_OFF      HIGH

#define MODE_DISABLE 0
#define MODE_ENABLE  1

#define CHANNEL	     11

/*
 *  Global Variables
 */
unsigned long gCallbackCount = 0;

// I happen to know that most datagrams will be 12, 60, or 128 bytes
unsigned long gDatagramCount_012 = 0;
unsigned long gDatagramCount_060 = 0;
unsigned long gDatagramCount_128 = 0;
unsigned long gDatagramCount_Other = 0;

/*
 *  Forward declarations
 */
void promiscuous_callback(uint8_t *buf, uint16_t len);
void print_report();

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LED_OFF);

  // Sometimes it helps to force a Serial end
  Serial.end();
  
  Serial.begin(115200);
  Serial.printf("\n\nSDK version:%s\n\r", system_get_sdk_version());
  Serial.println(F("ESP8266- BlinkSniffer Sketch"));

  // Promiscuous only works with STATION_MODE
  wifi_set_opmode(STATION_MODE);
  wifi_set_channel(CHANNEL);
  
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
  static long prevBlinkCallbackCount = 0;
  static long prevPrintCallbackCount = 0;

  long packetsDetectedSinceLastBlink = gCallbackCount - prevBlinkCallbackCount;
  
  if (packetsDetectedSinceLastBlink >= 5) {
    digitalWrite(LED_BUILTIN, LED_ON);
    delay(10);  // delay in microseconds
    digitalWrite(LED_BUILTIN, LED_OFF);

    prevBlinkCallbackCount = gCallbackCount;
  }

  // Did the user request a report?
  bool bReportRequestd = false;
  if ((Serial.available() > 0) && (Serial.read() == '\n')) {
    bReportRequestd = true;
  }
  
  long packetsDetectedSinceLastPrint = gCallbackCount - prevPrintCallbackCount;
  
  if ((packetsDetectedSinceLastPrint >= 100) || bReportRequestd) {
    print_report();
    prevPrintCallbackCount = gCallbackCount;
  }
}

/*
 * Promiscuous Mode Callback
 */
 
void promiscuous_callback(uint8_t *buf, uint16_t len) {
  gCallbackCount++;

  switch(len) {
    case 12:
    {
      gDatagramCount_012++;
    }
    break;

    case 60:
    {
      gDatagramCount_060++;
    }
    break;

    case 128:
    {
      gDatagramCount_128++;
    }
    break;

    default:
    {
      gDatagramCount_Other++;
    }
    break;
  }
}


/*
 * Print Report
 */

 void print_report() {
   char datagram_string_012[32];
   char datagram_string_060[32];
   char datagram_string_128[32];
   char datagram_string_other[32];
   char datagram_string_total[32];

   sprintf(datagram_string_012, "Datagram    12:    %7lu\n", gDatagramCount_012);
   sprintf(datagram_string_060, "Datagram    62:    %7lu\n", gDatagramCount_060);
   sprintf(datagram_string_128, "Datagram   128:    %7lu\n", gDatagramCount_128);
   sprintf(datagram_string_other, "Datagram Other:    %7lu\n", gDatagramCount_Other);
   sprintf(datagram_string_total, "Datagram Total:    %7lu\n", gCallbackCount);
   
   Serial.println("\n-------------------------------------------------------------------------------------\n");
   Serial.print(datagram_string_012);
   Serial.print(datagram_string_060);
   Serial.print(datagram_string_128);
   Serial.print(datagram_string_other);
   Serial.println("---------------------------");
   Serial.print(datagram_string_total);
   Serial.println("\n-------------------------------------------------------------------------------------\n\n");
 }
