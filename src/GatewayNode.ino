/*
 * Project GatewayNode
 * Description:
 * Author:
 * Date:
 */

#include "ble_hal.h"
#include <vector>
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


// setup() runs once, when the device is first turned on.
void setup() {

  #ifdef DEBUG
  delay(3000);
  Log.info("This is a debug build.");
  #endif

  Log.info("Starting application setup.");

  // Put initialization like pinMode and begin functions here.
  pinMode(buttonPin,INPUT_PULLUP); 
  pinMode(ledPin, OUTPUT);

  //Register functions and variables with Particle cloud. Doing this before connecting will save cell data.
  // Particle.function("blink",blink);
  // Particle.variable("buttonPress",numButtonPress);
  
  //Connect to the cloud. This should automatically connect cellular.
  if(!Particle.connected()){
    //Particle.connect(); //Only connect if were not already connected.
  }

  if(Particle.connected()){
    Log.info("Connected to the Particle Cloud.");
  }
  else{
    Log.error("Could not connect to the Particle Cloud.");
  }

      
  BleStack.startBLE();
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.
  if(digitalRead(buttonPin) == LOW){

      BleStack.scanBLE();
      BleStack.connectBLE();
  }

  // //Check if we received data, and copy to bigger buffer for SPI transmision.
  // uint8_t numBytesA = BleStack.dataAvailable(0);
  // uint8_t numBytesB = BleStack.dataAvailable(1);
  // if(numBytesA>0){
  //   Log.info("%d bytes from A",numBytesA);
  //   bool dataGood = BleStack.getData(&bufferA[buffAIdx],0);
  //   buffAIdx += numBytesA;

  //   if(!dataGood) Log.info("Overwrite data A!");

  // }
  // if(numBytesB){
  //   Log.info("%d bytes from B",numBytesA);
  //   bool dataGood = BleStack.getData(&bufferB[buffBIdx],1);
  //   buffBIdx += numBytesB;

  //   if(!dataGood)  Log.info("Overwrite dataB!");
  // }

  // //send the data over spi.
  // if(buffAIdx > 700){

  //   //SPI.send(data)
  //   buffAIdx = 0;
  // }

  //   if(buffBIdx > 700){

  //   //SPI.send(data)
  //   buffBIdx = 0;
  // }


  delay(1);
}

