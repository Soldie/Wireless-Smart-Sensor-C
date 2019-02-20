/*
 * Library for Wireless Smart Sensors using Arduino MKR WiFi 1010
 * and accelerometer ADXL355 
 *
 */

#include <WirelessSmartSensor.h>

void WirelessSmartSensor::setupAll(){

  /* Initializing everything */
  setupSerial();
  setupSD();
  setupWifi();
  setupNTPClient();
  setupUDP();
  setupTelnetServer();
  adxl.setupADXL();


  /* Obtaining sampling interval from sampling frequency from .cfg */
  samplingInterval = ((float)1/(float)getFS()) * 1000.0;

}

WiFiUDP WirelessSmartSensor::getUDPConnection(){

  return udp;

}

void WirelessSmartSensor::setupUDP(){

  udp.begin(port);

}

bool WirelessSmartSensor::sendPacketUDP(char * buf){

  /* Define broadcast address */
  IPAddress broadcast(255, 255, 255, 255);

  /* Sending udp data */
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

  /* Initialize pin and variables for SD */
  pinMode(pinSelectSD, OUTPUT);
  FS = 0;
  INTERVAL = 0;
  SSID = "";
  PASS = "";  
  CONFIG_FILE = "file.cfg";

  /* Setup the SD card */
  while (!SD.begin(pinSelectSD)) {
    ;
  }
  
  /* Read our configuration from the SD card file */
  const uint8_t CONFIG_LINE_LENGTH = 127;
  
  /* The open configuration file */
  SDConfigFile cfg;
  
  /* Open the configuration file */
  while (!cfg.begin(CONFIG_FILE, CONFIG_LINE_LENGTH)) {
    ;
  }
  
  /* Read each setting from the file */
  while (cfg.readNextSetting()) {

    /* Sampling frequency */
    if (cfg.nameIs("FS")) {
      
      FS = cfg.getIntValue();
    
    /* Interval */
    } else if (cfg.nameIs("INTERVAL")) {
      
      INTERVAL = cfg.getIntValue();
   
    /* Network name */
    } else if (cfg.nameIs("SSID")) {
      
      SSID = cfg.copyValue();
   
    /* Network password */
    } else if (cfg.nameIs("PASS")) {
      
      PASS = cfg.copyValue();
    }
  }
  
  cfg.end();
  
}

void WirelessSmartSensor::setupSerial(){

  /* Initialize Serial Monitor */
  Serial.begin(9600);
  while (!Serial) {
    ;
  }

}

void WirelessSmartSensor::setupWifi(){

  status = WiFi.begin(SSID, PASS);
  
  /* While not connected, keep trying */
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

  /* Getting MAC address */
  WiFi.macAddress(mac);

  /* Writing MAC to output file */
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

  /* Synchronizing clocks */
  timeClient.forceUpdate();
  setState(WAIT);
  
  /* Getting time stamp and sending via Telnet */
  timeStamp = getTime();
  timeStamp.toCharArray(buf,20);
  
  sv.write("Time: ");
  sv.write(buf);
  sv.write("\n");
  
}

void WirelessSmartSensor::record(){

  unsigned long previousMillis;
  unsigned long t = 0;
  unsigned long it = getInterval();
  String timeStamp, dateStamp;
  int xdata = 0, ydata = 0, zdata = 0;
  int count = 0;
  String buffer = "";

  outputFile = SD.open(outputFileName, FILE_WRITE);

  /* Date and time of recording */
  timeStamp = getTime();
  dateStamp = getDate();

  /* IP */
  outputFile.print("\nIP: ");
  outputFile.print(WiFi.localIP());
  outputFile.print("\t");

  /* Arduino's MAC address */
  printMacAddressWifi(outputFile);
  outputFile.print("\n");

  /* Date stamp */
  outputFile.print(dateStamp);
  outputFile.print("\t");

  /* Time stamp */
  outputFile.print(timeStamp);
  outputFile.print("\n\n");
  
  adxl.resetDevice();

  previousMillis = millis();
    
  /* Run during time interval */
  while (t < it){ 

    /* Using real time as time stamp */
    timeStamp = getTime();
    
    /* Adding data from adxl to a string buffer, to reduce writings in the file */
    adxl.getAxis(&xdata,&ydata,&zdata);
    buffer = buffer + timeStamp + "\t" + String(xdata) + "\t" + String(ydata) + "\t" + String(zdata) + "\n";
    count = count + 1;

    /* Writing buffer in the file */
    if (count == 10){
      outputFile.print(buffer);
      count = 0;
      buffer = "";
    }

    /* It takes 1 ms to get the time stamp, the data from adxl and to put it in the buffer */
    delay(samplingInterval - 1);

    t = millis() - previousMillis;
  }
 
  setState(WAIT);

  /* Updating the next file's name */
  recordIndex = recordIndex + 1;
  outputFileName = "record";
  outputFileName = outputFileName + String(recordIndex);

  sv.write("Done recording data from sensor.\n");
  outputFile.close();

}

void WirelessSmartSensor::sendDataBackHome(){

  String lastRec = "record" + String(recordIndex - 1);

  /* Open the file for reading */
  outputFile = SD.open(lastRec, FILE_READ);

  if (!outputFile){
    sv.write("\nNo file found.....\n");
    
    setState(WAIT);
    return;
  }
  else{
    sv.write("File found.\n");
  }

  /* Read from the file until there's nothing else in it */
  while (outputFile.available()) {
    sv.write(outputFile.read());
  }
    
  outputFile.close();
  setState(WAIT);

  sv.write("Done sending data back home.\n");

}

void WirelessSmartSensor::diagnosis(){

  /* TODO */

}

void WirelessSmartSensor::temperature(){

  /* Temperature reading */
  float temperature = adxl.readTemperature();

  /* Sending it to computer's terminal */
  sv.write("Temperature: ");
  sv.print(temperature,1);
  sv.write("\n");

  setState(WAIT);

}

String WirelessSmartSensor::getTime(){

  String formattedDate, timeStamp;
  int splitT;

  /* Getting time from NTP */
  formattedDate = timeClient.getFormattedDate();
  splitT = formattedDate.indexOf("T");
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
 
  return timeStamp;

}

String WirelessSmartSensor::getDate(){

  String formattedDate, dayStamp;
  int splitT;

  /* Getting date from NTP */
  formattedDate = timeClient.getFormattedDate();
  splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  
  return dayStamp;

}

void WirelessSmartSensor::wait(){

  /* Define your own way for the wss to wait */

}
