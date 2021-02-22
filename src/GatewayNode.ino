#include "GatewayBLE.h"
#include "CircularBuffer.h"
#include "CommandHandler.h"
#include "GatewayCommands.h"

SYSTEM_THREAD(ENABLED);

//#include "spark_wiring_ble.h"
//The system mode changes how the cloud connection is managed.
//In Semi-Automatic mode, we must initiate the connection and then it is managed by the Particle firmware.
//This lets us run code before connecting.
SYSTEM_MODE(SEMI_AUTOMATIC);

//This sets the log level for logging over USB.
SerialLogHandler logHandler(LOG_LEVEL_ALL, {{"app", LOG_LEVEL_ALL}});


// #define GCP

 uint8_t spi_buff[241]; //Hold 40 samples +1 byte id.

CommandHandler cmdHandler = CommandHandler();

//Setup the input and output pins.
int buttonPin = D5;
int ledPin = D4; 

//Gloabl variables
bool runBlink = false;
GatewayBLE BleStack;
#define NUM_BLE_NODES 1

#define MOSI D12
#define MISO D11
#define CS D14
#define SCK D13
#define HANDSHAKE A4
char buf[6] = "hello";
char spiSendBuf[32] = "SPI transmission - dummy data";

const size_t READ_BUF_SIZE = 255;
char readBuf[READ_BUF_SIZE];
size_t readBufOffset = 0;
int64_t t = 0; 

volatile bool spiBusy = false;
void spiDoneHandler();
void connectLTE();

void setup()
{


  #ifdef DEBUG
  delay(3000);
  Log.info("This is a debug build.");
  #endif

  Log.info("Starting application setup.");


      

  Serial.println("Starting application setup.");
  Serial.begin(9600);
  waitFor(Serial.isConnected, 30000);

  pinMode(buttonPin,INPUT_PULLUP);
  pinMode(D7,OUTPUT);
  pinMode(D6,OUTPUT);
  pinMode(D2,OUTPUT);
  pinMode(D3,OUTPUT);

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
    while(!Particle.connected()){}
 
  #endif

  
  //Wait until were connected then log some info.
  if(Time.isValid()){
          Log.info("Sending uart command to update time.");
          GatewayUartPacket packet;
          packet.command = TIME_UPDATE;
          packet.len = 5;

          packet.data.uint8[0] =Time.month(); 
          packet.data.uint8[1] =Time.day();
          packet.data.uint8[2] =Time.hour();
          packet.data.uint8[3] =Time.minute();
          packet.data.uint8[4] = Time.second();
          
          uint8_t uartBuf[7];
          uint8_t bytesToSend=0;

          bytesToSend = PreparePacket(uartBuf,&packet);
          Serial1.write(uartBuf,bytesToSend);

        }

        GatewayUartPacket packet;
        packet.command = LTE_RSSI_DATA;
        packet.len = sizeof(float);

        CellularSignal sig = Cellular.RSSI();
        float quality = sig.getQuality(); //Percentage 0 to 100.
        
        packet.data.float32[0] = quality;
        uint8_t uartBuf[packet.len+2];
        uint8_t bytesToSend=0;

        bytesToSend = PreparePacket(uartBuf,&packet);
        Serial1.write(uartBuf,bytesToSend);

  BleStack.startBLE();

  
  //Auto connect
  while(BleStack.numConnections<NUM_BLE_NODES){
      
      BleStack.scanBLE();
      uint8_t numCon = BleStack.connectBLE();
      delay(200);
      for(int i=0; i<numCon;i++){
        GatewayUartPacket packet;
        packet.command = BLE_CONNECTION_EVENT;
        packet.len = 3;

        packet.data.uint8[0] =1; //This is a connection(1), not a disconnection(0).
        packet.data.uint8[1] =i;
        
        uint8_t uartBuf[4];
        uint8_t bytesToSend=0;

        bytesToSend = PreparePacket(uartBuf,&packet);
        Serial1.write(uartBuf,bytesToSend);
      }
  }
}


void loop()
{
  // The core of your code will likely live here.
  // if(digitalRead(buttonPin) == LOW){

  //     BleStack.scanBLE();
  //     BleStack.connectBLE();
  // }

  //Try and reconnect BLE if lost.
  if(BleStack.numConnections<NUM_BLE_NODES){
    BleStack.scanBLE();
    BleStack.connectBLE();
  }


  //Handle uart commands
  if (Serial1.available())
  {
    cmdHandler.getChar();
  }

  //Handle BLE data and send over SPI.
  uint8_t packets= 0;
  if( (packets =BleStack.dataAvailable(0)) && (digitalRead(HANDSHAKE)) && (!spiBusy)){

    digitalWrite(D6,HIGH); //For debugging
    
    Log.info("ready to send 1 of %d over SPI for node A.",packets);
    
    uint8_t* location = BleStack.getReadPtr(0);
    memcpy(spi_buff,location,BLE_RX_DATA_SIZE);
    
    spi_buff[240] = 0;//Last byte indicate this data is from node 0.

    spiBusy = true;
    digitalWrite(CS, LOW);
    SPI.transfer(spi_buff,NULL,241,spiDoneHandler);//This version of spi.transfer uses dma.
    digitalWrite(D6,LOW);
  }


 //It can take around 20 us for handshake  to go low after spi transaction, 
  //so we need delay to wait at least that long otherwise we falsely read the handshake is high and send when the esp is not read.

  delayMicroseconds(100);
}

// void processBuffer() {

//   String data = "";
//   int accel = 5; //random
//   bool result = false;
//   data = String::format(
//       "{\"station_a\":\"A\", \"station_b\":\"B\", \"accel\":%d}", readBuf[0]);
//   if (Particle.connected()) {
//     Log.trace("Publishing Data");
//     Log.trace(data);
//     result = Particle.publish("sampleData", data, PRIVATE);
    
//   }
//   Log.trace("Publish successfull: %d",result);
//     // Serial.print("Received from Arduino: ");
//     // for (int i=0; i<sizeof(readBuf); i++){
//     //   Serial.print(readBuf[i]);
//     // }
//     // Serial.print("\n");
// }

// void publishData(){
//   String data = "";
//   int accel = 5; //random
//   data = String::format(
//       "{\"station_a\":\"A\", \"station_b\":\"B\", \"accel\":%d}", accel);
//   Log.trace("Publishing Data");
//   Log.trace(data);
//   Particle.publish("sampleData", data, PRIVATE);
// }

void spiDoneHandler(){
  digitalWrite(CS,HIGH);
  spiBusy = false;
}
