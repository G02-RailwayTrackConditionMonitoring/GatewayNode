/*
 * Project GatewayNode
 * Description:
 * Author:
 * Date:
 */

#include "ble_hal.h"
#include <vector>

//#include "spark_wiring_ble.h"
//The system mode changes how the cloud connection is managed.
//In Semi-Automatic mode, we must initiate the connection and then it is managed by the Particle firmware.
//This lets us run code before connecting.
SYSTEM_MODE(SEMI_AUTOMATIC);

//This sets the log level for logging over USB.
SerialLogHandler logHandler(LOG_LEVEL_ALL, {{"app", LOG_LEVEL_ALL}});

//Public Functions and vairables.
int blink(String params); 
int numButtonPress; 
void scanResultCallback(const BleScanResult *scanResult, void *context);
void bleRxCallback(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context);
void disconnectCallback(const BlePeerDevice& peer, void* context);

//Setup the input and output pins.
int buttonPin = D5;
int ledPin = D4; 

//Gloabl variables
bool runBlink = false;

bool btDeviceFound = false; //Bluetooth device found.
particle::BleAddress btAddr;
const BleUuid serviceUuid("7abd7d09-dabd-4b5d-882d-7f4e5096f8f9"); 
const char dataUUID [16] = {0xe9,0xa4,0x19,0x3d,0x4d,0x05,0x45,0xf9,0x8b,0xc2,0x91,0x15,0x78,0x6c,0x96,0xc2};;
const BleUuid dataUuid((uint8_t *)dataUUID,BleUuidOrder::LSB); //LSB must be specified since that is how the ItsyBitsy specifies it.
BleCharacteristic dataCharcteristic;    
std::vector<particle::BlePeerDevice> connectedNodes;  //Handle for the BLE connections.
uint8_t numConnections = 0;

//For benchmarking.
uint32_t rxCount;
uint8_t rxData[2048];
uint32_t startTime=0;
uint32_t endTime=0;
bool benchmarkDone[2] = {false,false}; //Holds wether a device has sent all its data.
bool benchmarkInProgress = false;

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
  Particle.function("blink",blink);
  Particle.variable("buttonPress",numButtonPress);

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

    BLE.on();

    dataCharcteristic.onDataReceived(bleRxCallback,NULL);
    BLE.onDisconnected(disconnectCallback, NULL);
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.
  if(digitalRead(buttonPin) == LOW){

    
      Log.info("starting scan");
      int count = BLE.scan(scanResultCallback, NULL); //Blocking function.

      if (count > 0) {
          Log.info("%d devices found", count);
      }

      if(btDeviceFound){
        Log.info("Connecting to BT device.");

        //Try and connect.
        particle::BlePeerDevice newConnection;
        newConnection = BLE.connect(btAddr,false); //Blocking Function.


        if(newConnection.connected()){ 
          Log.info("Connected to BT device");

          btDeviceFound = false; // reset flag

          BleCharacteristic chars[10];
          ssize_t num = newConnection.discoverAllCharacteristics(chars, 10);
          
          for(int i =0; i<min(num,10);i++){

            BleUuid uuid = chars[i].UUID();
            Log.info("UUID: %s\n",uuid.toString().c_str());
          }
        
          bool result = newConnection.getCharacteristicByUUID(dataCharcteristic,dataUuid);
          if(result){
            Log.info("Data charactreristic found!");
            dataCharcteristic.subscribe(true);
            
            int result =  hal_ble_gatt_set_att_mtu(247, NULL);
            Log.info("Change ATT MTU: %d", result);
            hal_ble_conn_info_t conn_info;
            result = hal_ble_gap_get_connection_info(0, &conn_info, NULL);
            Log.info("get connection infor(0 for success) : %d",result);
            Log.info("Con att mtu: %d",conn_info.att_mtu);
            Log.info("Con role: %d",conn_info.role);
            Log.info("Con version: %d",conn_info.version);

            //Keep reference to the connection.  
            connectedNodes.push_back(newConnection);
            numConnections++;
          }

        }
      }
    


  }
  if(runBlink){

    digitalWrite(ledPin,!digitalRead(ledPin));
  
  }


  delay(1000);
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

void scanResultCallback(const BleScanResult *scanResult, void *context) {

    Log.info("MAC: %02X:%02X:%02X:%02X:%02X:%02X | RSSI: %dBm",
            scanResult->address[0], scanResult->address[1], scanResult->address[2],
            scanResult->address[3], scanResult->address[4], scanResult->address[5], scanResult->rssi);

    
    String name = scanResult->advertisingData.deviceName();
    if (name.length() > 0) {
        Log.info("deviceName: %s", name.c_str());
    }

    if(strcmp(name,"G02_A") ==0 || strcmp(name,"G02_B") ==0 || strcmp(name, "G02_C")==0){
      //Save the address of the device with name G02.
      //And notify the main loop.
      btDeviceFound = true;
      btAddr = scanResult->address;
      
    }
}

void disconnectCallback(const BlePeerDevice& peer, void* context){

  //Should try and reconnect. For now just drop the connection.
  for(int i=0; i<numConnections; i++){
      if(connectedNodes[i] == peer){
        Log.info("device %d(%s) disconnected.",i,connectedNodes[i]);
        connectedNodes.erase(connectedNodes.begin()+i);
        numConnections--;
      }
  }

}
void bleRxCallback(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context) {
      
        //Log.info("%p",&peer);
        rxCount += len;  //At this point we only count the number of bytes received.

        if(data[0] == 0xA5 && !benchmarkInProgress){
          Log.info("Startred benchmark");
          startTime = millis();
          benchmarkInProgress = true;
        }
        else if(data[0] == 0x5A){


          int node=0xFFFF;
          for(int i=0; i<numConnections; i++){
            if(connectedNodes[i] == peer){
                node = i;
             }
          }
          benchmarkDone[node] = true;

          bool done = true;
          for(int i=0; i<numConnections; i++){
              done &= benchmarkDone[i];
          }

          if(done){
            endTime = millis();
            uint32_t totalTime = (endTime-startTime);

            Log.info("start: %lu, end: %lu.",startTime,endTime);
            Log.info("Received %d bytes in %lu ms from conenction %d. %f bytes/second.",rxCount, totalTime ,node,rxCount/(totalTime/1000.0)); 
            
            rxCount = 0; //Reset the rx count.

            //reset the benchmark state.
            for(int i=0; i<numConnections; i++){
                benchmarkDone[i] = false;
            }
            benchmarkInProgress = false;

          }
        }
       
        digitalWriteFast(ledPin,!digitalRead(ledPin));

}