/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "e:/Files/University/Capstone/GatewayNode/src/GatewayNode.ino"
/*
 * Project GatewayNode
 * Description:
 * Author:
 * Date:
 */


//The system mode changes how the cloud connection is managed.
//In Semi-Automatic mode, we must initiate the connection and then it is managed by the Particle firmware.
//This lets us run code before connecting.
void setup();
void loop();
#line 12 "e:/Files/University/Capstone/GatewayNode/src/GatewayNode.ino"
SYSTEM_MODE(SEMI_AUTOMATIC);

//This sets the log level for logging over USB.
SerialLogHandler logHandler(LOG_LEVEL_WARN, {{"app", LOG_LEVEL_TRACE}});

//Public Functions and vairables.
int blink(String params); 
uint8_t numButtonPress; 

//Setup the input and output pins.
int modeButton = BTN;
int ledPin = RGBR; //Red pin for RGB led.

//Gloabl variables
bool runBlink = false;

// setup() runs once, when the device is first turned on.
void setup() {

  Log.info("Starting application setup.");

  // Put initialization like pinMode and begin functions here.
  pinMode(modeButton,INPUT); 
  pinMode(ledPin, OUTPUT);

  //Register functions and variables with Particle cloud. Doing this before connecting will save cell data.
  Particle.function("blink",blink);
  Particle.variable("buttonPress",numButtonPress);

  //check the connectivity. The cell connection should be setup automatically before the applicaiton code runs.
  
  
  Particle.connect();
  
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.
  if(digitalRead(modeButton)) numButtonPress++;

  if(runBlink){


  }
}

//This function will start or stop blinking the onboard LED.
int blink(String params){

  if(params == "Start"){

    runBlink = true ;
    return 1;
  }
  else if(params == "Stop"){

    runBlink = false;
    return 1;
  }
  else return -1;

}
