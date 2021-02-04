#include "Arduino.h"
#include "SPI.h"
SYSTEM_MODE(SEMI_AUTOMATIC);
SerialLogHandler logHandler(LOG_LEVEL_WARN, {{"app", LOG_LEVEL_TRACE}});

#define MOSI D12
#define MISO D11
#define CS D14
#define SCK D13
#define HANDSHAKE A4

char buf[32] = "testing data trans";
char r_buf[32];

void setup()
{
  Serial.begin(9600);
  pinMode(MOSI, OUTPUT);
  pinMode(CS, OUTPUT);
  pinMode(SCK, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(HANDSHAKE, INPUT);  
  Log.info("Starting application setup.");
  SPI.setClockSpeed(1000000);
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE3);
  SPI.begin(CS);
  }

void loop()
{
  while(!digitalRead(HANDSHAKE)){}
  digitalWrite(CS, LOW);
  SPI.transfer(buf, r_buf, 32, NULL);
  digitalWrite(CS, HIGH);
  Serial.printlnf("send buffer, %s \n",buf);
  delay(200);
  Serial.printlnf("rcv buffer %s \n",r_buf);
  delay(1000);
}
