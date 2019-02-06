/*
 * Library for Wireless Smart Sensors using Arduino MKR
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

    // Sampling Frequency
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



int WirelessSmartSensor::getFS(){

  return FS;
}



int WirelessSmartSensor::getInterval(){

  return INTERVAL;
}


  
WirelessSmartSensor::State WirelessSmartSensor::getState(){

  return sensor_state;
}



void WirelessSmartSensor::setState(State s){

  sensor_state = s;
}



void WirelessSmartSensor::printMacAddressWifi(File file){

  byte mac[6];

  // Getting mac address
  WiFi.macAddress(mac);

  // Writing it to file
  file.print("MAC: ");
  file.print(mac[5],HEX);
  file.print(":");
  file.print(mac[4],HEX);
  file.print(":");
  file.print(mac[3],HEX);
  file.print(":");
  file.print(mac[2],HEX);
  file.print(":");
  file.print(mac[1],HEX);
  file.print(":");
  file.println(mac[0],HEX);

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



String WirelessSmartSensor::getTime(){

}



String WirelessSmartSensor::getDate(){

}
