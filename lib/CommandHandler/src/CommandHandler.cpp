#include "CommandHandler.h"

#include "GatewayCommands.h"
#include <Particle.h>

CommandHandler::CommandHandler(){

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

void CommandHandler::handleCommand(char* cmdString){

    char * cmd = strtok(cmdString,":");
    char* data = strtok(NULL,":");
    // call GeoLocator get GPS
    // get GPS triggers a cloud task 
    // 

    uint8_t cmdNum = atoi(cmd);
    Log.info("\n******************\n");
    Log.info("UART CMD: %s",data);
    Log.info("\n******************\n");

    digitalWrite(D2,HIGH);
    switch(cmdNum){

        case AVG_FORCE_DATA:    snprintf(publishBuffer,PUBLISH_BUFFER_SIZE-1,"FORCE: %s\n",data);
                                Log.info("UART CMD: %s",publishBuffer);
                                if(Particle.connected()){

                                bool sucess = Particle.publish("data", publishBuffer,PRIVATE);
                                //We should handle if the send fails or were not connected...
                                Log.info("Published data: %d",sucess);
                                }
                                else{
                                    Log.warn("LTE not connected for publish, transmission skipped.");
                                }
                                break;

        default:                Log.warn("Invalid command received: %d",cmdNum);
                                break;

    }
    digitalWrite(D2,LOW);

}