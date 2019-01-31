
/*
 * Initial code for Wireless Smart Sensors (SLAVE NODES) using Arduino MKR 
 * WiFi 1010 plataform and accelerometer ADXL355. For while, we are
 * using NTP to synchronize clocks with a NTP server (probably it will change).
 * 
 */

#include "Slave.h"

Slave wss;

void setup(){

  // Initializing all
  wss.setupAll();
  adxl.setupADXL();

  // Initialize a NTPClient to get time
  timeClient.begin();

  // Initialize udp 
  udp.begin(port);

  Serial.println("Done setting up");
}

void Slave::wait(){
  
  int packetSize = udp.parsePacket();
  
  // Variables to save date and time
  String formattedDate;
  String dayStamp;
  String timeStamp; 
  
  // Listening to udp
  while(!udp.parsePacket()){
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

  // Receiving packet
  char task[255];
  int len = udp.read(task, 255);
  
  if (len > 0){
    task[len] = 0;
  }

  // New state depending on the task sent
  if (task[0] == 's'){
    wss.sensor_state = wss.SYNC;
  }
  else if (task[0] == 'r'){
    wss.sensor_state = wss.RECORD;

    int hours = (task[2] * 10) + task[3];
    int minutes = (task[5] * 10) + task[6];

    formattedDate = timeClient.getFormattedDate();
    int splitT = formattedDate.indexOf("T");
    timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
    
    while(hours >= int((timeStamp[0] * 10) + timeStamp[1]) && minutes > int((timeStamp[3] * 10) + timeStamp[4])){
      // Wait until reaches the time to start recording
      formattedDate = timeClient.getFormattedDate();
      int splitT = formattedDate.indexOf("T");
      timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
    }
  }
  
}

void Slave::record(){

  unsigned long previousMillis = millis();
  unsigned long currentMillis = previousMillis;
  File outputFile = SD.open("output.txt", FILE_WRITE);

  // Waiting for the file to be created
  while (!outputFile){
    ;
  }
  
  // Run during time interval 
  while ((currentMillis - previousMillis) < INTERVAL){ 

    int axisAddresses[] = {adxl.XDATA1, adxl.XDATA2, adxl.XDATA3, adxl.YDATA1, adxl.YDATA2, adxl.YDATA3, adxl.ZDATA1, adxl.ZDATA2, adxl.ZDATA3};
    int axisMeasures[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    int dataSize = 9;

    // Read accelerometer data
    adxl.readMultipleData(axisAddresses, dataSize, axisMeasures);

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
    //Serial.print("X=");
    Serial.print(xdata);
    Serial.print("\t");
  
    //Serial.print("Y=");
    Serial.print(ydata);
    Serial.print("\t");

    //Serial.print("Z=");
    Serial.print(zdata);
    Serial.print("\n");

    // Next data in 100 milliseconds
    delay(100);
    currentMillis = millis();
  
  }
 
  outputFile.close();
  wss.sensor_state = wss.WAIT;

  Serial.println("Done recording");
}

void Slave::sync(){
  
  Serial.println("Synchronizing clocks");

  // Synchronizing clocks
  timeClient.forceUpdate();
  
  wss.sensor_state = wss.WAIT;
  Serial.println("Synchronization is over");
}

void loop(){

  // States
  if (wss.sensor_state == wss.WAIT){
    wss.wait();
  }
  else if (wss.sensor_state == wss.SYNC){
    wss.sync();
  }
  else if (wss.sensor_state == wss.RECORD){
    wss.record();
  }

}
