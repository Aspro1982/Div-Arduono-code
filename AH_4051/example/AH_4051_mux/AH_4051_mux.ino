#include "AH_4051.h"

//AH_4051(int A0_pin,int A1_pin,int A2_pin,int E_pin,int Z_pin)
AH_4051 IC4051(5,6,7,3,2);

void setup() { 
  IC4051.setMuxMode();
//  IC4051.disable();
//  delay(5000); 
}

void loop() {
//  IC4051.enable();

  for(int i=0;i<8;i++){
    IC4051.setValue(i);
    delay(200); 
  }

//  IC4051.enable();
  for(int i=7;i>=0;i--){
    IC4051.setValue(i);
    delay(200); 
  }
}


