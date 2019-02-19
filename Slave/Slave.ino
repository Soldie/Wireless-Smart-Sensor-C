
/**
 * Code for Slave node. Uses NTP sync and adxl 355.
 * 
 */

#include "Slave.h"

Slave wss;

void setup(){

  /* Initializing all */
  wss.setupAll();
  
}

void Slave::wait(){

  String timeStamp;
  char task[255];
  WiFiUDP udp = wss.getUDPConnection();
  int packetSize = udp.parsePacket();
  int dif;
  WiFiClient client = wss.getTelnetClient();
  WiFiServer server = wss.getTelnetServer();

  /* Listening to udp */
  while(!udp.parsePacket()){
    ;
  }
    
  /* Receiving packet */
  udp.read(task, 255);

  /* New state depending on the task sent */
  if (task[0] == 'r'){
    wss.setState(wss.RECORD);
  
    timeStamp = wss.getTime();
    dif = 60 - int(((timeStamp[6] - 48) * 10) + (timeStamp[7] - 48));

    /* Start recording when the minute changes */
    delay(dif * 1000);
  }
  else if (task[0] == 's'){
    wss.setState(wss.SYNC);
    
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
