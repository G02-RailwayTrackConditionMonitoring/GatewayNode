#ifndef COMMAND_HANDLER_H_
#define COMMAND_HANDLER_H_

#include <string.h>
#include <stdint.h>
#include "Geolocator.h"
#include "PublishQueueAsyncRK.h"

#define PUBLISH_BUFFER_SIZE 1024*15-1
#define UART_BUFFER_SIZE    255


class CommandHandler{

    public:

        CommandHandler(PublishQueueAsync *q);
        char fromGPS[256]; 
        char toGPS[256]; 
        char yStdBuf[380]; 
        char xRmsBuf[380]; 
        char yRmsBuf[380]; 
        char zRmsBuf[380]; 
        int GPSState = 1;     
        String pubData = "";
        void getChar();
        void setGPS(const char *data);
        void locationCallback(float lat, float lon, float accuracy);
        GoogleMapsDeviceLocator locator;// = GoogleMapsDeviceLocator();
        int txStdY = 0;
        int txRmsX = 0;
        int txRmsY = 0;
        int txRmsZ = 0;
        
    private:

        PublishQueueAsync* publishQueue;
        char publishBuffer[PUBLISH_BUFFER_SIZE];

        String endTime ;
        String startTime;
        char readBuf[UART_BUFFER_SIZE];
        size_t readBufOffset = 0;

        void handleCommand(char* cmdString);

};

#endif