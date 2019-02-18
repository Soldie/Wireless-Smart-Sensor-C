/*
 * Library for Wireless Smart Sensors using Arduino MKR WiFi 1010
 * and accelerometer ADXL355 
 *
 */

#include <WirelessSmartSensor.h>

void WirelessSmartSensor::setupAll(){

  // Initializing Serial, SD and Wifi
  setupSerial();
  
  setupSD();

  setupWifi();
  
  setupNTPClient();

  setupUDP();

  setupTelnetServer();

  adxl.setupADXL();

  samplingInterval = ((float)1/(float)getFS()) * 1000.0;

}



WiFiUDP WirelessSmartSensor::getUDPConnection(){

  return udp;

}



void WirelessSmartSensor::setupUDP(){

  udp.begin(port);

}



bool WirelessSmartSensor::sendPacketUDP(char * buf){

  // Define broadcast address
  IPAddress broadcast(255, 255, 255, 255);

  // Sending udp data 
  udp.beginPacket(broadcast, port);
  udp.write(buf);
  udp.endPacket();

  return true;

}



WiFiClient WirelessSmartSensor::getTelnetClient(){

  return cl;

}



WiFiServer WirelessSmartSensor::getTelnetServer(){

  return sv;

}



void WirelessSmartSensor::setupTelnetServer(){

  WiFiServer server(23);
  sv = server;
  sv.begin();
}



void WirelessSmartSensor::setupNTPClient(){

  NTPClient tc(ntpUDP, "192.168.1.212", 0);
  timeClient = tc;
  timeClient.begin();

}



NTPClient WirelessSmartSensor::getNTPClient(){

  return timeClient;

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

  String timeStamp;
  char buf[20];
 
  // Synchronizing clocks
  timeClient.forceUpdate();
  setState(WAIT);
  
  timeStamp = getTime();
  timeStamp.toCharArray(buf,20);
  
  sv.write("Time: ");
  sv.write(buf);
  sv.write("\n");
  
}



void WirelessSmartSensor::record(){

  outputFile = SD.open(outputFileName, FILE_WRITE);

  unsigned long previousMillis, currentMillis, t = 0, ts, cron, it = getInterval();
  String timeStamp, dateStamp;
  int xdata = 0, ydata = 0, zdata = 0, count = 0, xaxis = 0;
  char buffer[10000];
 
  buffer[0] = '\0';

  //String timeStamp = timeClient.getMilliSecond();
  //dateStamp = getDate();
  
  adxl.resetDevice();

  previousMillis = millis();
  currentMillis = previousMillis;
  outputFile.println(timeClient.getMilliSecond());
    
  // Run during time interval 
  while (t < it){ 

    ts = timeClient.getMilliSecondL();
    
    ts = ts - 1 * (t/2230); 
    
    //Serial.println(timeStamp);
    adxl.getAxis(&xdata,&ydata,&zdata);
    sprintf(buffer + strlen(buffer),"%d\t%d\t%d\t%d\n", ts, xdata, ydata, zdata);
    //outputFile.print(buffer);
    
    count = count + 1;
    
    if (count == 170){
      outputFile.print(buffer);
      count = 0;
      buffer[0] = '\0';
      cron = timeClient.getMilliSecondL();
      while (cron < ts + 100){
        cron = timeClient.getMilliSecondL(); 
      }
    }

    // Sampling frequency
    //delay((int) samplingInterval);
    //if (t % 3000 == 0){
    //  delay(3); 
    //}
    //else{
      delayMicroseconds(2000);
    //}

    currentMillis = millis();
    t = currentMillis - previousMillis;
    xaxis = xaxis + 1;
  }
  outputFile.println(timeClient.getMilliSecond());
  
  // Ip 
  outputFile.print("\nIP: ");
  outputFile.print(WiFi.localIP());
  outputFile.print("\t");

  // Arduino's MAC address
  printMacAddressWifi(outputFile);
  outputFile.print("\n");

  // Date stamp
  //outputFile.print(dateStamp);
  outputFile.print("\t");

  // Time stamp
//  outputFile.print(timeStamp);
  outputFile.print(":");
  //outputFile.print(delta);
  outputFile.print("\n\n");
 
  setState(WAIT);

  recordIndex = recordIndex + 1;
  outputFileName = "record";
  outputFileName = outputFileName + String(recordIndex);

  sv.write("Done recording data from sensor.\n");
  
  outputFile.close();

}



void WirelessSmartSensor::sendDataBackHome(){

  sv.write("Please, insert the name of the file.\n");
  
  String fileName = "";
  char aux = 'a';

  // Wait for the file name until <ENTER>
  while(aux != '\n') {
    
    // Receiving file name from the client
    if (cl.available() > 0 ){
      
      aux = cl.read();
      
      if (aux != '\n'){
        fileName = fileName + aux;
      }  
    }
    // Cleaning input
    else{
    
      fileName = "";      
    }
  }

  char buf[15];
  fileName.toCharArray(buf,15);

  // Tell others the name of the file to be read
  sendPacketUDP(buf);
  

  // Open the file for reading
  outputFile = SD.open(fileName, FILE_READ);

  if (!outputFile){
    sv.write("No file found.\n");
    return;
  }
  else{
    sv.write("File found.\n");
  }

  // Read from the file until there's nothing else in it
  while (outputFile.available()) {
    sv.write(outputFile.read());
  }
    
  outputFile.close();
  setState(WAIT);

  sv.write("Done sending data back home.\n");

}



void WirelessSmartSensor::diagnosis(){

}



void WirelessSmartSensor::temperature(){

  // Temperature reading
  float temperature = adxl.readTemperature();

  // Sending it to computer's terminal
  sv.write("Temperature: ");
  sv.print(temperature,1);
  sv.write("\n");

  setState(WAIT);

}



String WirelessSmartSensor::getTime(){

  String formattedDate, timeStamp;
  int splitT;

  // Getting time from NTP
  formattedDate = timeClient.getFormattedDate();
  splitT = formattedDate.indexOf("T");
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
 
  return timeStamp;
}



String WirelessSmartSensor::getDate(){

  String formattedDate, dayStamp;
  int splitT;

  // Getting date from NTP
  formattedDate = timeClient.getFormattedDate();
  splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  
  return dayStamp;

}



void WirelessSmartSensor::wait(){

  /* Define your own way for the sensor to wait */

}
