/*
 * Header file for master.ino
 */

#ifndef MASTER
#define MASTER

#include <Functions.h>

// To connect via Telnet
WiFiServer server(23);

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
