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

const int LED = 10;
const int RELE_1 = D7; // D7
const int RELE_2 = D8; // D8

//estados
volatile bool stateButton = false;
volatile bool stateLED_1 = false;
volatile bool stateLED_2 = false;
volatile bool isChanged01 = false;
volatile bool isChanged02 = false;

//bloqueio
volatile bool bloquear = false;
volatile bool contando = false;
unsigned long int blockTime = 0;
unsigned short int cont = 0;
unsigned long int countTime = 0;

//processamento botões
bool lendo_1 = false;
bool lendo_2 = false;
unsigned long int lastTime_1 = 0;
unsigned long int delayLeitura_1 = 0;
short int media_1 = 0;
short int leituras_1 = 0;
unsigned long int lastTime_2 = 0;
unsigned long int delayLeitura_2 = 0;
short int media_2 = 0;
short int leituras_2 = 0;

int report(int, int);
void interrupcao_1();
void interrupcao_2();
void calcMedia(int);
//Parametros da conexão
WiFiClient espClient;

//Parametros do device
SaIoTDeviceLib sonoff("IntLabESQ32", "errrrr52", "ricardo@email.com");
SaIoTController onOff("{\"key\":\"on\",\"class\":\"onoff\",\"tag\":\"Geral\"}");
SaIoTController toggle_1("{\"key\":\"on_1\",\"class\":\"toggle\",\"tag\":\"Esquerda\"}");
SaIoTController toggle_2("{\"key\":\"on_2\",\"class\":\"toggle\",\"tag\":\"Direita\"}");
String senha = "12345678910";

//Variveis controladores
volatile bool reconfigura = false;

//Funções controladores
void setReconfigura();
//Funções MQTT
void callback(char *topic, byte *payload, unsigned int length);
//Funções padão
void setup();
void loop();
//funções
void writeAndReport(int port, int value);

void setup()
{
  Serial.begin(115200);
  pinMode(CHAVE_1, INPUT_PULLUP);
  pinMode(CHAVE_2, INPUT_PULLUP);
  pinMode(RELE_1, OUTPUT);
  pinMode(RELE_2, OUTPUT);

  delay(80);
  attachInterrupt(digitalPinToInterrupt(CHAVE_1), interrupcao_1, FALLING);
  attachInterrupt(digitalPinToInterrupt(CHAVE_2), interrupcao_2, FALLING);

  //lib
  sonoff.addController(onOff);
  sonoff.addController(toggle_1);
  sonoff.addController(toggle_2);
  sonoff.preSetCom(espClient, callback, 60);
  sonoff.start(senha);
}

void loop()
{
  sonoff.handleLoop();
  if (lendo_1)
  {
    calcMedia(CHAVE_1);
  }
  if (lendo_2)
  {
    calcMedia(CHAVE_2);
  }

  if ((cont == 1 || cont == 2) && !contando) //inicio da contagem de vezes apertadas/tempo
  {
    countTime = millis();
    contando = true;
  }
  else if (cont >= 8)
  {
    //zerar e bloquear
    digitalWrite(LED, HIGH);
    bloquear = true;
    blockTime = millis();
    Serial.println("BLOQUEOU");
    cont = 0;
    contando = false;
  }
  else if (contando && (abs(millis() - countTime) > 10000))
  {
    //zerar
    cont = 0;
    contando = false;
    Serial.println("Resetou a contagem");
  }
  if (bloquear && (abs(millis() - blockTime) > 10000))
  {
    Serial.println("Desbloqueou ");
    bloquear = false;
    digitalWrite(LED, LOW);
  }

  if (isChanged01)
  {
    cont++;
    writeAndReport(RELE_1, stateLED_1);
    isChanged01 = !isChanged01;
  }
  if (isChanged02)
  {
    cont++;
    writeAndReport(RELE_2, stateLED_2);
    isChanged02 = !isChanged02;
  }
  if (stateButton)
  {
    if(stateLED_1 == stateLED_2 && stateLED_1 == 0){
      stateButton = stateLED_1;
      //report
      report(-1,stateButton);
    }
  }else{
    if(stateLED_1 || stateLED_2){
      stateButton = 1;
      report(-1,stateButton);
    }
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
    if (!bloquear)
    {
      cont++;
      digitalWrite(RELE_1, payloadS.toInt());
      stateLED_1 = payloadS.toInt();
      Serial.print("Estado led_1: ");
      Serial.println(stateLED_1);
      //chegando um dado direto pra um dos botoes menores, atualiza o estado e escreve na porta
    }else{
      report(RELE_1,stateLED_1);
    }
  }
  if (strcmp(topic, (sonoff.getSerial() + toggle_2.getKey()).c_str()) == 0)
  {
    Serial.println("Value: " + payloadS);
    if (!bloquear)
    {
      cont++;
      digitalWrite(RELE_2, payloadS.toInt());
      stateLED_2 = payloadS.toInt();
      Serial.print("Estado led_2: ");
      Serial.println(stateLED_2);
    }else{
      report(RELE_2,stateLED_2);
    }
  }
  if (strcmp(topic, (sonoff.getSerial() + onOff.getKey()).c_str()) == 0)
  {
    Serial.println("Value: " + payloadS);
    if (!bloquear)
    {
      cont += 2;
      int valueStateRecived = payloadS.toInt();
      stateButton = valueStateRecived;
      stateLED_1 = valueStateRecived;
      stateLED_2 = valueStateRecived;
      writeAndReport(RELE_1, valueStateRecived);
      writeAndReport(RELE_2, valueStateRecived);
    }else{
      report(-1,stateButton);
    }
  }
}

void writeAndReport(int port, int value)
{
  digitalWrite(port, value);
  String cKey;
  report(port, value);
}
void setReconfigura()
{
  reconfigura = true;
}

void calcMedia(int chave)
{
  if (chave == CHAVE_1)
  {
    if (abs(millis() - delayLeitura_1) > 10) //colocar define pra esse 10 -> intervalo de tempo entre cada leitura
    {
      if (leituras_1 < 20) //define pra esse 20 -> qnt de leituras
      {
        if (!digitalRead(CHAVE_1))
        {
          media_1++;
        }
        leituras_1++;
        if (media_1 >= 10)
        {
          leituras_1 = 25;
        }
      }

      if (leituras_1 >= 20) //colocar isso em uma função seria bom
      {
        if (media_1 >= 10)
        {
          stateLED_1 = !stateLED_1;
          isChanged01 = true;
        }
        media_1 = 0;
        leituras_1 = 0;
        lendo_1 = false;
        lastTime_1 = millis();
      }
      delayLeitura_1 = millis();
    }
  }
  /////////////////////////////////////////// CHAVE 2
  else
  {
    if (abs(millis() - delayLeitura_2) > 10)
    {
      if (leituras_2 < 15)
      {
        if (!digitalRead(CHAVE_2))
        {
          media_2++;
        }
        leituras_2++;
        if (media_2 >= 10)
        {
          leituras_2 = 25;
        }
      }

      if (leituras_2 >= 15)
      {
        if (media_2 >= 10)
        {
          stateLED_2 = !stateLED_2;
          isChanged02 = true;
        }
        media_2 = 0;
        leituras_2 = 0;
        lendo_2 = false;
        lastTime_2 = millis();
      }
      delayLeitura_2 = millis();
    }
  }
}

void interrupcao_1()
{
  if (!lendo_1 && (abs(millis() - lastTime_1) > 300) && !bloquear) //trocar 300 por um define -> intervalo de tempo entre o inicio de duas leituras
  {
    lendo_1 = true;
  }
}
void interrupcao_2()
{
  if (!lendo_2 && (abs(millis() - lastTime_2) > 300) && !bloquear)
  {
    lendo_2 = true;
  }
}

int report(int type, int value)
{
  String cKey;
  if (type == RELE_1)
  {
    cKey = toggle_1.getKey();
  }
  else if (type == RELE_2)
  {
    cKey = toggle_2.getKey();
  }
  else
  {
    cKey = onOff.getKey();
  }

  if (!sonoff.reportController(cKey, value))
  {
    Serial.println("Erro ao enviar dados pro SaIoT");
  }
}