
/*************************************************************************
**  Device: HEF4051 8-CH multiplexer/demultiplexer			**
**  File:   AH_4051.h   				          	**
**									**
**  Created by A. Hinkel 2012-06-24		                        **
**  Download from "http://www.arduino-projekte.de"  			**
**									**
**									**
**  Code license: CC-BY-SA						**
**  http://creativecommons.org/licenses/by-sa/3.0/             		**
**                                                                  	**
**************************************************************************/

#include <Arduino.h>
#include <AH_4051.h>

//************************************************************************
// Constructor
AH_4051::AH_4051(int A0_pin,int A1_pin,int A2_pin,int E_pin,int Z_pin){
	_Address0_pin = A0_pin;
	_Address1_pin = A1_pin;
	_Address2_pin = A2_pin;
	_Enable_pin   = E_pin;
	_Z_pin 	      = Z_pin;
	
	pinMode(_Address0_pin, OUTPUT);
	pinMode(_Address1_pin, OUTPUT);
	pinMode(_Address2_pin, OUTPUT);
	pinMode(_Enable_pin,   OUTPUT);
	pinMode(_Z_pin,        OUTPUT);
	
}


//************************************************************************

void AH_4051::setMuxMode(){
  pinMode(_Z_pin,OUTPUT);
  digitalWrite(_Z_pin, true);  
}

//************************************************************************

void AH_4051::setDemuxMode(){
  pinMode(_Z_pin,INPUT);
  digitalWrite(_Z_pin, true);  
}

//************************************************************************

void AH_4051::enable(){
  digitalWrite(_Enable_pin, false);  
}

//************************************************************************

void AH_4051::disable(){
  digitalWrite(_Enable_pin, true);  
}

//************************************************************************

void AH_4051::setValue(int Address){
  
  boolean A0 = Address&B001;
  digitalWrite(_Address0_pin, A0);

  boolean A1 = (Address&B010)>>1;
  digitalWrite(_Address1_pin, A1);

  boolean A2 = (Address&B100)>>2;  
  digitalWrite(_Address2_pin, A2);

  digitalWrite(_Z_pin, true);

  enable();
}


//************************************************************************

boolean AH_4051::readValue(int Address){
  
  boolean A0 = Address&B001;
  digitalWrite(_Address0_pin, A0);

  boolean A1 = (Address&B010)>>1;
  digitalWrite(_Address1_pin, A1);

  boolean A2 = (Address&B100)>>2;  
  digitalWrite(_Address2_pin, A2);

  return digitalRead(_Z_pin);

}
