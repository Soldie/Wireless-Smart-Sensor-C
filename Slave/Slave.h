/*
 * Header file for Slave.ino
 */

#ifndef SLAVE
#define SLAVE

#include <WirelessSmartSensor.h>
#include <ADXL355.h>

// Define NTP Client
WiFiUDP ntpUDP, udp;
NTPClient timeClient(ntpUDP, "192.168.1.212", -14400, 60000);

// Define broadcast address
IPAddress broadcast(255, 255, 255, 255);
int port = 4210;

ADXL355 adxl;


class Slave: public WirelessSmartSensor {
  public:
     void record();
     void wait();
     void sync();

};


#endif