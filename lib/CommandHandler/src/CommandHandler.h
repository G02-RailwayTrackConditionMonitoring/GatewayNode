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
        uint8_t uartRxBuffer[UART_BUFFER_SIZE];
        uint8_t uartTxBuffer[UART_BUFFER_SIZE];

        uint16_t rxCount=0;
        uint16_t rxLen=0;

        void handleCommand();

};

#endif