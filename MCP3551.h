/******************************************************************************************************************************************************
 * Arduino MCP3551 library - Version 0.2
 *
 *
 * This library implements the SPI communication to one or more MCP3551 ADC devices using the single mode conversion.
 * 
 * Changelog 0.2:
 * Debugged and tested on Rosemount PT100.
 *
 *
 **********************************************************************************************/
#include <Arduino.h>
#include <pins_arduino.h>
#include <SPI/SPI.h>
#include <stdint.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#ifndef MCP3551_H
#define MCP3551_H

class MCP3551
{
	public:
	MCP3551 (uint8_t CSPIN, float RZERO);
	~MCP3551 ();
	
	boolean getCode();
	 /*writes the word to the reference and returns true if succeed. Should be called once a circle.
	 Conversion will need about 74ms (hardware). So you may poll the method more often to get maximum time resolution, 
	 depending on your loop time.
	 The method decides if it is time to do a new conversion. */
	
	boolean getOVH();
	boolean getOVL();	
	//These bits to know can be useful to see if everything goes right with the hardware.
	// Once both ov flags are set something went wrong with SPI communication

	
	
	int32_t byteCode; //It'll be only updated when a new and good read was accomplished. Otherwise last good reading. 
	
	boolean getTemp(float &temp);
	boolean calibrateZeroDegree();
	float getRcal();
	void setRcal(float &RCAL);
	
	private:
	boolean isReady(); //Checks the status of conversion.
	unsigned long lastPollTime;//remember the last time we have polled the !RDY Signal.
	uint8_t CSPin;
	boolean OVH, OVL;
	
	float Rzero;
	float Rcal;
	float avrage[3];
};

#endif