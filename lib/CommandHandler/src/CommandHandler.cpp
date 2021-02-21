#include "CommandHandler.h"

#include "GatewayCommands.h"
#include <Particle.h>

CommandHandler::CommandHandler(){

}

void CommandHandler::getChar(){

    char c = Serial1.read();

    //First byte of a command
    if(rxCount == 0){

        uartRxBuffer[rxCount] = c;
        rxCount++;
    }
    else if(rxCount == 1){
    //This is always the len byte.
        rxLen = c;
        uartRxBuffer[rxCount] = c;
        rxCount++;
    }
    else{
        uartRxBuffer[rxCount] = c;
        rxCount++;
    }

    //If count = len  then we have a full command.
    //If rxCount is 0 or 1 we don't have a valid length yet.
    if((rxCount >= rxLen) && rxCount>1){
        rxCount = 0;
        rxLen = 0;
        handleCommand();
    }
}

void CommandHandler::handleCommand(){

    GatewayUartPacket packet;

    GetPacket(uartRxBuffer,&packet);
    digitalWrite(D2,HIGH);
    switch(packet.command){

        case AVG_FORCE_DATA:    snprintf(publishBuffer,PUBLISH_BUFFER_SIZE-1,"FORCE: %d\n",packet.data.int16[0]);
                                Log.info("UART CMD: %s",publishBuffer);
                                if(Particle.connected()){

                                bool sucess = Particle.publish("data", publishBuffer,PRIVATE);
                                //We should handle if the send fails or were not connected...
                                Log.info("Published data: %d",sucess);
                                }
                                else{
                                    Log.warn("LTE not connected for publihs, transmission skipped.");
                                }
                                break;

        default:                Log.warn("Invalid command received: %d",packet.command);
                                break;

    }
    digitalWrite(D2,LOW);

}