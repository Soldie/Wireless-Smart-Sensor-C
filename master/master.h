/*
 * Header file for master.ino
 */

#ifndef MASTER
#define MASTER

#include <WiFiNINA.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <SD.h>
#include <SDConfigFile.h>

// the WiFi radio's status
int status = WL_IDLE_STATUS; 

// Define NTP Client
WiFiUDP ntpUDP, udp;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", -14400, 60000);

// Define broadcast address
IPAddress broadcast(255, 255, 255, 255);
int port = 4210;

// SD shield Chip Select pin
const int pinSelectSD = 4; 

// The filename of the configuration file on the SD card
const char CONFIG_FILE[] = "file.cfg";

//Variables that will be read
char *SSID;
char *PASS;
int FS;

/*
 * Standard setup function
 */
void setup();

/*
 * Starting serial connection
 */
void setupSerial();

/*
 * Starting SD
 */
void setupSD();

/*
 * Starting Wifi
 */
void setupWifi();

/*
 * Standard loop function
 */
void loop();

/*
 * Read configuration in the file.cfg in SD. Returns true 
 * if successful and false if it failed
 */
bool readConfiguration();

#endif
