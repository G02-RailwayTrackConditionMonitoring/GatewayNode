#include "CommandHandler.h"

#include "GatewayCommands.h"
#include <Particle.h>






CommandHandler::CommandHandler(PublishQueueAsync *q){
  //GoogleMapsDeviceLocator locator;// = GoogleMapsDeviceLocator();
  
  publishQueue = q;
 
  
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
      //Log.info("uart: %c",c);
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
                                    "{\"fg\":\"%s\", \"tg\":\"%s\", \"st\":\"%s\", \"et\":\"%s\", \"xyz\":\"%s\"}",fromGPS, toGPS,startTime.c_str(),endTime.c_str(), data);       
                                Serial.println(pubData) ;
                                //snprintf(publishBuffer,PUBLISH_BUFFER_SIZE-1,"FORCE: %s\n",pubData);
                                Log.info("UART CMD: %s",publishBuffer);
                                Serial.printlnf("FROM GPS: %s", fromGPS);
                                Serial.printlnf("TO GPS: %s", toGPS);
                                


                                GPSState =1; 
                                

                                publishQueue->publish("data", pubData,PRIVATE);

                                //Also send to esp, so we can log to sd card...
                                char buf[255];
                                snprintf(buf,255,"%d: fg:%.13s tg:%.13s st:%s, et:%s, xyz:%s\n",AVG_FORCE_DATA,fromGPS,toGPS,startTime.c_str(),endTime.c_str(),data);
                                Serial1.printf(buf);

                                //We should handle if the send fails or were not connected...
                                Log.info("Published data");
                                //}
                                //else{
                                //    Log.warn("LTE not connected for publish, transmission skipped.");
                                //}
                                break;
                               

        case GATEWAY_BATTERY:   {

                                float batterySoc = System.batteryCharge();
                                
                                snprintf(publishBuffer,PUBLISH_BUFFER_SIZE-1,"bl:%.1f t:%s\n",batterySoc,Time.timeStr().c_str());
                                publishQueue->publish("telemetry",publishBuffer,PRIVATE);

                                char buf[255];
                                sprintf(buf,"%d: %.1f\n",GATEWAY_BATTERY,batterySoc);

                                Serial1.printf(buf);
                                break;
                                }

        case GATEWAY_BATTERY_REQ:{ float batterySoc = System.batteryCharge();
                                
                                char buf[255];
                                sprintf(buf,"%d: %.1f\n",GATEWAY_BATTERY,batterySoc);

                                Serial1.printf(buf);
                                break;
                                }

        case GATEWAY_FREE_SPACE:{

                                snprintf(publishBuffer,PUBLISH_BUFFER_SIZE-1,"fs:%s t:%s\n",data, Time.timeStr().c_str());
                                publishQueue->publish("telemetry",publishBuffer,PRIVATE);
                                break;

                                }

        case SET_GPS:           {//locator.publishLocation();

                                const char *scanData = locator.scan();
                                if(scanData[0]){
                                  publishQueue->publish("tcm-arm-device-locator",scanData,PRIVATE);//Do we want to queue these or just drop if no connection?
                                }
                                
                                //set fromTime
                                if(GPSState == 0){
                                  startTime = Time.timeStr();
                                  Log.info("start time: %s",startTime.c_str());
                        
                                }
                                else if(GPSState == 1){
                                  endTime = Time.timeStr();
                                  Log.info("end time: %s",startTime.c_str());
                                }

                                //set toTime
                                break;  
                                }
                                //case BATTERY_LEVEL(event = telemetry)
                                //case SD_STORAGE (event = telemetry)      

        default:                Log.warn("Invalid command received: %d",cmdNum);
                                break;

    }
    digitalWrite(D2,LOW);

}