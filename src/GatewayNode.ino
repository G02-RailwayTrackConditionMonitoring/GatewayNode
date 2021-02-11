#include "GatewayBLE.h"


//#include "spark_wiring_ble.h"
//The system mode changes how the cloud connection is managed.
//In Semi-Automatic mode, we must initiate the connection and then it is managed by the Particle firmware.
//This lets us run code before connecting.
SYSTEM_MODE(SEMI_AUTOMATIC);

//This sets the log level for logging over USB.
SerialLogHandler logHandler(LOG_LEVEL_ALL, {{"app", LOG_LEVEL_ALL}});

//Public Functions and vairables.
// int blink(String params); 
// int numButtonPress; 

uint8_t buffAIdx =0;
uint8_t buffBIdx =0;
uint8_t bufferA[756]; // Holds about 3 chunks of  42 samples.
uint8_t bufferB[756];

//Setup the input and output pins.
int buttonPin = D5;
int ledPin = D4; 

//Gloabl variables
bool runBlink = false;
GatewayBLE BleStack;

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


  #ifdef DEBUG
  delay(3000);
  Log.info("This is a debug build.");
  #endif

  Log.info("Starting application setup.");

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
  BleStack.startBLE();
}


void loop()
{
  // The core of your code will likely live here.
  if(digitalRead(buttonPin) == LOW){

      BleStack.scanBLE();
      BleStack.connectBLE();
  }

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

  
}

void processBuffer() {
    Serial.print("Received from Arduino: ");
    for (int i=0; i<sizeof(readBuf); i++){
      Serial.print(readBuf[i]);
    }
    Serial.print("\n");
}

void publishData(){
  String data = "";
  int accel = 5; //random
  data = String::format(
      "{\"station_a\":\"A\", \"station_b\":\"B\", \"accel\":%d}", accel);
  Log.trace("Publishing Data");
  Log.trace(data);
  Particle.publish("sampleData", data, PRIVATE);
}

