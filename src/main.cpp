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
#define CHAVE 14

volatile bool stateLED = true;
const int LED = 10;
unsigned long int lastTime = 0;
unsigned long int delayLeitura = 0;
bool ultimoEstado;
short int media = 0;
short int leituras = 0;
const int RELE = 13;
bool lendo = false;
void lightOn();
void lightOff();
void report(bool);
void interrupcao();
void calcMedia();
//Parametros da conexão
WiFiClient espClient;

//Parametros do device
SaIoTDeviceLib sonoff("IntLab1", "IntLab1", "ricardo@email.com");
SaIoTController onOff("{\"key\":\"on\",\"class\":\"onoff\",\"tag\":\"ON\"}");
String senha = "12345678910";

//Variveis controladores
volatile bool reconfigura = false;

//Funções controladores
void interruptor();
void setReconfigura();
void setOn(String);
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
  pinMode(CHAVE, INPUT_PULLUP);
  pinMode(RELE, OUTPUT);
  delay(80);
  attachInterrupt(digitalPinToInterrupt(CHAVE), interrupcao, CHANGE);
  sonoff.addController(onOff);
  sonoff.preSetCom(espClient, callback);
  sonoff.startDefault(senha);
  ultimoEstado = digitalRead(CHAVE);
  setupOTA();
  Serial.begin(115200);
}

void loop()
{
//Serial.print("leitura butao: ");
//Serial.println(digitalRead(BUTTON));
  int tentativa = 0;
  sonoff.handleLoop();
if(lendo){
  calcMedia();
}
 
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(500);
    if (++tentativa>=5) {
      ESP.restart();
    }
  }
  Serial.flush();

  stateLED ? lightOn() : lightOff();
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
  if (strcmp(topic, (sonoff.getSerial() + onOff.getKey()).c_str()) == 0)
  {
    Serial.println("Value: " + payloadS);
    setOn(payloadS);
  }
}

void setReconfigura()
{
  reconfigura = true;
}

void setOn(String json)
{
  if ( json == "1" )
  {
    stateLED = true;
    lightOn();
  }else {
    stateLED=false;
    lightOff();
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


void lightOn(){
  digitalWrite(RELE, LOW);
  digitalWrite(LED, LOW);
}
void lightOff(){
  digitalWrite(RELE, HIGH);
  digitalWrite(LED, HIGH);
}


void calcMedia(){
  if(abs(millis() - delayLeitura ) > 50){ // ACima de 5 já  dar break e trocar estados
  if(leituras <= 10 ){
    if(ultimoEstado^digitalRead(CHAVE)){
      media++;
    }
    leituras++;
  }

  else{
    if(media >5){
      ultimoEstado = !ultimoEstado;
      stateLED = !stateLED;
      report(stateLED);
    }
    media = 0;
    leituras = 0;
    lendo = false;
  }
    delayLeitura = millis();
  }
}

void interrupcao(){
  if((abs(millis() - lastTime) > 150) && !lendo ){
    lendo = true;
    lastTime = millis();
  }
}

void report(bool stateLED){
  if(stateLED){
      sonoff.reportController("on", "1");
  }
  else{
      sonoff.reportController("on", "0");

  }
  }