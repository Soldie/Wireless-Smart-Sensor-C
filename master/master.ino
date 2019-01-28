/*
 * Initial code for Master node that interacts with PC and WSS using Arduino 
 * MKR 1000 plataform
 * 
 */

#include "master.h"

void setup(){

  // Initializing all
  setupSerial();
  setupSD();
  setupWifi();

  // Initialize a NTPClient to get time
  timeClient.begin();

  // Initialize udp 
  udp.begin(port);

}

void setupSD(){

  // Initialize pin and variables for SD 
  pinMode(pinSelectSD, OUTPUT);
  FS = 0;
  SSID = "";
  PASS = "";  

  // Setup the SD card 
  Serial.println("Connecting to SD card");
  while (!SD.begin(pinSelectSD)) {
    Serial.print(".");
  }

  // Read our configuration from the SD card file.
  readConfiguration();
  
}

void setupSerial(){

  // Initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect
  }
}

void setupWifi(){

  // Connecting to WiFi
  Serial.print("Connecting to ");
  Serial.println(SSID);
  Serial.println(PASS);
  
  status = WiFi.begin("VTR-2582559", "4wptdnBHjfxz");
  
  while (status != WL_CONNECTED) {
    delay(500);
    
    status = WiFi.begin("VTR-2582559", "4wptdnBHjfxz");
  }

  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

bool readConfiguration() {
  /*
   * Length of the longest line expected in the config file
   * The larger this number, the more memory is used
   * to read the file
   */
  const uint8_t CONFIG_LINE_LENGTH = 127;
  
  // The open configuration file
  SDConfigFile cfg;
  
  // Open the configuration file
  while (!cfg.begin(CONFIG_FILE, CONFIG_LINE_LENGTH)) {
    Serial.print("Failed to open configuration file: ");
    Serial.println(CONFIG_FILE);
  }
  
  // Read each setting from the file
  while (cfg.readNextSetting()) {

    // Sampling Rate
    if (cfg.nameIs("FS")) {
      
      FS = cfg.getIntValue();
       
    // Network name
    } else if (cfg.nameIs("SSID")) {
      
      SSID = cfg.copyValue();
   
    // Network password
    } else if (cfg.nameIs("PASS")) {
      
      PASS = cfg.copyValue();
    }
  }
  
  cfg.end();
  return true;
}

void loop(){

  String task = "";
  char buf[8];

  // Listening to serial port
  if(Serial.available() != 0){
    
    task = Serial.readString();
  
    if (task[0] == 's'){
      Serial.println("Synchronizing all sensors via NTP.");    
      timeClient.forceUpdate();
    }
    else{
      Serial.println("Start recording at specified time.");
    }

    task.toCharArray(buf,8);

    // Tell others to sync
    udp.beginPacket(broadcast, port);
    udp.write(buf);
    udp.endPacket();
  }
  
}
