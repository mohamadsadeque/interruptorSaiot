#include <Arduino.h>
#include <SaIoTDeviceLib.h>

//Parametros da conexão
#define rotaRetorno "/control/put/me/"

WiFiClient espClient;

//Funçoes utilizadas
void callback(char *topic, byte *payload, unsigned int length);
void sendEstadoAtual();
void acenderApagar(String retorno);

//Parametros de funcionamento
volatile int estado = 0;
const int Rele = D3;
const int pinInterrupt = D7;
unsigned long int tempoAnterior;
unsigned long int tempoAtual;
unsigned long int deboucingTime = 100;

//Device
SaIoTDeviceLib interruptor("Device_interruptor", "2709181123LAB", "ricardo@email.com");
String senha = "12345678910";
//Controlador
SaIoTController contOnOff("intpS", "interruptorLab", "onoff");

void ICACHE_RAM_ATTR interrupcao()
{
  if (estado ^ digitalRead(pinInterrupt))
  {
    if (abs(millis() - tempoAnterior) > deboucingTime)
    {
      estado = !estado;
      tempoAnterior = millis();
      //Deverá enviar os dados pro server aqui, após atualizar
    }
  }
}

void setup()
{
  pinMode(Rele, OUTPUT);
  pinMode(pinInterrupt, INPUT);
  attachInterrupt(digitalPinToInterrupt(pinInterrupt), interrupcao, CHANGE); //Configurando a interrupção
  interruptor.addController(contOnOff);
  Serial.begin(115200);
  Serial.println("START");
  interruptor.preSetCom(espClient, callback);
  interruptor.startDefault(senha);
}

void loop()
{
  digitalWrite(Rele, estado);
  interruptor.handleLoop();
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
  if (strcmp(topic, interruptor.getSerial().c_str()) == 0)
  {
    Serial.println("SerialLog: " + payloadS);
  }
  if (strcmp(topic, (interruptor.getSerial() + contOnOff.getKey()).c_str()) == 0)
  {
    Serial.println("SerialLog: " + payloadS);
    acenderApagar(payloadS);
  }
}

void acenderApagar(String retorno){
  digitalWrite(Rele, retorno.toInt());
}

/*void sendEstadoAtual()
{
  String JSON;
  JSON += "{\"key\":\"" + contOnOff.getKey() + "\",\"value\":" + String(estado) + "}";
  mqttClient.publish(rotaRetorno, JSON.c_str());
}*/
