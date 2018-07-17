#include <Arduino.h>
int estado = 0;
int tensao = 0;
int ultimaTensao = 0;
int sensorValue = 0;
const int Rele = 7;
const int sensorTensao = A0;
int condicao = 0; 
 

void interrupcao(){
  if (estado == HIGH) {    
   estado = 0;
  }
  else {
   estado = 1;
  }
  delay(20);
}
void setup() {
 pinMode(Rele, OUTPUT);
 pinMode(sensorTensao, INPUT);
 Serial.begin(9600);
 pinMode(2, INPUT);
 // attachInterrupt(0,interrupcao,RISING); //Configurando a interrupção
}

void loop(){
  /*if(digitalRead(2)){
    if(estado == HIGH){
      estado = 0;
      }
    else{
      estado = 1;
      }
     delay(20);
    }
  
  CondicaoAC();
  if(tensao != ultimaTensao){
    if(estado == HIGH){
      estado = 0;
      }
    else{
      estado = 1;
      }
     ultimaTensao = tensao; 
    }
digitalWrite(Rele, estado);  */
Serial.println(analogRead(sensorTensao));
delay(20);

}

void CondicaoAC(){
  for (int i = 0; i < 30; i++){ 
    if(analogRead(sensorTensao) < 70){ 
   tensao = 0;
   delay(10);
    }else{ 
   tensao = 220;
   delay(40);      
    }
  }  
}

  
  
