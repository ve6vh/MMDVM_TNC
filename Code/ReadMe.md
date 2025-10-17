# MMDVM-TNC Code
This code is in two parts: the first is the STM Cube IDE project, the second is the source code.

First, import the zip file as a project into the IDE, then import the source code. Currently there
are two supported platforms, the Zum radio Pi HAT and Arduino style board that can be mounted on
a Nucleo platform for code debugging.

Add the source directory to the 'Source Location' tab on the 'Paths and Symbols' project properties.
Add the include directory on the 'Includes' tab at the same place.

Open the IOC file and change an unused GPIO to an input, this will force a regenerate. Then 
generate the Driver and O/S code. 

Then build the project. The PI HAT code will also generate a .bin file that can be used to
flash the board with STM32 Flash.
