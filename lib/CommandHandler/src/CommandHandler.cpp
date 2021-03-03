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


        case AVG_FORCE_DATA:{    
                                pubData = String::format(
                                    "{\"fg\":\"%s\", \"tg\":\"%s\", \"st\":\"%s\", \"et\":\"%s\", \"xyz\":\"%s\"}",fromGPS, toGPS,startTime.c_str(),endTime.c_str(), data);       
                                Serial.println(pubData) ;
                                //snprintf(publishBuffer,PUBLISH_BUFFER_SIZE-1,"FORCE: %s\n",pubData);
                                Log.info("UART CMD: %s",publishBuffer);
                                Serial.printlnf("FROM GPS: %s", fromGPS);
                                Serial.printlnf("TO GPS: %s", toGPS);
                                


                                GPSState =1; 
                                

                                publishQueue->publish("data", pubData,PRIVATE);

                                char* x = strtok(fromGPS,",");
                                char* y = strtok(NULL,",");
                                char* x2 = strtok(toGPS,",");
                                char* y2 = strtok(NULL,",");
                                //Also send to esp, so we can log to sd card...
                                char buf[500];
                                snprintf(buf,499,"%d: %.5s,%.5s  %.5s,%.5s %s, %s, %s\n",AVG_FORCE_DATA,x,y,x2,y2,startTime.c_str(),endTime.c_str(),data);
                                Serial1.printf(buf);

                                //We should handle if the send fails or were not connected...
                                Log.info("Published data");
                                //}
                                //else{
                                //    Log.warn("LTE not connected for publish, transmission skipped.");
                                //}
                                break;
        }

        case GATEWAY_BATTERY:   {

                                float batterySoc = System.batteryCharge();
                                
                                snprintf(publishBuffer,PUBLISH_BUFFER_SIZE-1,"bl:%.1f,t:%s\n",batterySoc,Time.timeStr().c_str());
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

                                snprintf(publishBuffer,PUBLISH_BUFFER_SIZE-1,"fs:%s,t:%s\n",data, Time.timeStr().c_str());
                                publishQueue->publish("telemetry",publishBuffer,PRIVATE);

                                if(Particle.connected()){
                                  CellularSignal sig = Cellular.RSSI();
                                  float quality = sig.getQuality(); //Percentage 0 to 100.
                                    
                                  char buf[255];
                                  sprintf(buf,"%d:%3.1f\n",LTE_RSSI_DATA,quality);     
                                  Serial1.printf(buf);
                                }
                                break;

                                }

        case SET_GPS:           {//locator.publishLocation();

                                const char *scanData = locator.scan();
                                if(scanData[0]){
                                  publishQueue->publish("tcm-arm-device-locator",scanData,PRIVATE);//Do we want to queue these or just drop if no connection?
                                }
                                
                                // //set fromTime
                                // if(GPSState == 0){
                                //   startTime = Time.timeStr();
                                //   Log.info("start time: %s",startTime.c_str());
                        
                                // }
                                // else if(GPSState == 1){
                                //   endTime = Time.timeStr();
                                //   Log.info("end time: %s",startTime.c_str());
                                // }

                                //set toTime
                                break;  
                                }
        case SET_START_TIME:   {
                                  startTime = Time.timeStr();
                                  Log.info("start time: %s",startTime.c_str());
                                  break;
                               }
        case SET_END_TIME:   {
                                  endTime = Time.timeStr();
                                  Log.info("end time: %s",endTime.c_str());
                                  break;
                               }            
        case TX_STD_Y:        {
                                
                                if(strlen(yStdBuf)>=360){
                                  if(txStdY ==1){
                                    Log.info("STD Y Data %s",yStdBuf);
                                    snprintf(publishBuffer,PUBLISH_BUFFER_SIZE-1,"yStd:%s,t:%s\n",yStdBuf, Time.timeStr().c_str());
                                    publishQueue->publish("telemetry",publishBuffer,PRIVATE);
                                  }
                                  memset(yStdBuf, 0, 380);
                                }
                                sprintf(yStdBuf+strlen(yStdBuf),"%s",data);
                                break;
                              }      
        case TX_RMS_X:        {
                                if(strlen(xRmsBuf)>=360){
                                  if(txRmsX ==1){
                                    Log.info("RMS X Data %s",xRmsBuf);
                                    snprintf(publishBuffer,PUBLISH_BUFFER_SIZE-1,"xRms:%s,t:%s\n",xRmsBuf, Time.timeStr().c_str());
                                    publishQueue->publish("telemetry",publishBuffer,PRIVATE);
                                  }
                                  memset(xRmsBuf, 0, 380);
                                }
                                sprintf(xRmsBuf+strlen(xRmsBuf),"%s",data);
                                break;
                              }                                                             
        case TX_RMS_Y:        {
                                if(strlen(yRmsBuf)>=360){
                                  if(txRmsY ==1){
                                    Log.info("RMS Y Data %s",yRmsBuf);
                                    snprintf(publishBuffer,PUBLISH_BUFFER_SIZE-1,"yRms:%s,t:%s\n",yRmsBuf, Time.timeStr().c_str());
                                    publishQueue->publish("telemetry",publishBuffer,PRIVATE);
                                  }
                                  memset(yRmsBuf, 0, 380);
                                }
                                sprintf(yRmsBuf+strlen(yRmsBuf),"%s",data);
                                break;
                              }                                                       
        case TX_RMS_Z:        {
                                if(strlen(zRmsBuf)>=360){
                                  if(txRmsZ ==1){
                                    Log.info("RMS Z Data %s",zRmsBuf);
                                    snprintf(publishBuffer,PUBLISH_BUFFER_SIZE-1,"zRms:%s,t:%s\n",zRmsBuf, Time.timeStr().c_str());
                                    publishQueue->publish("telemetry",publishBuffer,PRIVATE);
                                  }
                                  memset(zRmsBuf, 0, 380);
                                }
                                sprintf(zRmsBuf+strlen(zRmsBuf),"%s",data);
                                break;
                              }       

        default:                Log.warn("Invalid command received: %d",cmdNum);
                                break;

    }
    digitalWrite(D2,LOW);

}