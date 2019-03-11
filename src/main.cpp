/*
Projeto para controle da porta de sala
v 1.1.2
Software:
  Danielly
  Patricio Oliveira
  Ricardo Cavalcanti
  Mohamad Sadeque
Hardware:
  Wesley Wagner
  Mohamad Sadeque
*/

#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <SaIoTDeviceLib.h>
#define CHAVE_1 14
#define CHAVE_2 12


volatile bool stateLED_1 = false;
volatile bool stateLED_2 = false;
volatile bool bloquear= false;
volatile bool contando= false;

const int LED = 10;
unsigned long int lastTime_1 = 0;
unsigned long int delayLeitura_1 = 0;
short int media_1 = 0;
short int leituras_1 = 0;
unsigned long int lastTime_2 = 0;
unsigned long int delayLeitura_2 = 0;
short int media_2 = 0;
short int leituras_2 = 0;
unsigned long int reportTime= 0;
unsigned long int countTime= 0;
unsigned long int blockTime= 0;
unsigned short int cont = 0;
const int RELE_1 = 13; // D7
const int RELE_2 = 15; // D8

bool lendo_1 = false;
bool lendo_2 = false;

void lightOn(int);
void lightOff(int);
void report();
void interrupcao_1();
void interrupcao_2();
void calcMedia(int);
//Parametros da conexão
WiFiClient espClient;

//Parametros do device
SaIoTDeviceLib sonoff("IntLabESQ32", "IntLabESQ32", "ricardo@email.com");
SaIoTController onOff("{\"key\":\"on\",\"class\":\"onoff\",\"tag\":\"Geral\"}");
SaIoTController toggle_1("{\"key\":\"on_1\",\"class\":\"toggle\",\"tag\":\"Esquerda\"}");
SaIoTController toggle_2("{\"key\":\"on_2\",\"class\":\"toggle\",\"tag\":\"Direita\"}");
String senha = "12345678910";

//Variveis controladores
volatile bool reconfigura = false;

//Funções controladores
void setReconfigura();
void setOn(String, int);
//Funções MQTT
void callback(char *topic, byte *payload, unsigned int length);
//Funções padão
void setup();
void loop();
//funções
void setupOTA();

void setup()
{
  Serial.begin(115200);
  //Serial.println("------------setup----------");
  // pinMode(RECONFIGURAPIN, INPUT_PULLUP);
  pinMode(CHAVE_1, INPUT_PULLUP);
 pinMode(CHAVE_2, INPUT_PULLUP);
pinMode(RELE_1, OUTPUT);
 pinMode(RELE_2, OUTPUT);

  delay(80);
  attachInterrupt(digitalPinToInterrupt(CHAVE_1), interrupcao_1, FALLING);
  attachInterrupt(digitalPinToInterrupt(CHAVE_2), interrupcao_2, FALLING);
  sonoff.addController(onOff);
  sonoff.addController(toggle_1);
  sonoff.addController(toggle_2);

  sonoff.preSetCom(espClient, callback, 60);
  sonoff.start(senha);
  

  setupOTA();
  Serial.begin(115200);
}

void loop()
{

  sonoff.handleLoop();
if(lendo_1){
  calcMedia(CHAVE_1);
}
if(lendo_2){
  calcMedia(CHAVE_2);
}
 
 if((cont == 1 || cont == 2) && !contando ){ 
    countTime = millis();
    contando = true;
  }

  if(contando && (abs(millis() - countTime ) < 5000 ) ){ 
    if(cont >= 8){
      digitalWrite(LED, HIGH);
    bloquear = true;
    blockTime = millis();
    Serial.println("BLOQUEOU");
    cont = 0;
    contando = false;
    }
    
    
  }

if(contando && (abs(millis() - countTime ) > 5000 )){
      cont = 0;
      contando = false;
          Serial.println("Resetou a contagem");

    }

  if(bloquear && (abs(millis() - blockTime ) > 10000 ) ){
    Serial.println("Desbloqueou ");

    bloquear = false;
    digitalWrite(LED, LOW);

  }





  stateLED_1 ? lightOn(RELE_1) : lightOff(RELE_1);
  stateLED_2 ? lightOn(RELE_2) : lightOff(RELE_2);

 if((abs(millis() - reportTime ) > 300 )){  
    report();
    reportTime = millis();
  }



}

void callback(char *topic, byte *payload, unsigned int length)
{
  String payloadS;
  Serial.print("Topic: ");
  Serial.println(topic);
  for (unsigned int i = 0; i < length; i++)
  {
    payloadS += (char)payload[i];
  }
  if (strcmp(topic, sonoff.getSerial().c_str()) == 0)
  {
    Serial.println("SerialLog: " + payloadS);
  }
  if (strcmp(topic, (sonoff.getSerial() + toggle_1.getKey()).c_str()) == 0)
  {
    Serial.println("Value: " + payloadS);
    setOn(payloadS,RELE_1);
  }
  if (strcmp(topic, (sonoff.getSerial() + toggle_2.getKey()).c_str()) == 0)
  {
    Serial.println("Value: " + payloadS);
    setOn(payloadS, RELE_2);
  }
  if (strcmp(topic, (sonoff.getSerial() + onOff.getKey()).c_str()) == 0)
  {
    Serial.println("Value: " + payloadS);
  if(!bloquear){
    if ( payloadS == "1" )
   {
    stateLED_1 = true;
    stateLED_2 = true;
    cont+=2;
   }else {
    stateLED_1=false;
    stateLED_2 = false;
    cont+=2;
   }
  }
    report();
    

  }
}

void setReconfigura()
{
  reconfigura = true;
}

void setOn(String json, int controller)
{
  if(!bloquear){
  if(controller == RELE_1){
  if ( json == "1" )
  {
    stateLED_1 = true;
    cont++;
  }else {
    stateLED_1=false;
      cont++;
  }
  }

 if(controller == RELE_2){
  if ( json == "1" )
  {
    stateLED_2 = true;
        cont++;

  }else {
    stateLED_2= false;
        cont++;

  }
  }

  }
}



void setupOTA(){

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();

}


void lightOn(int controller){
  digitalWrite(controller, HIGH);
 // digitalWrite(LED, LOW);
}
void lightOff(int controller){
  digitalWrite(controller, LOW);
 // digitalWrite(LED, HIGH);
}


void calcMedia(int chave){

  if(chave == CHAVE_1){

  if(abs(millis() - delayLeitura_1 ) > 15){
  if(leituras_1 < 20 ){
    if(!digitalRead(CHAVE_1)){
      media_1++;
    }
    leituras_1++;
    if(media_1 >= 10){
    leituras_1 = 25;
    }
  }

  if(leituras_1 >= 20){
    if(media_1 >= 10){
      stateLED_1 = !stateLED_1;
    }
    media_1 = 0;
    leituras_1 = 0;
    lendo_1 = false;
    cont++;
    lastTime_1 = millis();
    Serial.print("cont: ");
        Serial.println(cont);


  }
    delayLeitura_1 = millis();
  }

  }

  /////////////////////////////////////////// CHAVE 2
  if(chave == CHAVE_2){

  if(abs(millis() - delayLeitura_2 ) > 15){
  if(leituras_2 < 20 ){
    if(!digitalRead(CHAVE_2)){
      media_2++;
    }
    leituras_2++;
    if(media_2 >= 10){
    leituras_2 = 25;
    }
  }

  if(leituras_2 >= 20){
    if(media_2 >= 10){
      stateLED_2 = !stateLED_2;
    }
    media_2 = 0;
    leituras_2 = 0;
    lendo_2 = false;
    cont++;
    lastTime_2 = millis();
    Serial.print("cont: ");
        Serial.println(cont);
  }
    delayLeitura_2 = millis();
  }
  }
}

void interrupcao_1(){
  if(!lendo_1 &&  (abs(millis() - lastTime_1 ) > 500 ) ){
    lendo_1 = true;
    lastTime_1 = millis();
  }
}
void interrupcao_2(){
  if(!lendo_2 &&   (abs(millis() - lastTime_2 ) > 500 ) ){
    lendo_2 = true;
    lastTime_2 = millis();
  }
}

void report(){
 
  if(stateLED_1){
      sonoff.reportController("on_1", "1");
  }
  else{
      sonoff.reportController("on_1", "0");

  }

if(stateLED_2){
      sonoff.reportController("on_2", "1");
  }
  else{
      sonoff.reportController("on_2", "0");

  }
if(!stateLED_1 && !stateLED_2){
      sonoff.reportController("on", "0");
}
if(stateLED_1 || stateLED_2){
      sonoff.reportController("on", "1");
}


  }