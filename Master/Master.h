/*
 * Header file for Master.ino
 */

#ifndef MASTER
#define MASTER

#include <WirelessSmartSensor.h>
#include <ADXL355.h>

// To connect via Telnet
WiFiServer server(23);
WiFiClient client;

// File to save data from adxl355
File outputFile;
String outputFileName = "record1";

// Record number
int recordIndex = 1;

// Define NTP Client
WiFiUDP ntpUDP, udp;
NTPClient timeClient(ntpUDP, "192.168.1.212", -14400, 60000);

// Define broadcast address
IPAddress broadcast(255, 255, 255, 255);
int port = 4210;

ADXL355 adxl;


class Master: public WirelessSmartSensor {
  public:
     void record();
     void wait();
     void sync();
     void temperature();
     void diagnosis();
     void sendDataBackHome();

  private:
     String getTime();
     String getDate();

};


#endif
