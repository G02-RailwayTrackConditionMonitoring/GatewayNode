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
char spiSendBuf[32] = "SPI transmission - dummy data";

const size_t READ_BUF_SIZE = 33;
char readBuf[READ_BUF_SIZE];
size_t readBufOffset = 0;
int64_t t = 0; 
void setup()
{
  #ifdef GCP
  if (!Particle.connected())
  {
    Particle.connect();
  }

  if (Particle.connected())
  {
    Log.info("Connected to the Particle Cloud.");
  }
  else
  {
    Log.error("Could not connect to the Particle Cloud.");
  }
  #endif
  Serial.println("Starting application setup.");
  Serial.begin(9600);
  waitFor(Serial.isConnected, 30000);
  //UART
  Serial1.begin(115200, SERIAL_DATA_BITS_8 | SERIAL_STOP_BITS_1 | SERIAL_PARITY_NO); 
  //SPI
  pinMode(MOSI, OUTPUT);
  pinMode(CS, OUTPUT);
  pinMode(SCK, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(HANDSHAKE, INPUT);  
  SPI.setClockSpeed(1000000);
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE3);
  SPI.begin(CS);  
  t = millis();
}

void loop()
{
  if (Serial1.available())
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
  if(digitalRead(HANDSHAKE) && (millis() -t >= 1000)){
    digitalWrite(CS, LOW);
    SPI.transfer(spiSendBuf, NULL, 32, NULL);
    digitalWrite(CS, HIGH);
    Serial.printlnf("send buffer, %s \n",spiSendBuf);
    //delay(1000);
    //Serial.printlnf("rcv buffer %s \n",r_buf);
    //delay(1000);
    t = millis();
  }
  
}

void processBuffer() {
    Serial.print("Received from Arduino: ");
    for (int i=0; i<sizeof(readBuf); i++){
      Serial.print(readBuf[i]);
    }
    Serial.print("\n");
}

