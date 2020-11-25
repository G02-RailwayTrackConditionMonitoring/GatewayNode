# GatewayNode

A Particle project named GatewayNode.

This project requires the following hardware:
    - An LED connected to pin D4.
    - A button connected to pin D5.

This project will search for BLE devices when the button is pressed and will print their addresses and names over serial.
If there is a device name "GO2" the boron will connect to it.

The cell connection is disabled for this version of the project.

You could send an API request to start blinking the LED using the following:

curl https://api.particle.io/v1/devices/DEVICE_ID/blink \
     -d access_token=TOKEN \
     -d "args=Start"

You can get DEVICE_ID from the dashboard and generate TOKEN using the Particle CLI.

Overall this project currently tests the following things:
- Input/Output
- Serial Logging
- Particle functions and variables
- Connecting to the Particle cloud.

##Some helpful commands in the Particle CLI:

Put the device into DFU mode (flashing yellow led):
- particle usb dfu

Once in DFU mode, program the board with:
- particle flash --usb src/target/1.5.2/boron/GatewayNode.bin

Open the serial monitor with:
- particle serial monitor

Generate an access token:
- particle token create

List all devices and their functions/variables:
- particle list

## Welcome to your project!

Every new Particle project is composed of 3 important elements that you'll see have been created in your project directory for GatewayNode.

#### ```/src``` folder:  
This is the source folder that contains the firmware files for your project. It should *not* be renamed. 
Anything that is in this folder when you compile your project will be sent to our compile service and compiled into a firmware binary for the Particle device that you have targeted.

If your application contains multiple files, they should all be included in the `src` folder. If your firmware depends on Particle libraries, those dependencies are specified in the `project.properties` file referenced below.

#### ```.ino``` file:
This file is the firmware that will run as the primary application on your Particle device. It contains a `setup()` and `loop()` function, and can be written in Wiring or C/C++. For more information about using the Particle firmware API to create firmware for your Particle device, refer to the [Firmware Reference](https://docs.particle.io/reference/firmware/) section of the Particle documentation.

#### ```project.properties``` file:  
This is the file that specifies the name and version number of the libraries that your project depends on. Dependencies are added automatically to your `project.properties` file when you add a library to a project using the `particle library add` command in the CLI or add a library in the Desktop IDE.

## Adding additional files to your project

#### Projects with multiple sources
If you would like add additional files to your application, they should be added to the `/src` folder. All files in the `/src` folder will be sent to the Particle Cloud to produce a compiled binary.

#### Projects with external libraries
If your project includes a library that has not been registered in the Particle libraries system, you should create a new folder named `/lib/<libraryname>/src` under `/<project dir>` and add the `.h`, `.cpp` & `library.properties` files for your library there. Read the [Firmware Libraries guide](https://docs.particle.io/guide/tools-and-features/libraries/) for more details on how to develop libraries. Note that all contents of the `/lib` folder and subfolders will also be sent to the Cloud for compilation.

## Compiling your project

When you're ready to compile your project, make sure you have the correct Particle device target selected and run `particle compile <platform>` in the CLI or click the Compile button in the Desktop IDE. The following files in your project folder will be sent to the compile service:

- Everything in the `/src` folder, including your `.ino` application file
- The `project.properties` file for your project
- Any libraries stored under `lib/<libraryname>/src`
