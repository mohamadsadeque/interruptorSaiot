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

const int LED = 10;
unsigned long int lastTime_1 = 0;
unsigned long int delayLeitura_1 = 0;
bool ultimoEstado_1;
short int media_1 = 0;
short int leituras_1 = 0;
unsigned long int lastTime_2 = 0;
unsigned long int delayLeitura_2 = 0;
bool ultimoEstado_2;
short int media_2 = 0;
short int leituras_2 = 0;
const int RELE_1 = 13; // D7
const int RELE_2 = 15; // D8

bool lendo_1 = false;
bool lendo_2 = false;

void lightOn(int);
void lightOff(int);
void report(int);
void interrupcao_1();
void interrupcao_2();
void calcMedia(int);
//Parametros da conexão
WiFiClient espClient;

//Parametros do device
SaIoTDeviceLib sonoff("IntLabESQ23", "IntLabESQ32", "ricardo@email.com");
SaIoTController onOff("{\"key\":\"on\",\"class\":\"onoff\",\"tag\":\"Geral\"}");
SaIoTController toggle_1("{\"key\":\"on_1\",\"class\":\"toggle\",\"tag\":\"Esquerda\"}");
SaIoTController toggle_2("{\"key\":\"on_2\",\"class\":\"toggle\",\"tag\":\"Direita\"}");
String senha = "12345678910";

//Variveis controladores
volatile bool reconfigura = false;

//Funções controladores
void interruptor_1();
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
  attachInterrupt(digitalPinToInterrupt(CHAVE_1), interrupcao_1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(CHAVE_2), interrupcao_2, CHANGE);
  sonoff.addController(onOff);
  sonoff.addController(toggle_1);
  sonoff.addController(toggle_2);

  sonoff.preSetCom(espClient, callback, 60);
  sonoff.start(senha);
  ultimoEstado_1 = digitalRead(CHAVE_1);
    ultimoEstado_2 = digitalRead(CHAVE_2);

  setupOTA();
  Serial.begin(115200);
}

void loop()
{
//Serial.print("leitura butao: ");
//Serial.println(digitalRead(BUTTON));
  int tentativa = 0;
  sonoff.handleLoop();
if(lendo_1){
  calcMedia(CHAVE_1);
}
if(lendo_2){
  calcMedia(CHAVE_2);
}
 
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(500);
    if (++tentativa>=5) {
      ESP.restart();
    }
  }
  Serial.flush();

  stateLED_1 ? lightOn(RELE_1) : lightOff(RELE_1);
  stateLED_2 ? lightOn(RELE_2) : lightOff(RELE_2);

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
    if ( payloadS == "1" )
   {
    stateLED_1 = true;
    stateLED_2 = true;
   }else {
    stateLED_1=false;
    stateLED_2 = false;

   }
    report(RELE_1);
    report(RELE_2);

  }
}

void setReconfigura()
{
  reconfigura = true;
}

void setOn(String json, int controller)
{
  if(controller == RELE_1){
  if ( json == "1" )
  {
    stateLED_1 = true;
  }else {
    stateLED_1=false;
  }
  }

 if(controller == RELE_2){
  if ( json == "1" )
  {
    stateLED_2 = true;
  }else {
    stateLED_2=false;
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
          Serial.println("TO NA LEITURA 1");

  if(abs(millis() - delayLeitura_1 ) > 25){
  if(leituras_1 < 20 ){
    if(ultimoEstado_1^digitalRead(CHAVE_1)){
      media_1++;
    }
    leituras_1++;
    if(media_1 >= 10){
    leituras_1 = 25;
    }
  }

  if(leituras_1 >= 20){
    if(media_1 >= 10){
      ultimoEstado_1 = !ultimoEstado_1;
      stateLED_1 = !stateLED_1;
      report(RELE_1);
    }
    media_1 = 0;
    leituras_1 = 0;
    lendo_1 = false;
  }
    delayLeitura_1 = millis();
  }

  }

  /////////////////////////////////////////// CHAVE 2
  if(chave == CHAVE_2){
      Serial.println("TO NA LEITURA 2");

  if(abs(millis() - delayLeitura_2 ) > 25){
  if(leituras_2 < 20 ){
    if(ultimoEstado_2^digitalRead(CHAVE_2)){
      media_2++;
    }
    leituras_2++;
    if(media_2 >= 10){
    leituras_2 = 25;
    }
  }

  if(leituras_2 >= 20){
    if(media_2 >= 10){
      ultimoEstado_2 = !ultimoEstado_2;
      stateLED_2 = !stateLED_2;
      report(RELE_2);
    }
    media_2 = 0;
    leituras_2 = 0;
    lendo_2 = false;
  }
    delayLeitura_2 = millis();
  }
  }
}

void interrupcao_1(){
  if(!lendo_1 ){
    lendo_1 = true;
  }
}
void interrupcao_2(){
  if(!lendo_2 ){
    lendo_2 = true;
  }
}

void report(int controller){
  if(controller == RELE_1){
  if(stateLED_1){
      sonoff.reportController("on_1", "1");
  }
  else{
      sonoff.reportController("on_1", "0");

  }
  }
if(controller == RELE_2){
if(stateLED_2){
      sonoff.reportController("on_2", "1");
  }
  else{
      sonoff.reportController("on_2", "0");

  }
}
  }

