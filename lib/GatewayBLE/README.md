# GatewayBLE Library

This library provides an abstraction for the code required to use BLE functionality on the gateway node.

The main idea behind this is that there are a lot of parameters and setup for using BLE, so we will keep the main code clean by having all the BLE implementation in this library.

The underlying libraries used for Bluetooth communication is the Particle BLE libraries, which in turn uses the _Nordic SoftDevice_ BLE software stack. These are automatically included with the Particle workbench toolchain. The underlying particle and softdevice libraries are not accessible at this point. It would at least require integrating our own build of Device-OS to be able to modify these.

Here is a link to the the Particle BLE source code, since Intellisense can never find it:
* https://github.com/particle-iot/device-os/blob/8abb0e88cc2f586dc1ff47bd8940d2602f878d2d/wiring/src/spark_wiring_ble.cpp


## Library Parameters

In the header file there are constants defined for the following:
* Service UUID: The unique ID for the service. Not really being used right now.
* Characteristic UUID: The unique ID for the data characterisitc. Used to send data from sensor node to gateway.
* Max Number of connections
* Benchmark start and end flag
* Approved devices list: A list of strings. The code will try and connect to devices with these names.

## How To Use the Library

To use the library, include the header file "GatewayBLE.h" in your file.

Create a global GatewayBLE object, and use that to access BLE functionality.

Here is a simple example:

```
#include "GatewayBLE.h"

GatewayBLE bleStack;
void setup(){

    //Start the Bluetooth radio.
    bleStack.startBLE();
}

void loop(){

    BleStack.scanBLE();
    BleStack.connectBLE();

    delay(1000);
}
```
This example will try and connect to the approved devices, every second. 
Recevied data will be benchmarked.


