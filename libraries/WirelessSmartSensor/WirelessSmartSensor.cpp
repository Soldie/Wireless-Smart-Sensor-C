/*
 * WirelessSmartSensor.cpp represents a sketch for Wireless Smart Sensors
 *
 */

#include <WirelessSmartSensor.h>

void WirelessSmartSensor::setupAll(){

  // Initializing Serial, SD and Wifi
  setupSerial();
  setupSD();
  setupWifi();

}

void WirelessSmartSensor::setupSD(){

  // Initialize pin and variables for SD 
  pinMode(pinSelectSD, OUTPUT);
  FS = 0;
  INTERVAL = 0;
  SSID = "";
  PASS = "";  
  CONFIG_FILE = "file.cfg";

  // Setup the SD card 
  while (!SD.begin(pinSelectSD)) {
    ;
  }
  
  // Read our configuration from the SD card file.
  const uint8_t CONFIG_LINE_LENGTH = 127;
  
  // The open configuration file
  SDConfigFile cfg;
  
  // Open the configuration file
  while (!cfg.begin(CONFIG_FILE, CONFIG_LINE_LENGTH)) {
    ;
  }
  
  // Read each setting from the file
  while (cfg.readNextSetting()) {

    // Sampling Rate
    if (cfg.nameIs("FS")) {
      
      FS = cfg.getIntValue();
    
    // Interval
    } else if (cfg.nameIs("INTERVAL")) {
      
      INTERVAL = cfg.getIntValue();
   
    // Network name
    } else if (cfg.nameIs("SSID")) {
      
      SSID = cfg.copyValue();
   
    // Network password
    } else if (cfg.nameIs("PASS")) {
      
      PASS = cfg.copyValue();
    }
  }
  
  cfg.end();
  
}

void WirelessSmartSensor::setupSerial(){

  // Initialize Serial Monitor
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect
  }

}

void WirelessSmartSensor::setupWifi(){

  status = WiFi.begin(SSID, PASS);
  
  while (status != WL_CONNECTED) {
    delay(500);
    status = WiFi.begin(SSID, PASS);
  }

}

void WirelessSmartSensor::sync(){

  /* Define your own way to synchronize */
  
}

void WirelessSmartSensor::record(){

  /* Define your own way to record data */

}

void WirelessSmartSensor::wait(){

  /* Define your own way for the sensor to wait */

}

void WirelessSmartSensor::sendDataBackHome(){

  /* Define your own way to send data back home */

}

void WirelessSmartSensor::diagnosis(){

}

void WirelessSmartSensor::temperature(){
}
