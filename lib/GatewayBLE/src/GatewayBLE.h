#ifndef GATEWAY_BLE_H
#define GATEWAY_BLE_H

#include "ble_hal.h"
#include "Particle.h"
#include <vector>

#define SERVICE_UUID            "7abd7d09-dabd-4b5d-882d-7f4e5096f8f9"
#define CHARACTERISTIC_UUID     0xe9,0xa4,0x19,0x3d,0x4d,0x05,0x45,0xf9,0x8b,0xc2,0x91,0x15,0x78,0x6c,0x96,0xc2
#define COMMAND_UUID            0xbb,0xe9,0x9d,0x88,0xf7,0xc3,0x4f,0xad,0x8a,0xba,0xd5,0x19,0x25,0x13,0x14,0xc3
#define TELEMETRY_UUID          0xde,0xfa,0xf0,0x85,0xc8,0x8f,0x4f,0xa5,0xb9,0x8c,0x48,0x94,0xf6,0x71,0x09,0x99
#define BLE_MAX_CONNECTION      4

#define BENCHMARK_START_FLAG    0xA5
#define BENCHMARK_END_FLAG      0x5A

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

        //This will send a command to all of the sensor nodes. Parameters are a pointer to the data, and the size of the data in bytes.
        bool sendCommand(const void* command,size_t size);

        std::vector<bleConnection_t> connectedNodes;  //Handle for the BLE connections.
        uint8_t numConnections;

    private:

        //The value of these is from the defined constants at the top of the file.
        BleUuid serviceUuid; 
        BleUuid dataStream_Uuid;
        BleUuid telemetryStream_Uuid;
        BleUuid commandStream_Uuid;

        //BLE Characteristics
        BleCharacteristic dataStream; 
        BleCharacteristic telemetryStream;
        BleCharacteristic commandStream;

        //We will try and connect to devices with these names.
        const String approvedDevices[BLE_MAX_CONNECTION] = {"G02_A","G02_B","G02_C","G02_D"};

        //For scanning.
        std::vector<bleConnection_t> foundDevices;  

        //For benchmarking.
        uint32_t rxCount;
        uint32_t startTime=0;
        uint32_t endTime=0;
        bool benchmarkDone[2] = {false,false}; //Holds wether a device has sent all its data.
        bool benchmarkInProgress = false;

        //Utility functions
        
        //Returns the index of the device with the connection handle "peer".
        int8_t getDeviceIndex(const BlePeerDevice& peer);

        //Callbacks 
        //These are called on the BLE stack, so don't do long operations (delay,etc) or use tons of memory.

            //Called when data is received.
            static void bleRxDataCallback(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context);

            //Called when telemetry is received.
            static void bleRxTelemetryCallback(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context);

            //Called when a device connectes to the gateway.
            static void connectedCallback(const BlePeerDevice& peer, void* context);

            //Called when a device disconnects from the gateway.
            static void disconnectCallback(const BlePeerDevice& peer, void* context);

            //Called when the BLE scan results are in.
            static void scanResultCallback(const BleScanResult *scanResult, void *context);


};


#endif