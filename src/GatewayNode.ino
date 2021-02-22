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

          char buf[255];
          sprintf(buf,"%d: %d/%d/ %dh %dm %ds\n",TIME_UPDATE,Time.month(),Time.day(),Time.hour(),Time.minute(),Time.second());

          Serial1.printf(buf);
        }

  CellularSignal sig = Cellular.RSSI();
  float quality = sig.getQuality(); //Percentage 0 to 100.
    
  char buf[255];
  sprintf(buf,"%d:%3.1f\n",LTE_RSSI_DATA,quality);     
  Serial1.printf(buf);

  BleStack.startBLE();

  
  //Auto connect
  while(BleStack.numConnections<NUM_BLE_NODES){
      
      BleStack.scanBLE();
      uint8_t numCon = BleStack.connectBLE();
      delay(200);
      char buf[255];
      for(int i=0; i<numCon;i++){
        
        sprintf(buf,"%d: %d,%d\n",BLE_CONNECTION_EVENT,1,i);
        Serial1.printf(buf);
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
   uint8_t numCon = BleStack.connectBLE();
    for(int i=0; i<numCon;i++){
      char buf[255];
       sprintf(buf,"%d: %d,%d\n",BLE_CONNECTION_EVENT,1,i);
        Serial1.printf(buf);
  }
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
