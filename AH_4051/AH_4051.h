
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


#ifndef AH_4051_h
#define AH_4051_h

#include <Arduino.h>   //Arduino IDE >= V1.0


class AH_4051
{
  public:

    // Constructor
    AH_4051(int A0_pin,int A1_pin,int A2_pin,int E_pin,int Z_pin);
	
    void setMuxMode();
    void setDemuxMode();

    void setValue(int Address);
    boolean readValue(int Address);

    void enable();
    void disable();

  private:
    int _Address0_pin;
    int _Address1_pin;
    int	_Address2_pin;
    int _Enable_pin;
    int _Z_pin;

};

#endif 
