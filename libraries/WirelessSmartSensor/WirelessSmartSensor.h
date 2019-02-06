/*
 * Header file for WirlessSmartSensor.cpp
 *
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
	enum State { WAIT, SYNC, RECORD, TEMP, DIAG, GATHER };

	/*
	 * INTERVAL Getter 
	 */
	int getInterval();
	
	/*
	 * Sensor's state getter
	 */
	State getState();

	/*
	 * Sensor's state setter
	 */
	void setState(State s);

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
	 * Perform the self test operation 
	 */
	void diagnosis();

	/*
	 * Perform the reading of temperature 
	 */
	void temperature();

	/*
	 * Setup Wifi, SD and serial
	 */
	void setupAll();

	/*
	 * Protocol to send data back home
	 */
	void sendDataBackHome();

	/*
	 * Sampling frequency getter
	 */
	int getFS();

	/*
	 *	Get time from sensor
	 */
	String getTime();

	/*
	 *	Get date from sensor
	 */
	String getDate();

	/*
	 * Gets MAC address of WiFi shield
	 */
	void printMacAddressWifi(File file);

  private:
	// Initial state of the sensor
	State sensor_state = WAIT;

	// Interval to keep recording 
	int INTERVAL;

	// The filename of the configuration file on the SD card
	char *CONFIG_FILE;

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