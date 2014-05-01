NexaArduino
=========

It is a Nexa Arduino Library.  

Implementation of the NEXA protocol for 433Mhz wireless controlling system.  

It now supports only receiving of NEXA products now. It will in the future support transmitting to NEXA and other devices 
I find usefull.

Usage is:  
setup() { 
	connectInput(pin);  
}

loop() {
	 NexaMessage message = getMessage(pin);
}
