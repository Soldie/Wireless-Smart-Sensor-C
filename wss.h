/*
 * Header file for wss.ino
 */

#ifndef WSS
#define WSS

#include <SPI.h>
#include <WiFiNINA.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "config.h"

// Defining all possible states for the sensor
enum State { WAIT, SYNC, RECORD };

// Initial state of the sensor
State sensor_state = WAIT;

// Memory register addresses:
const int XDATA3 = 0x08;
const int XDATA2 = 0x09;
const int XDATA1 = 0x0A;
const int YDATA3 = 0x0B;
const int YDATA2 = 0x0C;
const int YDATA1 = 0x0D;
const int ZDATA3 = 0x0E;
const int ZDATA2 = 0x0F;
const int ZDATA1 = 0x10;
const int RANGE = 0x2C;
const int POWER_CTL = 0x2D;

// Device values
const int RANGE_2G = 0x01;
const int RANGE_4G = 0x02;
const int RANGE_8G = 0x03;
const int MEASURE_MODE = 0x06; // Only accelerometer

// Operations
const int READ_BYTE = 0x01;
const int WRITE_BYTE = 0x00;

// Pins used for the connection with the sensor
const int CHIP_SELECT_PIN = 7;

// Replace with your network credentials
char ssid[] = "VTR-2582559";
char pass[] = "4wptdnBHjfxz";

// the WiFi radio's status
int status = WL_IDLE_STATUS; 

// Define NTP Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", -14400, 60000);

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;

// Interval of recording, default is 10 seconds
long interval = 10000; 

/*
 * Standard setup function
 */
void setup();

/*
 * Perform waiting operation, listening 
 */
void wait();

/*
 * Perform synchronazing operation
 */
void sync();

/* 
 * Write registry in specific device address
 */
void writeRegister(byte thisRegister, byte thisValue);

/* 
 * Read registry in specific device address
 */
unsigned int readRegistry(byte thisRegister);

/* 
 * Read multiple registries
 */
void readMultipleData(int *addresses, int dataSize, int *readedData);

/*
 * Perform recording operation
 */
void record();

/*
 * Standard loop function
 */
void loop();

#endif