
/*
 * Initial code for Wireless Smart Sensors (SLAVE NODES) using Arduino MKR 
 * WiFi 1010 plataform and accelerometer ADXL355. For while, we are
 * using NTP to synchronize clocks with a NTP server (probably it will change).
 * 
 */

#include "wss_serial.h"

void setup(){

  // Initializing all
  wss.setupSerial();
  wss.setupSD();
  wss.setupWifi();
  adxl.setupADXL();

  // Initialize a NTPClient to get time
  timeClient.begin();

  // Initialize udp 
  udp.begin(port);

  Serial.println("Done setting up");
}

void wait(){
  
  int packetSize = udp.parsePacket();
  
  // Listening to udp
  while(!udp.parsePacket()){
//    /*******************DEBUG*************************************************/
//    // The formattedDate comes with the following format:
//    // 2018-05-28T16:00:13Z
//    // We need to extract date and time
//    formattedDate = timeClient.getFormattedDate();
//    Serial.println(formattedDate);
//
//     // Extract date
//    int splitT = formattedDate.indexOf("T");
//    dayStamp = formattedDate.substring(0, splitT);
//    Serial.print("DATE: ");
//    Serial.println(dayStamp);
//    // Extract time
//    timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
//    Serial.print("HOUR: ");
//    Serial.println(timeStamp);
//    delay(1000);
//    /*******************DEBUG*************************************************/
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

void loop(){

  // States
  if (wss.sensor_state == wss.WAIT){
    wait();
  }
  else if (wss.sensor_state == wss.SYNC){
    sync();
  }
  else if (wss.sensor_state == wss.RECORD){
    record();
  }

}
