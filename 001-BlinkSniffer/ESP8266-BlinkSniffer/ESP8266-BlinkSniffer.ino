/******************************************************************************* 
 *  Simple ESP8266 Sketch to demonstrate promiscuous mode
 *  
 *  This Sketch is designed for use with the ESP8266 microcontroller.  It sets
 *  the WiFi to promiscuous mode (meaning, it will simply "listen" to all WiFi
 *  traffic, including datagrams intended for other devices).
 *  
 *  This uses the wifi_set_promiscuous_rx_cb function to set a callback; this
 *  callback will be called whenever a WiFi datagram is detected.  In this
 *  Sketch, we simply count the number of times the callback is called (which
 *  is essentially counting the number of datagrams detected), and for every 5
 *  datagrams we detect, the built in LED will blink.  Pretty simple!
 *  
 *  Speeial thanks to Ray Burnette for his post on using the ESP8266 in 
 *  promiscuous mode:
 *  
 *  https://www.hackster.io/rayburne/esp8266-mini-sniff-f6b93a
 *  
 ******************************************************************************/


#include <ESP8266WiFi.h>

// For some reason on my ESP8266 board, the builtin LED is off when the pin
// is set to HIGH, and on when the pin is set to LOW.  So...

#define LED_ON       LOW
#define LED_OFF      HIGH

#define MODE_DISABLE 0
#define MODE_ENABLE  1

#define CHANNEL	     11

// Global Variables
long gCallbackCount = 0;

// Forward declarations
void promiscuous_callback(uint8_t *buf, uint16_t len);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LED_OFF);
  
  Serial.begin(115200);
  Serial.printf("\n\nSDK version:%s\n\r", system_get_sdk_version());
  Serial.println(F("ESP8266- BlinkSniffer Sketch"));
  Serial.end();

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
  static long prevCallbackCount = 0;

  long packetsDetected = gCallbackCount - prevCallbackCount;
  
  if (packetsDetected >= 5) {
    digitalWrite(LED_BUILTIN, LED_ON);
    delay(10);  // delay in microseconds
    digitalWrite(LED_BUILTIN, LED_OFF);

    prevCallbackCount = gCallbackCount;
  }
}

void promiscuous_callback(uint8_t *buf, uint16_t len) {
  gCallbackCount++;
}
