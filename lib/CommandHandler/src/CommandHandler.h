#ifndef COMMAND_HANDLER_H_
#define COMMAND_HANDLER_H_

#include <string.h>
#include <stdint.h>

#define PUBLISH_BUFFER_SIZE 255
#define UART_BUFFER_SIZE    255


class CommandHandler{

    public:

        CommandHandler();

        void getChar();

    private:

        char publishBuffer[PUBLISH_BUFFER_SIZE];

        
        char readBuf[UART_BUFFER_SIZE];
        size_t readBufOffset = 0;

        void handleCommand(char* cmdString);

};

#endif