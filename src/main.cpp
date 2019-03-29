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
#include <Ticker.h>

#define CHAVE_1 14
#define CHAVE_2 12

const int LED = 10;
const int RELE_1 = D7;              // D7
const int RELE_2 = D8;              // D8
const int tempoCadaAmostragem = 10; //em MS
const int maxLeituras = 20;

//estados
volatile bool stateButton = false;
volatile bool stateLED_1 = false;
volatile bool stateLED_2 = false;
volatile bool wasReported_1 = true;
volatile bool wasReported_2 = true;

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
volatile short int qntLeituras_1 = 0;
volatile short int qntLeituras_2 = 0;

//interrupçao interna
Ticker amostragem_1;
Ticker amostragem_2;

int report(int, int);
void interrupcao_1();
void interrupcao_2();
void calcMedia(int);
//Parametros da conexão
WiFiClient espClient;

//Parametros do device
SaIoTDeviceLib sonoff("Teste Int Off", "290319Test", "ricardo@email.com");
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
void setAmostragem1();
void setAmostragem2();

void setup()
{
  Serial.begin(115200);
  pinMode(CHAVE_1, INPUT_PULLUP);
  pinMode(CHAVE_2, INPUT_PULLUP);
  pinMode(RELE_1, OUTPUT);
  pinMode(RELE_2, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  delay(80);
  attachInterrupt(digitalPinToInterrupt(CHAVE_1), interrupcao_1, FALLING);
  attachInterrupt(digitalPinToInterrupt(CHAVE_2), interrupcao_2, FALLING);

  //lib2
  sonoff.addController(onOff);
  sonoff.addController(toggle_1);
  sonoff.addController(toggle_2);
  sonoff.preSetCom(espClient, callback, 60);
  sonoff.start(senha);
}

void loop()
{
  sonoff.handleLoop();
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

  if (!wasReported_1)
  {
    report(RELE_1, stateLED_1);
    wasReported_1 = true;
  }
  if (!wasReported_2)
  {
    report(RELE_2, stateLED_2);
    wasReported_2 = true;
  }
  if (stateButton)
  {
    if (stateLED_1 == stateLED_2 && stateLED_1 == 0)
    {
      stateButton = stateLED_1;
      //report
      report(-1, stateButton);
    }
  }
  else
  {
    if (stateLED_1 || stateLED_2)
    {
      stateButton = 1;
      report(-1, stateButton);
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
  if (!lendo_1 && (abs(millis() - lastTime_1) > 300) && !bloquear) //trocar 300 por um define -> intervalo de tempo entre o inicio de duas leituras
  {
    Serial.println("Habilitou01");
    amostragem_1.attach_ms(tempoCadaAmostragem, setAmostragem1);
    lendo_1 = true;
  }
}
void interrupcao_2()
{
  if (!lendo_2 && (abs(millis() - lastTime_2) > 300) && !bloquear)
  {
    Serial.println("Habilitou02");
    amostragem_2.attach_ms(tempoCadaAmostragem, setAmostragem2);
    lendo_2 = true;
  }
}

void setAmostragem1()
{
  if (!digitalRead(CHAVE_1))
  {
    media_1++;
  }
  qntLeituras_1++;
  if (media_1 >= (maxLeituras / 2) || qntLeituras_1 >= maxLeituras)
  {
    if (media_1 >= (maxLeituras / 2) && !bloquear)
    {
      stateLED_1 = !stateLED_1;
      wasReported_1 = false;
      Serial.println("ACENDER LUZ 1");
      cont++;
      digitalWrite(RELE_1, stateLED_1);
      //acender luz
    }
    media_1 = 0;
    qntLeituras_1 = 0;
    lendo_1 = false;
    lastTime_1 = millis();
    //desabilitar interrupt
    Serial.println("Desabilitou01");
    amostragem_1.detach();
    return;
  }
}

void setAmostragem2()
{
  if (!digitalRead(CHAVE_2))
  {
    media_2++;
  }
  qntLeituras_2++;
  if (media_2 >= (maxLeituras / 2) || qntLeituras_2 >= maxLeituras)
  {
    if (media_2 >= (maxLeituras / 2) && !bloquear)
    {
      stateLED_2 = !stateLED_2;
      wasReported_2 = false;
      Serial.println("ACENDER LUZ 2");
      cont++;
      digitalWrite(RELE_2, stateLED_2);
    }
    media_2 = 0;
    qntLeituras_2 = 0;
    lendo_2 = false;
    lastTime_2 = millis();
    Serial.println("Desabilitou02");
    amostragem_2.detach();
    //desabilitar interrupt
    return;
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
  Serial.print("Reported: ");
  Serial.println(cKey);
  if (!sonoff.reportController(cKey, value))
  {
    Serial.println("Erro ao enviar dados pro SaIoT");
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
    if (!bloquear && wasReported_1)
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
    if (!bloquear && wasReported_2)
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
    if (!bloquear && wasReported_1 && wasReported_2)
    {
      cont += 2;
      int valueStateRecived = payloadS.toInt();
      stateButton = valueStateRecived;
      stateLED_1 = valueStateRecived;
      stateLED_2 = valueStateRecived;
      writeAndReport(RELE_1, valueStateRecived);
      writeAndReport(RELE_2, valueStateRecived);
    }
    else
    {
      report(-1, stateButton);
    }
  }
}