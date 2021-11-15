/******************************************************************************************************************************************************
 * Arduino MCP3551 library - Version 0.2
 *
 * Copyright (c) 2013 Simon Bauer.  All rights reserved.
 *
 * This library implements the SPI communication to one or more MCP3551 ADC devices using the single mode conversion.
 * 
 * Changelog 0.2:
 * Debugged and tested on Rosemount PT100.
 *
 * This code is licensed under a GPLv3 License.
 *
 *
 **********************************************************************************************/
#include <MCP3551/MCP3551.h>

MCP3551::MCP3551 (uint8_t CSPIN, float RZERO)
: CSPin(CSPIN), Rzero(RZERO) {
	pinMode(CSPin,OUTPUT);
	digitalWrite(CSPin,HIGH);
	SPI.begin();
	};
MCP3551::~MCP3551 (){};

boolean MCP3551::getCode() //returns 1 when a conversion was clocked to byteCode
{
	if (MCP3551::isReady()) 
	{
		//at least you will have to wait for tready max. 50ns to start with clock out
		union{
			int32_t myByteCode;
			uint8_t aByte[4];
		}c ;
		
		SPI.setClockDivider(SPI_CLOCK_DIV32); //configurate SPI
		SPI.setBitOrder(MSBFIRST);
		SPI.setDataMode(SPI_MODE0);
		
		c.aByte[3] = 0x00;
		c.aByte[2] = SPI.transfer(0);
		c.aByte[1] = SPI.transfer(0);
		c.aByte[0] = SPI.transfer(0);
		
		digitalWrite(CSPin,HIGH);
		//enable next conversion by passing a falling edge, will take about 300mikroseconds to wake device from shutdown mode.
		//meanwhile we can handle the byteCode to pass some time.
		OVH = ((c.aByte[2] & B01000000));
		OVL =((c.aByte[2] & B10000000));
		
		if (OVH && OVL) return 0; //will not touch the outgoing code. Leaves the last valid.	
		
		if (OVH && !OVL) c.aByte[2] &= B00111111; // clear both flags from byte when positive
		
		//enough time passed on high
		delayMicroseconds(1); //may be not neccessary
		digitalWrite(CSPin, LOW); 
		
		
		if ((!OVH && (c.aByte[2] & B00100000)) || OVL ) //when OVH is not set and signed (21th) bit is set we have negative value
		{
			c.aByte[2] |= B11000000; //set remaining bits high
			c.aByte[3] =0xFF; //sets high order byte, value is then negative. 
		}		
		
		delayMicroseconds(1);//may be not neccessary
		digitalWrite(CSPin,HIGH); // finally this should be enough time to switch conversion again on (typical 50ns).
		byteCode = c.myByteCode; //new byte code is now available. 
		lastPollTime = millis();
		return 1;
	}
	else return 0;
}

boolean MCP3551::isReady()
{	
	if (millis() - lastPollTime > 74) //This is 1 ms longer than a single conversion + start up should take
	{
		digitalWrite(CSPin,LOW); //to enable the devices
		//delay acc. datasheet tcl = 50ns. It may work without delay
		delayMicroseconds(1);
		if (digitalRead(MISO) == 0) //note that !rdy 
		{
			return 1 ;
		}		
		else //this should not happen, but....
		{
			lastPollTime = millis();
			digitalWrite(CSPin, HIGH); //switch off CS to be sure nothing corrupt the data
			return 0;
		}		
	}
	else return 0;	
}
boolean MCP3551::getOVH()
{
	return OVH;
}

boolean MCP3551::getOVL()
{
	return OVL;
}

boolean MCP3551::getTemp(float &temp)
{
	if ( MCP3551::getCode())
	{
		float RTD = Rcal * (float(byteCode) / ( 2097152.0 - float(byteCode)));
		RTD = (RTD / Rzero) - 1; 
		avrage[0] = avrage[1];
		avrage[1] = avrage[2];
		avrage[2] = (RTD * (255.8723 + RTD * (9.6 + RTD * 0.878)));
		temp = (avrage[0] + avrage[1] + avrage[2]) / 3;
		return 1;
	}
	return 0;
}
boolean MCP3551::calibrateZeroDegree()
{
	uint8_t n=0;
	float temp=0;
	do
	{
		if (MCP3551::getCode());
		{
			temp+= float(byteCode);
			n++;
		}
		
	} while (n<10);
	temp /= 10;
	Rcal = Rzero;
	Rcal /= ( temp /( 2097152.0 - temp));
	return 1;
}

float MCP3551::getRcal() 
{
	return Rcal;
}
void MCP3551::setRcal(float &RCAL)
{
	Rcal = RCAL;
}