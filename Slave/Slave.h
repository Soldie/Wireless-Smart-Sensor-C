/*
 * Header file for Slave.ino
 */

#pragma once

#include <WirelessSmartSensor.h>

class Slave: public WirelessSmartSensor {
  public:
     void wait();
     
};
