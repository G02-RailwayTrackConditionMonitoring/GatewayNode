#ifndef GATEWAY_BLE_H
#define GATEWAY_BLE_H

#include "ble_hal.h"
#include "Particle.h"
#include <vector>

#define SERVICE_UUID            "7abd7d09-dabd-4b5d-882d-7f4e5096f8f9"
#define CHARACTERISTIC_UUID     0xe9,0xa4,0x19,0x3d,0x4d,0x05,0x45,0xf9,0x8b,0xc2,0x91,0x15,0x78,0x6c,0x96,0xc2
#define BLE_MAX_CONNECTION      4

//This groups useful info about a connection together.
typedef struct{

    String name;
    particle::BleAddress addr;
    particle::BlePeerDevice conn;

} bleConnection_t;


class GatewayBLE{

    public:

        //Constructor
        GatewayBLE();

        //Turns the BT radio on.
        void startBLE();

        //Starts a BLE scan. Blocks for timeout (seconds?), then the callback is called with the results.
        //Returns the number of devices found.
        int scanBLE(size_t timeout=5);

        //This will connect to all devices with the correct name. Should be called after the scan function is called.
        //Returns the number of devices that were connected to.
        int connectBLE();


        std::vector<bleConnection_t> connectedNodes;  //Handle for the BLE connections.
        uint8_t numConnections;

    private:

        //The value of these is from the defined constants at the top of the file.
        BleUuid serviceUuid; 
        BleUuid characteristicUuid;

        //Used for getting data from the sensor nodes.
        BleCharacteristic dataCharcteristic; 

        //We will try and connect to devices with these names.
        const String approvedDevices[BLE_MAX_CONNECTION] = {"G02_A","G02_B","G02_C","G02_D"};

        //For scanning.
        std::vector<bleConnection_t> foundDevices;  

        //Callbacks 
        //These are called on the BLE stack, so don't do long operations (delay,etc) or use tons of memory.

            //Called when data is received.
            static void bleRxCallback(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context);

            //Called when a device connectes to the gateway.
            static void connectedCallback(const BlePeerDevice& peer, void* context);

            //Called when a device disconnects from the gateway.
            static void disconnectCallback(const BlePeerDevice& peer, void* context);

            //Called when the BLE scan results are in.
            static void scanResultCallback(const BleScanResult *scanResult, void *context);


};


#endif