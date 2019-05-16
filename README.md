# TM4C_STDIO_Implementation

This design sets up uart, lcd, etc. as file descriptors, then uses the fprintf family of functions to target the appropriate output 
device. Additionally, while not implemented in the project, a default target could be added to enable printf() to work as well. I am happy
to discuss and develop this more, please reach out to me if you have any questions. 

This design inserts the input/output targets into fputc()/fgetc() under the identity of a global file descriptor. Afterwards, all that is 
required is to extern the file descriptor wherever it is needed. If it used in its current form, it would eliminate a user having to do
any customization, but can easily be expanded for more targets. (e.g I have used CAN descriptor for a project with multiple 
microcontrollers). 
