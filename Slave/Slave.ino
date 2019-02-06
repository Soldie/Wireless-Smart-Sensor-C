
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

  String timeStamp;
  char task[255];
  int len;
  int packetSize = udp.parsePacket();
  
  // Listening to udp
  while(!udp.parsePacket()){
    ;
  }

  // Receiving packet
  len = udp.read(task, 255);
  
  if (len > 0){
    task[len] = 0;
  }

  // New state depending on the task sent
  if (task[0] == 's'){
    wss.setState(wss.SYNC);
  }
  else if (task[0] == 'r'){
    wss.setState(wss.RECORD);

    int hours = (task[2] * 10) + task[3];
    int minutes = (task[5] * 10) + task[6];

    timeStamp = wss.getTime();
    
    while(hours >= int((timeStamp[0] * 10) + timeStamp[1]) && minutes > int((timeStamp[3] * 10) + timeStamp[4])){
      // Wait until reaches the time to start recording
      timeStamp = wss.getTime();
    }
    
  }
  else if (task[0] == 't'){
    wss.setState(wss.TEMP);
    
    server.write("Temperature reading.\n");
  }
  else if (task[0] == 'd'){
    
  }
  else if (task[0] == 'g'){
    wss.setState(wss.GATHER);

    server.write("Sending data back home.");
  }
  
}

void Slave::record(){

  unsigned long previousMillis;
  unsigned long currentMillis;
  int xdata, ydata, zdata;
  String timeStamp, dateStamp;
  int samplingInterval = (1/wss.getFS()) * 1000.0;

  timeStamp = wss.getTime();
  dateStamp = wss.getDate();

  // Ip 
  outputFile.print("IP: ");
  outputFile.print(WiFi.localIP());
  outputFile.print("\t");

  // Arduino's MAC address
  wss.printMacAddressWifi(outputFile);
  outputFile.print("\n");

  // Date stamp
  outputFile.print(dateStamp);
  outputFile.print("\t");

  // Time stamp
  outputFile.print(timeStamp);
  outputFile.print("\n\n");
  
  adxl.resetDevice();

  previousMillis = millis();
  currentMillis = previousMillis;

  // Run during time interval 
  while ((currentMillis - previousMillis) < getInterval()){ 

    adxl.getAxis(&xdata,&ydata,&zdata);

    // X axis
    outputFile.print(xdata);
    outputFile.print("\t");

    // Y axis
    outputFile.print(ydata);
    outputFile.print("\t");

    // Z axis
    outputFile.print(zdata);
    outputFile.print("\n");

    // Sampling frequency
    delay(samplingInterval);
    currentMillis = millis();
  
  }
 
  wss.setState(wss.WAIT);

  recordIndex = recordIndex + 1;
  outputFileName = "record";
  outputFileName = outputFileName + String(recordIndex);
  
}

void Slave::sync(){
  
  // Synchronizing clocks
  timeClient.forceUpdate();
  wss.setState(wss.WAIT);

}

void Slave::temperature(){

  // Temperature reading
  float temperature = adxl.readTemperature();

  // Sending it to computer's terminal
  server.write("Temperature: ");
  server.print(temperature,1);
  server.write("\n");

  wss.setState(wss.WAIT);
}

void Slave::sendDataBackHome(){

  server.write("Waiting for the name of the file.\n");
 
  // Listening to udp
  while(!udp.parsePacket()){
    ;
  }

  // Receiving packet
  char fileName[255];
  int len = udp.read(fileName, 255);
  
  if (len > 0){
    fileName[len] = 0;
  }

  // Open the file for reading
  outputFile = SD.open(fileName);

  if (!outputFile){
    server.write("No file found.\n");
    return;
  }
  else{
    server.write("File found.\n");
  }
  
  // Read from the file until there's nothing else in it
  while (outputFile.available()) {
    server.write(outputFile.read());
  }
    
  outputFile.close();
  wss.setState(wss.WAIT);

  server.write("Done sending data back home.\n");
}

String Slave::getTime(){

  String formattedDate, timeStamp;
  int splitT;

  formattedDate = timeClient.getFormattedDate();
  splitT = formattedDate.indexOf("T");
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  
  return timeStamp;

}

String Slave::getDate(){

  String formattedDate, dayStamp;
  int splitT;

  formattedDate = timeClient.getFormattedDate();
  splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  
  return dayStamp;

}

void loop(){

  // States
  if (wss.getState() == wss.WAIT){
    wss.wait();
  }
  else if (wss.getState() == wss.SYNC){
    wss.sync();
  }
  else if (wss.getState() == wss.RECORD){
    outputFile = SD.open(outputFileName, FILE_WRITE);
    wss.record();
    outputFile.close();
  }
  else if (wss.getState() == wss.TEMP){
    wss.temperature();
  }
  else if (wss.getState() == wss.DIAG){
    
  }
  else if (wss.getState() == wss.GATHER){
    wss.sendDataBackHome();
  }

}
