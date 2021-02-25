#ifndef COMMAND_HANDLER_H_
#define COMMAND_HANDLER_H_

#include <string.h>
#include <stdint.h>
#include "Geolocator.h"

#define PUBLISH_BUFFER_SIZE 1023
#define UART_BUFFER_SIZE    255


class CommandHandler{

    public:

        CommandHandler();
        char fromGPS[256]; 
        char toGPS[256]; 
        int GPSState = 1;     
        String pubData = "";
        void getChar();
        void setGPS(const char *data);
        void locationCallback(float lat, float lon, float accuracy);
        GoogleMapsDeviceLocator locator;// = GoogleMapsDeviceLocator();
        
    private:

        char publishBuffer[PUBLISH_BUFFER_SIZE];

        
        char readBuf[UART_BUFFER_SIZE];
        size_t readBufOffset = 0;

        void handleCommand(char* cmdString);

};

#endif