#include <Arduino.h>

volatile int estado = 0;
const int Rele = D3;
const int pinInterrupt = D7;
int cont = 0;
unsigned long int tempoAnterior;
unsigned long int tempoAtual;
unsigned long int deboucingTime = 100;

void ICACHE_RAM_ATTR interrupcao(){
  if(estado^digitalRead(pinInterrupt)){
  if(abs(millis() - tempoAnterior) > deboucingTime){
    cont++;
    estado = !estado;
    tempoAnterior = millis();
    Serial.print("Mudou estado: ");  
    Serial.print(estado);
    Serial.print(" Vez: ");
    Serial.println(cont);
  }
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

  
  
