#include "GatewayBLE.h"

GatewayBLE::GatewayBLE(){

    numConnections = 0;

    //Setup up the service and characteristic UUIDs.
    serviceUuid = BleUuid(SERVICE_UUID);

    const char dataUUID [16] = {CHARACTERISTIC_UUID};
    characteristicUuid = BleUuid((uint8_t *)dataUUID,BleUuidOrder::LSB); //LSB must be specified since that is how the ItsyBitsy specifies it.


    //Register all the callbacks.
    BLE.onDisconnected(GatewayBLE::disconnectCallback,this);
    //BLE.onConnected(GatewayBLE::connectedCallback,this);  //We are not using this yet.

    dataCharcteristic.onDataReceived(bleRxCallback,NULL);

}

void GatewayBLE::startBLE(){

    BLE.on();
}

int GatewayBLE::connectBLE(){

    int numDevicesFound = foundDevices.size();
    Log.info("Connecting to %d  BT devices ...",numDevicesFound);
    
    //Try and connect to each device.
    for(int i = 0; i < numDevicesFound; i++){

        particle::BlePeerDevice newConnection;
        newConnection = BLE.connect(foundDevices[i].addr,false); //Blocking Function.

        if(newConnection.connected()){
            
            Log.info("Connected to BT device: %s", foundDevices[i].name);
            

        }
    }
}

int GatewayBLE::scanBLE(size_t timeout){

    BLE.setScanTimeout(timeout);
    return BLE.scan(scanResultCallback, this);
}

void GatewayBLE::bleRxCallback(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context){}
void GatewayBLE::disconnectCallback(const BlePeerDevice& peer, void* context){}
void GatewayBLE::connectedCallback(const BlePeerDevice& peer, void* context){}

void GatewayBLE::scanResultCallback(const BleScanResult *scanResult, void *context){
    
    //Get our BLE object, since we set this pointer when we registered the callback.
    GatewayBLE * gatewayBLE = (GatewayBLE *) context;

    Log.info("MAC: %02X:%02X:%02X:%02X:%02X:%02X | RSSI: %dBm",
    scanResult->address[0], scanResult->address[1], scanResult->address[2],
    scanResult->address[3], scanResult->address[4], scanResult->address[5], scanResult->rssi);


    String name = scanResult->advertisingData.deviceName();
    if (name.length() > 0) {
        Log.info("deviceName: %s", name.c_str());
    }

    //Check the device name we just found against our approved list.
    for(int i=0; i< BLE_MAX_CONNECTION; i++){

        if(strcmp(name,gatewayBLE->approvedDevices[i]) ==0 ){

            bleConnection_t connection;
            connection.addr = scanResult->address;
            connection.name = name;

            gatewayBLE->foundDevices.push_back(connection);
        }
    }

    // if(strcmp(name,"G02_A") ==0 || strcmp(name,"G02_B") ==0 || strcmp(name, "G02_C")==0){
    //     //Save the address of the device with name G02.
    //     //And notify the main loop.
    //     btDeviceFound = true;
    //     btAddr = scanResult->address;

    // }
}
        