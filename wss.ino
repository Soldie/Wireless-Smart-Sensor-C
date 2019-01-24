
/*
 * Initial code for Wireless Smart Sensors using Arduino MKR 
 * WiFi 1010 plataform and accelerometer ADXL355. For while,
 * using NTP to synchronize clocks with a NTP server (probably it will change).
 * 
 */

#include "wss.h"

void setup(){

  // Initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect
  }
  
//  pinMode(pinSelectSD, OUTPUT);
//
//  didReadConfig = false;
//  FS = 0;
//  INTERVAL = 0;
//  SSID = "";
//  PASS = "";  
//
//  /**********************DEBUG*******************************/
//  // Setup the SD card 
//  Serial.println("Calling SD.begin()...");
//  if (!SD.begin(pinSelectSD)) {
//    Serial.println("SD.begin() failed. Check: ");
//    Serial.println("  card insertion,");
//    Serial.println("  SD shield I/O pins and chip select,");
//    Serial.println("  card formatting.");
//    return;
//  }
//  Serial.println("...succeeded.");
//  /**********************************************************/
//
//  // Read our configuration from the SD card file.
//  didReadConfig = readConfiguration();

  // Connecting to WiFi
  Serial.print("Connecting to ");
  Serial.println(SSID);
  
  status = WiFi.begin(SSID, PASS);
  
  while (status != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    
    status = WiFi.begin(SSID, PASS);
  }

  /*******************DEBUG*************************/
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  /*******************DEBUG*************************/

  // Initialize a NTPClient to get time
  timeClient.begin();

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


//boolean readConfiguration() {
//  /*
//   * Length of the longest line expected in the config file.
//   * The larger this number, the more memory is used
//   * to read the file.
//   * You probably won't need to change this number.
//   */
//  const uint8_t CONFIG_LINE_LENGTH = 127;
//  
//  // The open configuration file.
//  SDConfigFile cfg;
//  
//  // Open the configuration file.
//  if (!cfg.begin(CONFIG_FILE, CONFIG_LINE_LENGTH)) {
//    Serial.print("Failed to open configuration file: ");
//    Serial.println(CONFIG_FILE);
//    return false;
//  }
//  
//  // Read each setting from the file.
//  while (cfg.readNextSetting()) {
//
//    // Sampling Rate
//    if (cfg.nameIs("FS")) {
//      
//      FS = cfg.getIntValue();
//      // DEBUG
//      Serial.print("Read FS: ");
//      if (doDelay) {
//        Serial.println("true");
//      } else {
//        Serial.println("false");
//      }
//      // DEBUG
//    
//    // Interval
//    } else if (cfg.nameIs("INTERVAL")) {
//      
//      INTERVAL = cfg.getIntValue();
//      //DEBUG
//      Serial.print("Read INTERVAL: ");
//      Serial.println(waitMs);
//      //DEBUG
//  
//    // Network name
//    } else if (cfg.nameIs("SSID")) {
//      
//      SSID = cfg.copyValue();
//      //DEBUG
//      Serial.print("Read SSID: ");
//      Serial.println(SSID);
//      //DEBUG
//
//    // Network password
//    } else if (cfg.nameIs("PASS")) {
//      
//      PASS = cfg.copyValue();
//      //DEBUG
//      Serial.print("Read PASS: ");
//      Serial.println(PASS);
//      //DEBUG
//    }
//  }
//  
//  // clean up
//  cfg.end();
//  
//  return true;
//}


void wait(){
  
  // Listening to serial port
  while(Serial.available() != 0){

    /*******************DEBUG*************************************************/
    // The formattedDate comes with the following format:
    // 2018-05-28T16:00:13Z
    // We need to extract date and time
    formattedDate = timeClient.getFormattedDate();
    Serial.println(formattedDate);

    // Extract date
    int splitT = formattedDate.indexOf("T");
    dayStamp = formattedDate.substring(0, splitT);
    Serial.print("DATE: ");
    Serial.println(dayStamp);
    // Extract time
    timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
    Serial.print("HOUR: ");
    Serial.println(timeStamp);
    delay(1000);
    /*******************DEBUG*************************************************/
    
  }

  String task;
  task = Serial.readString();

  // New state depending on the task sent
  if (task[0] == 's'){
    sensor_state = SYNC;
  }
  else if (task[0] == 'r'){
    sensor_state = RECORD;

    int hours = (task[2] * 10) + task[3];
    int minutes = (task[5] * 10) + task[6];

    while(hours >= hour() && minutes > minute()){
      // Wait until reaches the time to start recording
    }
  }
  
}


void sync(){

  Serial.print("Synchronizing clocks");

  // Synchronizing clocks
  timeClient.forceUpdate();
  
  // Go back to WAIT state
  sensor_state = WAIT;

  Serial.print("Done");
  
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
