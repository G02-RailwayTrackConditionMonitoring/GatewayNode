#include  <GatewayBLE.h>

GatewayBLE::GatewayBLE(){

    numConnections = 0;

    //Setup up the service and characteristic UUIDs.
    serviceUuid = BleUuid(SERVICE_UUID);

    const char dataUUID [16] = {CHARACTERISTIC_UUID};
    dataStream_Uuid = BleUuid((uint8_t *)dataUUID,BleUuidOrder::LSB); //LSB must be specified since that is how the ItsyBitsy specifies it.

    const char command1UUID [16] = {COMMAND1_UUID};
    commandStream_Uuids[0] = BleUuid((uint8_t *)command1UUID,BleUuidOrder::LSB); //LSB must be specified since that is how the ItsyBitsy specifies it.
    const char command2UUID [16] = {COMMAND2_UUID};
    commandStream_Uuids[1] = BleUuid((uint8_t *)command2UUID,BleUuidOrder::LSB); //LSB must be specified since that is how the ItsyBitsy specifies it.

    const char telemetryUUID [16] = {TELEMETRY_UUID};
    telemetryStream_Uuid = BleUuid((uint8_t *)telemetryUUID,BleUuidOrder::LSB); //LSB must be specified since that is how the ItsyBitsy specifies it.



    //Register all the callbacks.
    BLE.onDisconnected(GatewayBLE::disconnectCallback,this);
    //BLE.onConnected(GatewayBLE::connectedCallback,this);  //We are not using this yet.

    dataStream.onDataReceived(bleRxDataCallback,this);
    telemetryStream.onDataReceived(bleRxTelemetryCallback,this);

}

void GatewayBLE::startBLE(){

    BLE.on();
    BLE.setTxPower(8);
}

int GatewayBLE::connectBLE(){

    int numNewConnections = 0; //Keep track of how many new devices we add.

    int numDevicesFound = foundDevices.size();
    Log.info("Connecting to %d  BT devices ...",numDevicesFound);

    //Try and connect to each device.
    for(int i = 0; i < numDevicesFound; i++){

        particle::BlePeerDevice newConnection;
        newConnection = BLE.connect(foundDevices[i].addr,false); //Blocking Function.

        if(newConnection.connected()){
            
            
            int index = getDeviceId(foundDevices[i].name);
            foundDevices[i].idNum = index;

            Log.info("Connected to BT device: %s (%d)", foundDevices[i].name.c_str(),index);
            
            //Print out all characteristics, only needed for debuging.
            BleCharacteristic chars[10];
            ssize_t num = newConnection.discoverAllCharacteristics(chars, 10);
            for(int i =0; i<min(num,4);i++){
                BleUuid uuid = chars[i].UUID();
                Log.trace("UUID: %s\n",uuid.toString().c_str());
            }

            //Set the MTU and print some debug info.
            int result =  hal_ble_gatt_set_att_mtu(247, NULL);
            Log.trace("Change ATT MTU: %d", result);
            hal_ble_conn_info_t conn_info;
            result = hal_ble_gap_get_connection_info(0, &conn_info, NULL);
            Log.trace("get connection infor(0 for success) : %d",result);
            Log.trace("Con att mtu: %d",conn_info.att_mtu);
            Log.trace("Con role: %d",conn_info.role);
            Log.trace("Con version: %d",conn_info.version);

            //Check if the device has the proper characteristics.
            bool status = newConnection.getCharacteristicByUUID(dataStream,dataStream_Uuid);
            if(status){

                Log.info("Data charactreristic found!");
                dataStream.subscribe(true);
            }
            status &= newConnection.getCharacteristicByUUID(telemetryStream,telemetryStream_Uuid);
            if(status){

                Log.info("Telemetry charactreristic found!");
                telemetryStream.subscribe(true);
            }

            status &= newConnection.getCharacteristicByUUID(commandStreams[index],commandStream_Uuids[index]);
            if(status){

                int resp =  commandStreams[index].subscribe(true);
                Log.info("Command charactreristic found! %d",resp);
               
            }

            if(status){   

                //Keep reference to the connection.  
                foundDevices[i].conn = newConnection;
                connectedNodes.push_back(foundDevices[i]);
                numConnections++;
                numNewConnections++;
            }
        }
    }

    foundDevices.clear();
    return numNewConnections;
}

int GatewayBLE::scanBLE(size_t timeout){

    BLE.setScanTimeout(timeout);
    return BLE.scan(scanResultCallback, this);
}

int8_t GatewayBLE::getDeviceIndex(const BlePeerDevice& peer){

    int8_t result = -1;

    for(int i=0; i<numConnections; i++){
        if(connectedNodes[i].conn == peer){
            result = i;
        }
    }
    return result;
}


uint8_t GatewayBLE::getDeviceId(String name){

    for(int i=0; i < BLE_MAX_CONNECTION; i++){

        if(strcmp(name,approvedDevices[i])==0) return i;
    }
}

 bool GatewayBLE::sendCommand(const void* command,size_t len){

     for(int i=0; i < numConnections; i++){
        
        commandStreams[connectedNodes[i].idNum].setValue((uint8_t*)command,len);
     }
     return true; // TODO: Find out what setValue returns. Its a number but I cant find what it means.
 }


void GatewayBLE::bleRxDataCallback(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context){

    //Get our BLE object, since we set this pointer when we registered the callback.
    GatewayBLE * gatewayBLE = (GatewayBLE *) context;

    gatewayBLE->rxCount += len;  //At this point we only count the number of bytes received.

    if(data[0] == BENCHMARK_START_FLAG && !gatewayBLE->benchmarkInProgress){
        Log.info("Started benchmark");
        gatewayBLE->startTime = millis();
        gatewayBLE->benchmarkInProgress = true;
    }
    else if(data[0] == BENCHMARK_END_FLAG ){

        int node = gatewayBLE->getDeviceIndex(peer);
 
        gatewayBLE->benchmarkDone[node] = true;

        bool done = true;
        for(int i=0; i<gatewayBLE->numConnections; i++){
            done &= gatewayBLE->benchmarkDone[i];
        }

        if(done){
        gatewayBLE->endTime = millis();
        uint32_t totalTime = (gatewayBLE->endTime-gatewayBLE->startTime);

        Log.info("start: %lu, end: %lu.",gatewayBLE->startTime,gatewayBLE->endTime);
        Log.info("Received %d bytes in %lu ms from conenction %d. %f bytes/second.",gatewayBLE->rxCount, totalTime ,node,gatewayBLE->rxCount/(totalTime/1000.0)); 
        
        gatewayBLE->rxCount = 0; //Reset the rx count.

        //reset the benchmark state.
        for(int i=0; i<gatewayBLE->numConnections; i++){
            gatewayBLE->benchmarkDone[i] = false;
        }
        gatewayBLE->benchmarkInProgress = false;

        }
    }
       

}

void GatewayBLE::bleRxTelemetryCallback(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context){

    Log.trace("Recevied telemetry packet: %s", data);

}

void GatewayBLE::disconnectCallback(const BlePeerDevice& peer, void* context){
   
    //Get our BLE object, since we set this pointer when we registered the callback.
    GatewayBLE * gatewayBLE = (GatewayBLE *) context;

    //Should try and reconnect. For now just drop the connection.
    for(int i=0; i<gatewayBLE->numConnections; i++){

        if(gatewayBLE->connectedNodes[i].conn == peer){

            Log.info("device %d(%s) disconnected.",i,gatewayBLE->connectedNodes[i].name.c_str());

            gatewayBLE->connectedNodes.erase(gatewayBLE->connectedNodes.begin()+i);
            gatewayBLE->numConnections--;
        }
    }

}


void GatewayBLE::connectedCallback(const BlePeerDevice& peer, void* context){}

void GatewayBLE::scanResultCallback(const BleScanResult *scanResult, void *context){
    
    Log.info("BLE Scan Results");

    //Get our BLE object, since we set this pointer when we registered the callback.
    GatewayBLE * gatewayBLE = (GatewayBLE *) context;

    Log.trace("MAC: %02X:%02X:%02X:%02X:%02X:%02X | RSSI: %dBm",
    scanResult->address[0], scanResult->address[1], scanResult->address[2],
    scanResult->address[3], scanResult->address[4], scanResult->address[5], scanResult->rssi);


    String name = scanResult->advertisingData.deviceName();
    if (name.length() > 0) {
        Log.info("Found device: %s", name.c_str());
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


}
        