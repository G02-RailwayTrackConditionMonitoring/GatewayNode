#ifndef GATEWAY_BLE_H
#define GATEWAY_BLE_H

#include "ble_hal.h"
#include "Particle.h"
#include <vector>

#define SERVICE_UUID            "7abd7d09-dabd-4b5d-882d-7f4e5096f8f9"
#define CHARACTERISTIC_UUID     0xe9,0xa4,0x19,0x3d,0x4d,0x05,0x45,0xf9,0x8b,0xc2,0x91,0x15,0x78,0x6c,0x96,0xc2

class GatewayBLE{

    public:

        //Constructor
        GatewayBLE();

        //Turns the BT radio on.
        void startBLE();

        //Starts a BLE scan. Blocks for timeout (seconds?), then the callback is called with the results.
        void scanBLE(size_t timeout=5);

    private:
    
    BleUuid serviceUuid; 
    BleUuid characteristicUuid;

    BleCharacteristic dataCharcteristic; 

    std::vector<particle::BlePeerDevice> connectedNodes;  //Handle for the BLE connections.
    uint8_t numConnections;

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