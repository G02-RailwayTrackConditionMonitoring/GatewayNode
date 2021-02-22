#ifndef GATEWAY_BLE_H
#define GATEWAY_BLE_H

#include "ble_hal.h"
#include "Particle.h"
#include <vector>
#include "CircularBuffer.h"

#define SERVICE_UUID            "7abd7d09-dabd-4b5d-882d-7f4e5096f8f9"
#define CHARACTERISTIC_UUID     0xe9,0xa4,0x19,0x3d,0x4d,0x05,0x45,0xf9,0x8b,0xc2,0x91,0x15,0x78,0x6c,0x96,0xc2
#define TELEMETRY_UUID          0xde,0xfa,0xf0,0x85,0xc8,0x8f,0x4f,0xa5,0xb9,0x8c,0x48,0x94,0xf6,0x71,0x09,0x99

#define COMMAND1_UUID           0xbb,0xe9,0x9d,0x88,0xf7,0xc3,0x4f,0xad,0x8a,0xba,0xd5,0x19,0x25,0x13,0x14,0xc3
#define COMMAND2_UUID           0x53,0xd2,0x50,0xbb,0x45,0xba,0x48,0xd7,0xba,0xba,0xc7,0x33,0x61,0x46,0xb5,0x88    

#define BLE_MAX_CONNECTION      2

#define BENCHMARK_START_FLAG    0xA5
#define BENCHMARK_END_FLAG      0x5A

#define BLE_RX_DATA_SIZE        244     //The number of bytes for each BLE data packet.
#define BLE_RX_BUFFER_COUNT     6       //Number of buffers for each node. Total memory used for buffers will be BLE_RX_BUFFER_COUNT*BLE_RX_DATA_SIZE*BLE_MAX_CONNECTION.

//This groups useful info about a connection together.
typedef struct{

    String name;
    particle::BleAddress addr;
    particle::BlePeerDevice conn;
    uint8_t idNum;

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

        //Returns the number of bytes of data in the rx buffer for the node with id "nodeID".
        int dataAvailable(uint8_t nodeId);
 

        //Copies data from the rx buffer of a node into the circular buffer specified. Make sure that the buffer items are large enough.
        //Calling this effectively removes data from the rxBufffer.
        //Returns the total number of bytes.
        uint16_t getData(CircularBuffer& buffer, uint8_t nodeId);

        uint8_t * getReadPtr(uint8_t nodeId);

        //Returns the device id (index) based on the name. G02_A is 0, G02_B is 1, etc.
        uint8_t getDeviceId(String name);

        std::vector<bleConnection_t> connectedNodes;  //Handle for the BLE connections.
        uint8_t numConnections;

    private:

        //The value of these is from the defined constants at the top of the file.
        BleUuid serviceUuid; 
        BleUuid dataStream_Uuid;
        BleUuid telemetryStream_Uuid;
        BleUuid commandStream_Uuids[2];

        //BLE Characteristics
        BleCharacteristic dataStream; 
        BleCharacteristic telemetryStream;
        BleCharacteristic commandStreams[2];

        //We will try and connect to devices with these names.
        const String approvedDevices[BLE_MAX_CONNECTION] = {"G02_A","G02_B"};

        //For scanning.
        std::vector<bleConnection_t> foundDevices; 

        //For receiveing data.
        std::vector<uint16_t> rxBufferWriteIdx;
        std::vector<uint16_t> rxBufferReadIdx;
        std::vector<CircularBuffer> rxBuffers;
        std::vector<bool>   rxBufferOverwrite;

        //For benchmarking.
        uint32_t rxCount=0;
        uint32_t startTime=0;
        uint32_t endTime=0;
        bool benchmarkDone[2] = {false,false}; //Holds wether a device has sent all its data.
        bool benchmarkInProgress = false;

        //Utility functions
        //Call from the rx Callback to run the benchmarking.
        void benchmark(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context);
        
        //Returns the index of the device with the connection handle "peer". This is with repspect to the connectedDevices array.
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