
/**
 * Code for Master node. Uses NTP sync and adxl 355.
 * 
 */

#include "Master.h"

Master wss;

void setup(){

  /* Initializing all */
  wss.setupAll();
 
  /* Static IP address */
  WiFi.config(IPAddress(192, 168, 1, 24));

}

void Master::wait(){

  String timeStamp; 
  String task = "";
  char aux = 'a', buf[8];
  int dif;
  WiFiClient client = wss.getTelnetClient();
  WiFiServer server = wss.getTelnetServer();

  client = server.available();
  
  /* Wait for a Telnet client to connect */
  while(!client){
    client = server.available();
  }

  /* Wait for the task until <ENTER> */
  while(aux != '\n') {
    
    /* Receiving task from the client */
    if (client.available() > 0 ){
      
      aux = client.read();
      task = task + aux;
    }
    /* Cleaning input */
    else{
    
      task = "";      
    }
  }
  
  task.toCharArray(buf,8);

  /* Sync */  
  if (buf[0] == 's'){
    wss.setState(wss.SYNC);
    server.write("Synchronizing all sensors via NTP.\n");    
   
    /* Tell others to sync */
    wss.sendPacketUDP(buf);
  }
  /* Record */
  else if (buf[0] == 'r'){
    wss.setState(wss.RECORD);
    server.write("Start recording at specified time.\n");

    /* Tell others to record at specified time */
    wss.sendPacketUDP(buf);
    
    timeStamp = wss.getTime();
    dif = 60 - int(((timeStamp[6] - 48) * 10) + (timeStamp[7] - 48));

    /* Start recording when the minute changes */
    delay(dif * 1000);
  }
  /* Temperature info */
  else if (buf[0] == 't'){
    wss.setState(wss.TEMP);
    server.write("Temperature reading.\n");

    /* Tell others to read temperature */
    wss.sendPacketUDP(buf);
  }
  /* Self Diagnosis */
  else if (buf[0] == 'd'){
    
  }
  /* Retrieve data back home */
  else if (buf[0] == 'g'){
    wss.setState(wss.GATHER);
    server.write("Sending data back home.");

    /* Tell others to send data back home */
    wss.sendPacketUDP(buf);
  }

}

void loop(){

  /* States */
  if (wss.getState() == wss.WAIT){
    wss.wait();
  }
  else if (wss.getState() == wss.SYNC){
    wss.sync();
  }
  else if (wss.getState() == wss.RECORD){
    wss.record();
  }
  else if (wss.getState() == wss.TEMP){
    wss.temperature();
  }
  else if (wss.getState() == wss.DIAG){
    wss.diagnosis();
  }
  else if (wss.getState() == wss.GATHER){
    wss.sendDataBackHome();
  }

}
