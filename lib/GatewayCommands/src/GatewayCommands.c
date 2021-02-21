#include "GatewayCommands.h"
#include <string.h>

uint8_t PreparePacket(uint8_t* buffer, GatewayUartPacket* packet){

    uint8_t totalLen = packet->len+2;

    buffer[0] = packet->command;
    buffer[1] =totalLen;
    
    memcpy(&buffer[2],&(packet->data),packet->len);

    return totalLen;
}

uint8_t GetPacket(uint8_t* buffer,GatewayUartPacket* packet){


    packet->command = (GatewayCommand_t)buffer[0];

    if(packet->command>NUM_GATEWAY_COMMANDS) return 1;  //Invalid command byte.

    packet->len = buffer[1]-2;  //Subract 2, for the command and len fields.
    
    memcpy(&packet->data,&buffer[2],packet->len);

    return 0;
}