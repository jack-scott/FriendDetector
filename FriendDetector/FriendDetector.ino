#include "./esppl_functions.h"
#include <Adafruit_NeoPixel.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

/*
 * Define you friend's list size here
 */
#define NUMPIXELS 5
#define PIXEL_PIN 4

#define LIST_SIZE 5

// NTP setup
const char *ssid     = "WiFi-173E";
const char *password = "93522199";
const long utcOffsetInSeconds = (9*60*60) + (30*60);  //NSW is +09:30

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

//Neopixel setup 
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ400);

/*
 * This is your friend's MAC address list
 */
uint8_t friendmac[LIST_SIZE][ESPPL_MAC_LEN] = {
  {0xAC, 0x5F, 0x3E, 0x80, 0xC5, 0xAF}
  ,{0x60, 0x30, 0xD4, 0x02, 0x6E, 0xD9}
  ,{0x7C, 0x9A, 0x1D, 0x76, 0x80, 0x4C}	
  ,{0x94, 0xBF, 0x2D, 0x43, 0x30, 0x6A}
  ,{0xC0, 0xE8, 0x62, 0x25, 0x4E, 0x7F}
  };
/*
 * This is your friend's name list
 * put them in the same order as the MAC addresses
 */
String friendname[LIST_SIZE] = {
  "Jack"
  ,"Ollie"
  ,"Tim"
  ,"Sam"
  ,"Ed"
  };

int lastseen[LIST_SIZE];

bool maccmp(uint8_t *mac1, uint8_t *mac2) {
  for (int i=0; i < ESPPL_MAC_LEN; i++) {
    if (mac1[i] != mac2[i]) {
      return false;
    }
  }
  return true;
}

void cb(esppl_frame_info *info) {
  for (int i=0; i<LIST_SIZE; i++) {
    if (maccmp(info->sourceaddr, friendmac[i]) || maccmp(info->receiveraddr, friendmac[i])) {
        if(lastseen[i] < 255){
          lastseen[i] = lastseen[i] + 32;
          if(lastseen[i] > 255){
            lastseen[i] = 255;
          }
        }
        Serial.printf("\n%s is home : %d", friendname[i].c_str(), lastseen[i]);
        Serial.println(timeClient.getEpochTime());

    }else if(lastseen[i] >= 0){
      if(lastseen[i] > 0){
        lastseen[i] = lastseen[i] - 1;
      }
      Serial.printf("\n%s is NOT home : %d", friendname[i].c_str(), lastseen[i]);
    }

    pixels.setPixelColor(i, pixels.Color(0,lastseen[i],0));
    pixels.show();

  }
}

void setup() {
  delay(500);
  Serial.begin(115200);
  pixels.begin();
  esppl_init(cb);
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  timeClient.begin();
}

void loop() {
  esppl_sniffing_start();
  while (true) {
    for (int i = ESPPL_CHANNEL_MIN; i <= ESPPL_CHANNEL_MAX; i++ ) {
      esppl_set_channel(i);
      while (esppl_process_frames()) {
        //
      }
    }
  }  
}
