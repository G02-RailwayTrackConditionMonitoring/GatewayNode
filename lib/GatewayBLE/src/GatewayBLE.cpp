#include "GatewayBLE.h"

GatewayBLE::GatewayBLE(){

    numConnections = 0;

    //Setup up the service and characteristic UUIDs.
    serviceUuid = BleUuid(SERVICE_UUID);

    const char dataUUID [16] = {CHARACTERISTIC_UUID};
    characteristicUuid = BleUuid((uint8_t *)dataUUID,BleUuidOrder::LSB); //LSB must be specified since that is how the ItsyBitsy specifies it.


    //Register all the callbacks.
    BLE.onDisconnected(GatewayBLE::disconnectCallback,this);
    BLE.onConnected(GatewayBLE::connectedCallback,this);

    dataCharcteristic.onDataReceived(bleRxCallback,NULL);

}

void GatewayBLE::startBLE(){

    BLE.on();
}

void GatewayBLE::scanBLE(size_t timeout){

    BLE.setScanTimeout(timeout);
    BLE.scan(scanResultCallback, this);
}

void GatewayBLE::bleRxCallback(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context){}
void GatewayBLE::disconnectCallback(const BlePeerDevice& peer, void* context){}
void GatewayBLE::connectedCallback(const BlePeerDevice& peer, void* context){}
void GatewayBLE::scanResultCallback(const BleScanResult *scanResult, void *context){}
        