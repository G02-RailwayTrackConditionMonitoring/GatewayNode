#include "Arduino.h"
#include "SPI.h"
SYSTEM_MODE(SEMI_AUTOMATIC);
SerialLogHandler logHandler(LOG_LEVEL_WARN, {{"app", LOG_LEVEL_TRACE}});

#define MOSI D12
#define MISO D11
#define CS D14
#define SCK D13
#define HANDSHAKE A4
char buf[6] = "hello";
const size_t READ_BUF_SIZE = 33;
char readBuf[READ_BUF_SIZE];
size_t readBufOffset = 0;
void setup()
{
  Serial.begin(9600);
  waitFor(Serial.isConnected, 30000);
  Serial1.begin(115200, SERIAL_DATA_BITS_8 | SERIAL_STOP_BITS_1 | SERIAL_PARITY_NO); 
  Serial.println("Starting application setup.");
}

void loop()
{
  while (Serial1.available())
  {
    if (readBufOffset < READ_BUF_SIZE)
    {
      char c = Serial1.read();
      if (c != '\n')
      {
        
        readBuf[readBufOffset++] = c;
        
      }
      else
      {
        readBuf[readBufOffset] = 0;
        processBuffer();
        readBufOffset = 0;
        Serial1.printlnf("%s", buf);
      }
    }
    else
    {
      Serial.println("readBuf overflow, emptying buffer");
      readBufOffset = 0;
    }
  }
}

void processBuffer() {
    Serial.print("Received from Arduino: ");
    for (int i=0; i<sizeof(readBuf); i++){
      Serial.print(readBuf[i]);
    }
    Serial.print("\n");
}