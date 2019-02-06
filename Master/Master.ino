
/*
 * Initial code for Wireless Smart Sensors (SLAVE NODES) using Arduino MKR 
 * WiFi 1010 plataform and accelerometer ADXL355. For while, we are
 * using NTP to synchronize clocks with a NTP server (probably it will change).
 * 
 */

#include "Master.h"

Master wss;

void setup(){

  // Initializing all
  wss.setupAll();
  adxl.setupADXL();

  // Static IP address
  WiFi.config(IPAddress(192, 168, 1, 24));

  // Initialize a NTPClient to get time
  timeClient.begin();

  // Initialize udp 
  udp.begin(port);

  // Start the telnet server:
  server.begin();

}

void Master::wait(){

  String timeStamp; 
  String task = "";
  char aux = 'a', buf[8];
  
  client = server.available();
  
  // Wait for a Telnet client to connect
  while(!client){
    client = server.available();
  }

  // Wait for the task until <ENTER>
  while(aux != '\n') {
    
    // Receiving task from the client
    if (client.available() > 0 ){
      
      aux = client.read();
      task = task + aux;
    }
    // Cleaning input
    else{
    
      task = "";      
    }
  }
  
  task.toCharArray(buf,8);

  // Sync  
  if (buf[0] == 's'){
    wss.setState(wss.SYNC);
    server.write("Synchronizing all sensors via NTP.\n");    
   
    // Tell others to sync
    udp.beginPacket(broadcast, port);
    udp.write(buf);
    udp.endPacket();
  }
  // Record
  else if (buf[0] == 'r'){
    wss.setState(wss.RECORD);
    server.write("Start recording at specified time.\n");

    // Tell others to record at specified time
    udp.beginPacket(broadcast, port);
    udp.write(buf);
    udp.endPacket();
    
    int hours = (task[2] * 10) + task[3];
    int minutes = (task[5] * 10) + task[6];

    timeStamp = wss.getTime();
    
    while(hours >= int((timeStamp[0] * 10) + timeStamp[1]) && minutes > int((timeStamp[3] * 10) + timeStamp[4])){
      // Wait until reaches the time to start recording
      timeStamp = wss.getTime();
    }
  }
  // Temperature info
  else if (buf[0] == 't'){
    wss.setState(wss.TEMP);
    server.write("Temperature reading.\n");

    // Tell others to read temperature
    udp.beginPacket(broadcast, port);
    udp.write(buf);
    udp.endPacket();
  }
  // Self Diagnosis
  else if (buf[0] == 'd'){
    
  }
  // Retrieve data back home
  else if (buf[0] == 'g'){
    wss.setState(wss.GATHER);
    server.write("Sending data back home.");

    // Tell others to send data back home
    udp.beginPacket(broadcast, port);
    udp.write(buf);
    udp.endPacket();
  }

}

void Master::record(){

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

  server.write("Done recording data from sensor.\n");
}

void Master::temperature(){

  // Temperature reading
  float temperature = adxl.readTemperature();

  // Sending it to computer's terminal
  server.write("Temperature: ");
  server.print(temperature,1);
  server.write("\n");

  wss.setState(wss.WAIT);
}

void Master::sync(){
  
  String timeStamp;
  char buf[10];
  
  // Synchronizing clocks
  timeClient.forceUpdate();
  wss.setState(wss.WAIT);
  
  timeStamp = wss.getTime();
  timeStamp.toCharArray(buf,10);
  
  server.write("Time: ");
  server.write(buf);
  server.write("\n");

}

void Master::sendDataBackHome(){

  server.write("Please, insert the name of the file.\n");
  
  String fileName = "";
  char aux = 'a';

  // Wait for the file name until <ENTER>
  while(aux != '\n') {
    
    // Receiving file name from the client
    if (client.available() > 0 ){
      
      aux = client.read();
      
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
  udp.beginPacket(broadcast, port);
  udp.write(buf);
  udp.endPacket();

  // Open the file for reading
  outputFile = SD.open(fileName, FILE_READ);

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

String Master::getTime(){

  String formattedDate, timeStamp;
  int splitT;

  formattedDate = timeClient.getFormattedDate();
  splitT = formattedDate.indexOf("T");
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  
  return timeStamp;

}

String Master::getDate(){

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
