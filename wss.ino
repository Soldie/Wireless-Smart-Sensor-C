
/*
 * Initial code for Wireless Smart Sensors (SLAVE NODES) using Arduino MKR 
 * WiFi 1010 plataform and accelerometer ADXL355. For while, we are
 * using NTP to synchronize clocks with a NTP server (probably it will change).
 * 
 */

#include "wss.h"

void setup(){

  // Initializing all
  setupSD();
  setupWifi();
  setupADXL();

  // Initialize a NTPClient to get time
  timeClient.begin();

  // Initialize udp 
  udp.begin(port);
}

void setupADXL(){

  // Initialize SPI to interact with ADXL355
  SPI.begin();

  // Initalize the  data ready and chip select pins:
  pinMode(CHIP_SELECT_PIN, OUTPUT);

  //Configure ADXL355:
  writeRegister(RANGE, RANGE_2G); 
  writeRegister(POWER_CTL, MEASURE_MODE); 

  // Give the sensor time to set up:
  delay(100);
}

void setupSD(){

  // Initialize pin and variables for SD 
  pinMode(pinSelectSD, OUTPUT);
  FS = 0;
  INTERVAL = 0;
  SSID = "";
  PASS = "";  

  // Setup the SD card 
  if (!SD.begin(pinSelectSD)) {
    
    return;
  }

  // Read our configuration from the SD card file.
  readConfiguration();
}

void setupWifi(){
  
  status = WiFi.begin(SSID, PASS);
  
  while (status != WL_CONNECTED) {
    
    delay(500);
    status = WiFi.begin(SSID, PASS);
  }
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
  if (!cfg.begin(CONFIG_FILE, CONFIG_LINE_LENGTH)) {
    
    return false;
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
  
  return true;
}


void wait(){

  int packetSize = udp.parsePacket();
  
  // Listening to serial port
  while(!packetSize){
    packetSize = udp.parsePacket();
  }

  // Receiving packet
  char task[255];
  int len = udp.read(task, 255);
  
  if (len > 0){
    task[len] = 0;
  }

  // New state depending on the task sent
  if (task[0] == 's'){
    sensor_state = SYNC;
  }
  else if (task[0] == 'r'){
    sensor_state = RECORD;

    int hours = (task[2] * 10) + task[3];
    int minutes = (task[5] * 10) + task[6];

    formattedDate = timeClient.getFormattedDate();
    int splitT = formattedDate.indexOf("T");
    timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
    
    while(hours >= int((timeStamp[0] * 10) + timeStamp[1]) && minutes > int((timeStamp[3] * 10) + timeStamp[4])){
      ;// Wait until reaches the time to start recording
    }
    
  }
}


void sync(){

  // Synchronizing clocks
  timeClient.forceUpdate();
  
  // Go back to WAIT state
  sensor_state = WAIT;
}


void writeRegister(byte thisRegister, byte thisValue) {
  byte dataToSend = (thisRegister << 1) | WRITE_BYTE;
  digitalWrite(CHIP_SELECT_PIN, LOW);
  SPI.transfer(dataToSend);
  SPI.transfer(thisValue);
  digitalWrite(CHIP_SELECT_PIN, HIGH);
}


unsigned int readRegistry(byte thisRegister) {
  unsigned int result = 0;
  byte dataToSend = (thisRegister << 1) | READ_BYTE;

  digitalWrite(CHIP_SELECT_PIN, LOW);
  SPI.transfer(dataToSend);
  result = SPI.transfer(0x00);
  digitalWrite(CHIP_SELECT_PIN, HIGH);
  return result;
}


void readMultipleData(int *addresses, int dataSize, int *readedData) {
  digitalWrite(CHIP_SELECT_PIN, LOW);
  for(int i = 0; i < dataSize; i = i + 1) {
    byte dataToSend = (addresses[i] << 1) | READ_BYTE;
    SPI.transfer(dataToSend);
    readedData[i] = SPI.transfer(0x00);
  }
  digitalWrite(CHIP_SELECT_PIN, HIGH);
}


void record(){

  unsigned long previousMillis = millis();
  unsigned long currentMillis = previousMillis;

  // Run during time interval 
  while ((currentMillis - previousMillis) < INTERVAL){ 

    int axisAddresses[] = {XDATA1, XDATA2, XDATA3, YDATA1, YDATA2, YDATA3, ZDATA1, ZDATA2, ZDATA3};
    int axisMeasures[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    int dataSize = 9;

    // Read accelerometer data
    readMultipleData(axisAddresses, dataSize, axisMeasures);

    // Split data
    int xdata = (axisMeasures[0] >> 4) + (axisMeasures[1] << 4) + (axisMeasures[2] << 12);
    int ydata = (axisMeasures[3] >> 4) + (axisMeasures[4] << 4) + (axisMeasures[5] << 12);
    int zdata = (axisMeasures[6] >> 4) + (axisMeasures[7] << 4) + (axisMeasures[8] << 12);
  
    // Apply two complement
    if (xdata >= 0x80000) {
      xdata = ~xdata + 1;
    }
    if (ydata >= 0x80000) {
      ydata = ~ydata + 1;
    }
    if (zdata >= 0x80000) {
      zdata = ~zdata + 1;
    }

    // Print axis
    Serial.print("X=");
    Serial.print(xdata);
    Serial.print("\t");
  
    Serial.print("Y=");
    Serial.print(ydata);
    Serial.print("\t");

    Serial.print("Z=");
    Serial.print(zdata);
    Serial.print("\n");

    // Next data in 100 milliseconds
    delay(100);
    currentMillis = millis();
  }

  // Go back to WAIT state
  sensor_state = WAIT;
}


void loop(){

  // States
  if (sensor_state == WAIT){
    wait();
  }
  else if (sensor_state == SYNC){
    sync();
  }
  else if (sensor_state == RECORD){
    record();
  }
}
