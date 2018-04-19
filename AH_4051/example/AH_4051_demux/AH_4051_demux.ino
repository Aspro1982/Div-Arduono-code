#include "AH_4051.h"

//AH_4051(int A0_pin,int A1_pin,int A2_pin,int E_pin,int Z_pin)
AH_4051 IC4051(5,6,7,3,2);

void setup() { 
  IC4051.setDemuxMode();
  Serial.begin(9600);
}

void loop() {
  
  for(int i=0;i<8;i++){
    Serial.print("CH");
    Serial.print(i);    
    Serial.print(":");    
    Serial.println(IC4051.readValue(i));    
    delay(200); 
  }
}


