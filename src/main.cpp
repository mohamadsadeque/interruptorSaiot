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
volatile bool eBotaoDuplo = false;
volatile bool stateLED_1 = false;
volatile bool stateLED_2 = false;
volatile bool isChanged01 = false;
volatile bool isChanged02 = false;
volatile float debouncing_time = 300000; //equivale 0,5 segundos(esse tempo é em micro)

//bloqueio
volatile bool isBlocked = false;
volatile bool isCounting = false;
unsigned long int timeBlocked = 0;
unsigned short int cont = 0;
unsigned long int firstTimePushed = 0;

//processamento botões
unsigned long int lastTime_1 = 0;
unsigned long int lastTime_2 = 0;

int report(int, int);
void interrupcao_1();
void interrupcao_2();
//Parametros da conexão
WiFiClient espClient;

//Parametros do device
SaIoTDeviceLib sonoff("IntLabESQ32", "testingNewFilter", "ricardo@email.com");
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
  attachInterrupt(digitalPinToInterrupt(CHAVE_1), interrupcao_1, RISING);
  attachInterrupt(digitalPinToInterrupt(CHAVE_2), interrupcao_2, RISING);

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

  if (cont == 1 && !isCounting) //inicio da contagem de vezes apertadas/tempo
  {
    firstTimePushed = millis();
    isCounting = true;
  }
  else if (cont >= 8)
  {
    //zerar e isBlocked
    digitalWrite(LED, HIGH);
    isBlocked = true;
    timeBlocked = millis();
    Serial.println("BLOQUEOU");
    cont = 0;
    isCounting = false;
  }
  else if (isCounting && (abs(millis() - firstTimePushed) > 10000))
  {
    //zerar
    cont = 0;
    isCounting = false;
    Serial.println("Resetou a contagem");
  }
  if (isBlocked && (abs(millis() - timeBlocked) > 10000))
  {
    Serial.println("Desbloqueou ");
    isBlocked = false;
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
  if (eBotaoDuplo)
  {
    if (stateLED_1 == stateLED_2 && stateLED_1 == 0)
    {
      eBotaoDuplo = stateLED_1;
      report(-1, eBotaoDuplo);
    }
  }
  else
  {
    if (stateLED_1 || stateLED_2)
    {
      eBotaoDuplo = 1;
      report(-1, eBotaoDuplo);
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
    if (!isBlocked)
    {
      cont++;
      digitalWrite(RELE_1, payloadS.toInt());
      stateLED_1 = payloadS.toInt();
      Serial.print("Estado led_1: ");
      Serial.println(stateLED_1);
      //chegando um dado direto pra um dos botoes menores, atualiza o estado e escreve na porta
    }
    else
    {
      report(RELE_1, stateLED_1);
    }
  }
  if (strcmp(topic, (sonoff.getSerial() + toggle_2.getKey()).c_str()) == 0)
  {
    Serial.println("Value: " + payloadS);
    if (!isBlocked)
    {
      cont++;
      digitalWrite(RELE_2, payloadS.toInt());
      stateLED_2 = payloadS.toInt();
      Serial.print("Estado led_2: ");
      Serial.println(stateLED_2);
    }
    else
    {
      report(RELE_2, stateLED_2);
    }
  }
  if (strcmp(topic, (sonoff.getSerial() + onOff.getKey()).c_str()) == 0)
  {
    Serial.println("Value: " + payloadS);
    if (!isBlocked)
    {
      cont += 2;
      int valueStateRecived = payloadS.toInt();
      eBotaoDuplo = valueStateRecived;
      stateLED_1 = valueStateRecived;
      stateLED_2 = valueStateRecived;
      writeAndReport(RELE_1, valueStateRecived);
      writeAndReport(RELE_2, valueStateRecived);
    }
    else
    {
      report(-1, eBotaoDuplo);
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

void interrupcao_1()
{
  if (digitalRead(CHAVE_1) == HIGH)
  {
    if ((abs(micros() - lastTime_1) >= debouncing_time) && !isBlocked)
    {
      stateLED_1 = !stateLED_1;
      isChanged01 = true;
      lastTime_1 = micros();
    }
  }
}
void interrupcao_2()
{
  if (digitalRead(CHAVE_2) == HIGH)
  {
    if ((abs(micros() - lastTime_2) >= debouncing_time) && !isBlocked)
    {
      stateLED_2 = !stateLED_2;
      isChanged02 = true;
      lastTime_2 = micros();
    }
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