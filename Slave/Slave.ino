
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

  // Start the telnet server:
  server.begin();
  
}

void Slave::wait(){
  
  int packetSize = udp.parsePacket();

  if (data == 1){
    //wss.sendDataBackHome();
  }
  
  // Listening to udp
  while(!udp.parsePacket()){
    ;
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

    String formattedDate = timeClient.getFormattedDate();
    int splitT = formattedDate.indexOf("T");
    String timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
    
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

  adxl.resetDevice();
  adxl.activateMeasurementMode();

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

    // Next data in 5 milliseconds
    delay(5);
    currentMillis = millis();
  
  }
 
  wss.sensor_state = wss.WAIT;
  data = 1;

  adxl.activateStandByMode();
  
}

void Slave::sync(){
  
  // Synchronizing clocks
  timeClient.forceUpdate();
  wss.sensor_state = wss.WAIT;

}

void Slave::sendDataBackHome(){

  // Open the file for reading
  outputFile = SD.open("output.txt");
  
  // Read from the file until there's nothing else in it
  while (outputFile.available()) {
    Serial.write(outputFile.read());
  }
    
  outputFile.close();
  data = 0;
  
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
    outputFile = SD.open("output.txt", FILE_WRITE);
    wss.record();
    outputFile.close();
  }

}
