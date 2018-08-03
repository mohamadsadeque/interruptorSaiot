#include <SaIoTDeviceLib.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

//Parametros da conexão
#define HOST "10.7.227.121"
#define hostHttp "192.168.0.108:3001/device/auth/login"
#define PORT 3003 //MQTT
#define POSTDISPOSITIVO "/manager/post/device/" // v 1.7
#define rotaRetorno "/control/put/me/"
WiFiClient espClient;
PubSubClient mqttClient(espClient);

//Funçoes utilizadas 
void callback(char* topic, byte* payload, unsigned int length);
void reconnectMQTT();
String getToken();
void sendEstadoAtual();

//Parametros de funcionamento 
volatile int estado = 0;
const int Rele = D3;
const int pinInterrupt = D7;
unsigned long int tempoAnterior;
unsigned long int tempoAtual;
unsigned long int deboucingTime = 100;

//Parametros do device
String tokenRecebido = "";
String serialN = "coloridinhaMostra";

//Controlador
SaIoTController interruptor("intpS","onoff","interruptorLab");

void ICACHE_RAM_ATTR interrupcao(){
  if(estado^digitalRead(pinInterrupt)){
  if(abs(millis() - tempoAnterior) > deboucingTime){
    estado = !estado;
    tempoAnterior = millis();
    //Deverá enviar os dados pro server aqui, após atualizar 
  }
} 
}

void setup() {
  pinMode(Rele, OUTPUT);
  pinMode(pinInterrupt, INPUT);
  attachInterrupt(digitalPinToInterrupt(pinInterrupt),interrupcao,CHANGE); //Configurando a interrupção
  Serial.begin(115200);
  Serial.println("START");
  delay(50);
  //Init mqttClient
  mqttClient.setServer(HOST, PORT);
  mqttClient.setCallback(callback);
  //Conecxão wifi
  WiFiManager wifi;
  wifi.autoConnect(serialN.c_str());
  //tokenRecebido = getToken();
  //reconnectMQTT();
}

void loop(){
mqttClient.loop();
digitalWrite(Rele, estado); 
if (!mqttClient.connected()) {
  reconnectMQTT();
}

}

String getToken(){
  if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
 
   HTTPClient http;  
   http.begin("http://10.7.227.121:3001/v1/device/auth/login");  //Specify request destination
   
   http.addHeader("Content-Type","application/json"); 
 
   int httpCode = http.POST("{\"email\":\"gm@email.com\",\"password\":\"12345678910\",\"serial\":\"" + serialN + "\"}");   //Send the request
   String tokenConnect = http.getString(); 
   http.end();
   Serial.println(httpCode);
  for (int i = 0; i < 2; i++){
    tokenConnect.remove(tokenConnect.indexOf("\""),1);
  }     
  Serial.print("retorno: "); 
  Serial.println(tokenConnect);        
   return tokenConnect;
 
 }else{
   Serial.println("ERROR GET TOKEN");
   return "ERROR";

  }
}

void callback(char* topic, byte* payload, unsigned int length){
  String payloadS;
  Serial.print("Topic: ");
  Serial.println(topic);
  for (unsigned int i=0;i<length;i++) {
    payloadS += (char)payload[i];
  }
  if(strcmp(topic,serialN.c_str()) == 0){
    Serial.println("SerialLog: " + payloadS);
    estado = !estado;
  }

}
void reconnectMQTT() { //pensar em erros! Caso desconectado devido ao token n atualizado, chamar função getToken. Quanto tempo expira? 
    while (!mqttClient.connected()) {
      Serial.println("Tentando se conectar ao Broker MQTT" );
      if (mqttClient.connect(serialN.c_str(),"gm@email.com",tokenRecebido.c_str())) {
        Serial.println("Conectado");
        Serial.println(interruptor.getKey());
        mqttClient.subscribe(serialN.c_str());
        mqttClient.subscribe((serialN+interruptor.getKey()).c_str()); //subscribe da bib n aceita String ???
        String JSON;
        JSON += "{\"token\":\""+ tokenRecebido +"\",\"data\":{\"name\": \"testeInterr\",  \"serial\": \"" + serialN + "\",\"protocol\":\"mqtt\",\"controllers\":[" +  interruptor.getJsonConfig() + "]}";
        mqttClient.publish(POSTDISPOSITIVO,JSON.c_str());
      } else {
        Serial.println("Falha ao Reconectar");
        Serial.println("Tentando se reconectar em 2 segundos");
        delay(2000); //TIRAR O DELAY ? INTERRUPÇÃO N FUNCIONA MT BEM CM DELAY 
      }
    }
  }

  void sendEstadoAtual(){
    String JSON;
    JSON += "{\"key\":\"" + interruptor.getKey() + "\",\"value\":" + String(estado) + "}";
    mqttClient.publish(rotaRetorno,JSON.c_str());
}
  
