#include "CommandHandler.h"

#include "GatewayCommands.h"
#include <Particle.h>


CommandHandler::CommandHandler(){
  GoogleMapsDeviceLocator locator;// = GoogleMapsDeviceLocator();
  char fromGPS[256]; 
  char toGPS[256]; 
  int GPSState = 1; 
  //locator.withSubscribe(locationCallback);
  
}

void str_cpy(char *dst, const char *src) {
   while (*src != '\0') {
      *dst++ = *src++; 
   }
   *dst = '\0';
}

void CommandHandler::getChar(){

     if (readBufOffset < UART_BUFFER_SIZE)
    {
      char c = Serial1.read();
      if (c != '\n')
      {
        
        readBuf[readBufOffset++] = c;
        
      }
      else
      {
        readBuf[readBufOffset] = 0;
        handleCommand(readBuf);
        readBufOffset = 0;
        //Serial1.printlnf("%s", buf);
      }
    }
    else
    {
      Serial.println("readBuf overflow, emptying buffer");
      readBufOffset = 0;
    }
  
    
}

void CommandHandler::setGPS(const char* data){
  Serial.println("SET GPS");
  Serial.printlnf("%s", data);
  if(GPSState){
    //set fromGPS
    str_cpy(fromGPS, data);
    Serial.printlnf("FROM GPS: %s", fromGPS);
    GPSState =0 ; 
  }else{
    //set toGPS
    str_cpy(toGPS, data);
    Serial.printlnf("TO GPS: %s", toGPS);    
    
    
  }
}


void CommandHandler::handleCommand(char* cmdString){

    char * cmd = strtok(cmdString,":");
    char* data = strtok(NULL,":");
    // call GeoLocator get GPS
    // get GPS triggers a cloud task 
    // 
    //locator.publishLocation();
    //Particle.publish("device-locator", data, PRIVATE);        

      

    

    uint8_t cmdNum = atoi(cmd);
    Log.info("\n******************\n");
    Log.info("UART CMD: %s",data);
    Log.info("\n******************\n");

    digitalWrite(D2,HIGH);
    switch(cmdNum){

        case AVG_FORCE_DATA:    
                                pubData = String::format(
                                    "{\"fg\":\"%s\", \"tg\":\"%s\", \"xyz\":\"%s\"}",fromGPS, toGPS, data);       
                                Serial.println(pubData) ;
                                //snprintf(publishBuffer,PUBLISH_BUFFER_SIZE-1,"FORCE: %s\n",pubData);
                                Log.info("UART CMD: %s",publishBuffer);
                                Serial.printlnf("FROM GPS: %s", fromGPS);
                                Serial.printlnf("TO GPS: %s", toGPS);



                                GPSState =1; 
                                if(Particle.connected()){

                                bool sucess = Particle.publish("data", pubData,PRIVATE);
                                //We should handle if the send fails or were not connected...
                                Log.info("Published data: %d",sucess);
                                }
                                else{
                                    Log.warn("LTE not connected for publish, transmission skipped.");
                                }
                                break;
        case SET_GPS:           locator.publishLocation();
                                // @joseph.. how ? 
                                //set fromTime
                                //set toTime
                                break;  
                                //case BATTERY_LEVEL(event = telemetry)
                                //case SD_STORAGE (event = telemetry)                                                              

        default:                Log.warn("Invalid command received: %d",cmdNum);
                                break;

    }
    digitalWrite(D2,LOW);

}