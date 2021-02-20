#include "GatewayBLE.h"
#include "CircularBuffer.h"

SYSTEM_THREAD(ENABLED);

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

// #define GCP

// uint8_t buffAIdx =0;
// uint8_t buffBIdx =0;
// uint8_t bufferA[756]; // Holds about 3 chunks of  42 samples.
 uint8_t spi_buff[241]; //Hold 40 samples +1 byte id.
// //uint8_t bufferB[756];
// uint8_t spiBacklog = 0;

// CircularBuffer nodeABuff = CircularBuffer(240,4);

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

volatile bool spiBusy = false;
void spiDoneHandler();

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

  pinMode(buttonPin,INPUT_PULLUP);
  pinMode(D7,OUTPUT);
  pinMode(D6,OUTPUT);

  //UART
  Serial1.begin(115200, SERIAL_DATA_BITS_8 | SERIAL_STOP_BITS_1 | SERIAL_PARITY_NO); 
  //SPI
  pinMode(MOSI, OUTPUT);
  pinMode(CS, OUTPUT);
  pinMode(SCK, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(HANDSHAKE, INPUT);  
  SPI.setClockSpeed(8000000);
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

  
  //Check if we have BLE data to handle.
  // if(BleStack.dataAvailable(0)){
  //   BleStack.getData(nodeABuff,0);
  //   Log.info("data from A");
  //   nodeABuff.printDebugInfo(false);
  //   //Just copy data to local buff, so the rx buffs are clear.
  // }
  // if(BleStack.dataAvailable(1)){
  //   BleStack.getData(nodeBBuff,1);
  //  //Just copy data to local buff, so the rx buffs are clear.
  // }

  //Now check if we have data to send over spi. This is seperate to try and spread out the handling over time.

  uint8_t packets= 0;
  if( (packets =BleStack.dataAvailable(0)) && (digitalRead(HANDSHAKE)) && (!spiBusy)){
    digitalWrite(D6,HIGH);
    Log.info("ready to send 1 of %d over SPI for node A.",packets);
    uint8_t* location = BleStack.getReadPtr(0);
    memcpy(spi_buff,location,BLE_RX_DATA_SIZE);
    spi_buff[240] = 0;//Last byte indicate this data is from node 0.
    spiBusy = true;
    digitalWrite(CS, LOW);
    SPI.transfer(spi_buff,NULL,241,spiDoneHandler);//This version of spi.transfer uses dma.
    digitalWrite(D6,LOW);
  }

  // if(nodeBBuff.getCurrNumItems() && digitalRead(HANDSHAKE) && !spiBusy){
  //   uint8_t* location = nodeBBuff.getReadPtr();
  //   memcpy(spi_buff,location,nodeBBuff.getItemSize());
  //   spi_buff[240] = 1;//Last byte indicate this data is from node 1.
  //   spiBusy = true;
  //   digitalWrite(CS, LOW);
  //   SPI.transfer(spi_buff,NULL,241,spiDoneHandler);
      
  // }

 //It can take around 20 us for handshake  to go low after spi transaction, 
  //so we need delay to wait at least that long otherwise we falsely read the handshake is high and send when the esp is not read.

  delayMicroseconds(100);
}

void processBuffer() {

  String data = "";
  int accel = 5; //random
  bool result = false;
  data = String::format(
      "{\"station_a\":\"A\", \"station_b\":\"B\", \"accel\":%d}", readBuf[0]);
  if (Particle.connected()) {
    Log.trace("Publishing Data");
    Log.trace(data);
    result = Particle.publish("sampleData", data, PRIVATE);
    
  }
  Log.trace("Publish successfull: %d",result);
    // Serial.print("Received from Arduino: ");
    // for (int i=0; i<sizeof(readBuf); i++){
    //   Serial.print(readBuf[i]);
    // }
    // Serial.print("\n");
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

void spiDoneHandler(){
  digitalWrite(CS,HIGH);
  spiBusy = false;
}
