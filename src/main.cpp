#include <Arduino.h>

volatile int estado = 0;
const int Rele = D3;
const int pinInterrupt = D7;

void ICACHE_RAM_ATTR interrupcao(){
  if(estado^digitalRead(pinInterrupt)){
    estado = !estado;
    Serial.println("Mudou estado");  
    Serial.println(estado);
    delay(10);
  }
}

void setup() {
 pinMode(Rele, OUTPUT);
 pinMode(pinInterrupt, INPUT_PULLUP);
attachInterrupt(digitalPinToInterrupt(pinInterrupt),interrupcao,CHANGE); //Configurando a interrupção
Serial.begin(115200);
Serial.println("START");
delay(50);
}

void loop(){
digitalWrite(Rele, estado); 
/*if(digitalRead(pinInterrupt) ){
  estado = !estado;
}*/
}

  
  
