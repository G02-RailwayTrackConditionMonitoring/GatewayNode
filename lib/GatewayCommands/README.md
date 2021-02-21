# GatewayCommands

Includes the header file defining the commands between the gateway processors.
Also includes some convenience functions for working with the commands/data.

GatewayCommands.h and GatewayCommands.c are shared between the ESP32 code and the Boron code, and should work wit hc and c++.
If changes are made, the files should be updated  in both projects. 
(We should make this a seperate repo and include as a submodule so we don't hae to manually do this.)