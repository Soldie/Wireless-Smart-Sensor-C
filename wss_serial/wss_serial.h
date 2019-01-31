/*
 * Header file for wss.ino
 */

#ifndef WSS
#define WSS

#include <WirelessSmartSensor.h>
#include <ADXL355.h>

ADXL355 adxl;
WirelessSmartSensor wss;

/*
 * Standard setup function
 */
void setup();

/*
 * Perform waiting operation, listening 
 */
void wait();

/*
 * Standard loop function
 */
void loop();

#endif
