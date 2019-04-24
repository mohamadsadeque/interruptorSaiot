

#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <SaIoTDeviceLib.h>
#include <Ticker.h>


//chaves
const int CHAVE_1 = 12;  //D6

//reles
const int LED = LED_BUILTIN;
const int RELE_1 = 13; //D7


//Parametros Amostragem
const int tempoCadaAmostragem = 10; //em MS
const int maxLeituras = 20;

//estados
volatile bool stateOnOff = false;
volatile bool stateLED_1 = false;
volatile bool wasReported_1 = true;


//bloqueio
volatile bool bloquear = false;
volatile bool contando = false;
unsigned long int blockTime = 0;
unsigned short int cont = 0;
unsigned long int countTime = 0;

//processamento botões
volatile bool lendo_1 = false;
unsigned long int lastTime_1 = 0;
short int media_1 = 0;
short int leituras_1 = 0;
volatile short int qntLeituras_1 = 0;



//interrupçao interna
Ticker amostragem;

int report(int, int);
void interrupcao_1();


//Parametros da conexão
WiFiClient espClient;

//Parametros do device
SaIoTDeviceLib sonoff("Lampadas", "2404LAB", "ricardo@email.com");
SaIoTController toggle_1("{\"key\":\"on_1\",\"class\":\"toggle\",\"tag\":\"01\"}");
String senha = "12345678910";

//Funções MQTT
void callback(char *topic, byte *payload, unsigned int length);
//Funções padão
void setup();
void loop();
//funções
void writeAndReport(int port, int value);
void ICACHE_RAM_ATTR setAmostragem();
void verifyBlock();
void verifyReport();

void setup()
{
  //Serial.begin(115200);
  pinMode(CHAVE_1, INPUT_PULLUP);
  pinMode(RELE_1, OUTPUT);
  pinMode(LED, OUTPUT);

  delay(80);
  attachInterrupt(digitalPinToInterrupt(CHAVE_1), interrupcao_1, FALLING);


  //lib2
  sonoff.addController(toggle_1);
  sonoff.preSetCom(espClient, callback, 240);
  sonoff.start(senha);
}

void loop()
{
  sonoff.handleLoop();
  verifyBlock();
  verifyReport();
}
void verifyReport()
{
  if (!wasReported_1)
  {
    wasReported_1 = !report(RELE_1, stateLED_1);
  }
 
}

void verifyBlock()
{
  if (cont != 0 && !contando) //inicio da contagem de vezes apertadas/tempo
  {
    countTime = millis();
    contando = true;
  }
  else if (cont >= 12)
  {
    //zerar e bloquear
    digitalWrite(LED, HIGH);
    bloquear = true;
    blockTime = millis();
    //Serial.println("BLOQUEOU");
    cont = 0;
    contando = false;
  }
  else if (contando && (abs(millis() - countTime) > 10000))
  {
    //zerar
    cont = 0;
    contando = false;
    //Serial.println("Resetou a contagem");
  }
  if (bloquear && (abs(millis() - blockTime) > 10000))
  {
    //Serial.println("Desbloqueou ");
    bloquear = false;
    digitalWrite(LED, LOW);
  }
}

void writeAndReport(int port, int value)
{
  digitalWrite(port, value);
  String cKey;
  report(port, value);
}

void interrupcao_1()
{
  if (!lendo_1 && (abs(millis() - lastTime_1) > 300) && !bloquear) //trocar 300 por um define -> intervalo de tempo entre o inicio de duas leituras
  {
    //Serial.println("Habilitou01");
    amostragem.attach_ms(tempoCadaAmostragem, setAmostragem);
    lendo_1 = true;
  }
}


void ICACHE_RAM_ATTR setAmostragem()
{
  if (lendo_1)
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
        //Serial.println("ACENDER LUZ 1");
        cont++;
        digitalWrite(RELE_1, stateLED_1);
        //acender luz
      }
      media_1 = 0;
      qntLeituras_1 = 0;
      lendo_1 = false;
      lastTime_1 = millis();

    }
  }

  if (!lendo_1 )
  {
    amostragem.detach(); // Desabilita interrupção interna
  }

}

int report(int type, int value)
{
  String cKey;
  if (type == RELE_1)
  {
    cKey = toggle_1.getKey();
  }
  
  return !sonoff.reportController(cKey, value);
}

void callback(char *topic, byte *payload, unsigned int length)
{
  String payloadS;
  //Serial.print("Topic: ");
  //Serial.println(topic);
  for (unsigned int i = 0; i < length; i++)
  {
    payloadS += (char)payload[i];
  }
  if (strcmp(topic, sonoff.getSerial().c_str()) == 0)
  {
    //Serial.println("SerialLog: " + payloadS);
  }
  if (strcmp(topic, (sonoff.getSerial() + toggle_1.getKey()).c_str()) == 0)
  {
    //Serial.println("Value: " + payloadS);
    if (!bloquear)
    {
      cont++;
      digitalWrite(RELE_1, payloadS.toInt());
      stateLED_1 = payloadS.toInt();

      //chegando um dado direto pra um dos botoes menores, atualiza o estado e escreve na porta
    }
    else
    {
      report(RELE_1, stateLED_1);
    }
  }

}