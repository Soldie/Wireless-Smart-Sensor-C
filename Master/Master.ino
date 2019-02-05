
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

  client = server.available();
  
  // Variables to save date and time
  String formattedDate;
  String dayStamp;
  String timeStamp; 
  
  // Wait for a Telnet client to connect
  while(!client){
    client = server.available();
  }

  String task = "";
  char aux = 'a';

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
  
  char buf[8];
  task.toCharArray(buf,8);

  // Sync  
  if (buf[0] == 's'){
    wss.sensor_state = wss.SYNC;
    server.write("Synchronizing all sensors via NTP.\n");    
   
    // Tell others to sync
    udp.beginPacket(broadcast, port);
    udp.write(buf);
    udp.endPacket();
  }
  // Record
  else if (buf[0] == 'r'){
    wss.sensor_state = wss.RECORD;
    server.write("Start recording at specified time.\n");

    // Tell others to record at specified time
    udp.beginPacket(broadcast, port);
    udp.write(buf);
    udp.endPacket();
    
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
  // Temperature info
  else if (buf[0] == 't'){
    wss.sensor_state = wss.TEMP;
    
    server.write("Temperature reading.\n");
  }
  // Self Diagnosis
  else if (buf[0] == 'd'){
    
  }
  // Retrieve data back home
  else if (buf[0] == 'h'){
    wss.sensor_state = wss.HOME;

    server.write("Sending data back home.");
  }

}

void Master::record(){

  int *xdata, *ydata, *zdata;

  unsigned long previousMillis = millis();
  unsigned long currentMillis = previousMillis;

  adxl.resetDevice();

  // Run during time interval 
  while ((currentMillis - previousMillis) < INTERVAL){ 

    adxl.getAxis(xdata,ydata,zdata);

    // Print axis
    outputFile.print(*xdata);
    outputFile.print("\t");
  
    //Serial.print("Y=");
    outputFile.print(*ydata);
    outputFile.print("\t");

    //Serial.print("Z=");
    outputFile.print(*zdata);
    outputFile.print("\n");

    // Next data in 5 milliseconds
    delay(5);
    currentMillis = millis();
  
  }
 
  wss.sensor_state = wss.WAIT;
  
  recordIndex = recordIndex + 1;
  outputFileName = "record";
  outputFileName = outputFileName + String(recordIndex);
  
  server.write("Done recording data from sensor. \n");
}

void Master::temperature(){

  // Temperature reading
  float temperature = adxl.readTemperature();

  // Sending it to computer's terminal
  server.write("Temperature: ");
  server.print(temperature,1);
  server.write("\n");

  wss.sensor_state = wss.WAIT;
}

void Master::sync(){
  
  String formattedDate, timeStamp;
  int splitT;
  char buf[10];
  
  // Synchronizing clocks
  timeClient.forceUpdate();
  wss.sensor_state = wss.WAIT;

  formattedDate = timeClient.getFormattedDate();
  splitT = formattedDate.indexOf("T");
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
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
      fileName = fileName + aux;
    }
    // Cleaning input
    else{
    
      fileName = "";      
    }
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
  wss.sensor_state = wss.WAIT;

  server.write("Done sending data back home.\n");
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
    outputFile = SD.open(outputFileName, FILE_WRITE);
    wss.record();
    outputFile.close();
  }
  else if (wss.sensor_state == wss.TEMP){
    wss.temperature();
  }
  else if (wss.sensor_state == wss.DIAG){
    
  }
  else if (wss.sensor_state == wss.HOME){
    wss.sendDataBackHome();
  }

}
