#include <Arduino.h>
#include <SaIoTDeviceLib.h>

//Parametros da conexão
WiFiClient espClient;

//Funçoes utilizadas
void callback(char *topic, byte *payload, unsigned int length);
void sendEstadoAtual();
void acenderApagar(bool retorno);

//Parametros de funcionamento
volatile bool estado = 0;
volatile bool reportServer = false;
//const int rele = D1;
const int relePin = D7;
const int buttonPin = D3;
volatile unsigned long int tempoAnterior = 0;
//unsigned long int tempoAtual;
unsigned long int deboucingTime = 200;

//Device
SaIoTDeviceLib interruptor("Device_interruptor", "23102018LAB", "gm@email.com");
String senha = "12345678910";
//Controlador
SaIoTController contOnOff("intpS", "interruptorLab", "onoff");

void ICACHE_RAM_ATTR interrupcao()
{
  //if (estado ^ digitalRead(buttonPin))
  //{
  if (abs(millis() - tempoAnterior) > deboucingTime)
  {
    estado = !estado;
    tempoAnterior = millis();
    acenderApagar(estado);
    reportServer = true;
    //Deverá enviar os dados pro server aqui, após atualizar
  }
  //}
}


void setup()
{
  pinMode(relePin, OUTPUT);
  digitalWrite(buttonPin,HIGH);
  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPin), interrupcao, FALLING); //Configurando a interrupção
  interruptor.addController(contOnOff);
  Serial.begin(115200);
  Serial.println("START");
  interruptor.preSetCom(espClient, callback);
  interruptor.startDefault(senha);
}

void loop()
{
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
    acenderApagar(bool(payloadS.toInt()));
  }
}

void acenderApagar(bool retorno)
{
  digitalWrite(relePin, retorno);
  if (reportServer)
  {
    Serial.print(estado);
    Serial.print(" : ");
    Serial.println(reportServer);
    if (interruptor.reportController(contOnOff.getKey(), estado)){
      Serial.println("Rolou");
    }
    reportServer = false;
  }
}


