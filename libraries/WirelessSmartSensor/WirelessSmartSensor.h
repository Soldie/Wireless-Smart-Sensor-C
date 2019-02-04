/*
 * Header file for WirlessSmartSensor.cpp
 */

#ifndef WirelessSmartSensor_h
#define WirelessSmartSensor_h

#include <Arduino.h>
#include <WiFiNINA.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Time.h>
#include <SD.h>
#include <SDConfigFile.h>

class WirelessSmartSensor{
  public:
  	// Defining all possible states for the sensor
	enum State { WAIT, SYNC, RECORD };

	// Initial state of the sensor
	State sensor_state = WAIT;

	// The filename of the configuration file on the SD card
	char *CONFIG_FILE;
	
	// Interval to keep recording 
	int INTERVAL;

	/*
	 * Perform synchronazing operation
	 */
	void sync();

	/*
	 * Perform recording operation
	 */
	void record();

	/*
	 * Perform the wating operation, listening 
	 */
	void wait();

	/*
	 * Setup Wifi, SD and serial
	 */
	void setupAll();

	/*
	 * Protocol to send data back home
	 */
	void sendDataBackHome();

  private:
	// Pins used for the connection with the sensor
	const int CHIP_SELECT_PIN = 7;

	// the WiFi radio's status
	int status = WL_IDLE_STATUS; 

	// SD shield Chip Select pin
	const int pinSelectSD = 4; 

	//Variables that will be read
	char *SSID;
	char *PASS;
	int FS;

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

};

#endif