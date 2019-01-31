/*
 * Initial code for Master node that interacts with PC and WSS using Arduino 
 * MKR 1010 plataform. Uses Telnet protocol to send remote commands to arduino master
 * 
 */

#include "master.h"

void setup(){

  // Initializing all
  setupSerial();
  setupSD();
  setupWifi();
  setupADXL();

  // Static IP address
  WiFi.config(IPAddress(192, 168, 1, 24));

  // Initialize a NTPClient to get time
  timeClient.begin();

  // Initialize udp 
  udp.begin(port);

  // Start the telnet server:
  server.begin();

  Serial.println("Done setting up");
}

void wait(){
  
  WiFiClient client = server.available();
  
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
    sensor_state = SYNC;
    Serial.println("Synchronizing all sensors via NTP.");    
   
    // Tell others to sync
    udp.beginPacket(broadcast, port);
    udp.write(buf);
    udp.endPacket();
  }
  // Record
  else if (buf[0] == 'r'){
    sensor_state = RECORD;
    Serial.println("Start recording at specified time.");

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
