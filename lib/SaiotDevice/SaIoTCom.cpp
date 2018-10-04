#include "SaIoTCom.h"

SaIoTCom::SaIoTCom(){
};
SaIoTCom::SaIoTCom(WiFiClient& espClient){
  mqttClient.setClient(espClient);
};

boolean SaIoTCom::handleCom(void){
  return mqttClient.loop();
};

String SaIoTCom::getToken(String hostHttp, String user, String password, String serial){
  if(WiFi.status()== WL_CONNECTED){
    String tokenRecebido;
    HTTPClient http;
    http.begin(hostHttp); 
    http.addHeader("Content-Type","application/json");
    int httpCode = http.POST("{\"email\":\""+user+"\",\"password\":\""+password+"\",\"serial\":\"" + serial + "\"}");   //Send the request
    if(httpCode != 200){
      Serial.println("ERROR GET TOKEN");
      return "";
    }
    tokenRecebido += http.getString();
    Serial.println("Mensagem/token: " + tokenRecebido);
    http.end();
    return tokenRecebido;
 }else{
    Serial.println("ERROR GET TOKEN");
    return "";
  }
};

String SaIoTCom::getDateNow(){
  HTTPClient http;
  http.begin(rotaDateTime);
  int httpCode = http.GET(); //Retorna o código http, caso não conecte irá retornar -1
  String payload = http.getString();
  http.end();
  if (httpCode != 200) {
    return "0";
  }
  return payload;
};

boolean SaIoTCom::publishData(String data){
  return mqttClient.publish(rotaPSensor,data.c_str());
  //Serial.println(data.c_str());
}
boolean SaIoTCom::putMeController(String data){
  bool z = mqttClient.publish(rotaPController,data.c_str());
  Serial.println("Meh");
  Serial.println(z);
  return z;
}

void SaIoTCom::setServerPort(const char* server, uint16_t port){
  mqttClient.setServer(server, port);
}

void SaIoTCom::setCallback(functionPointer f){
  mqttClient.setCallback(f);
}
void SaIoTCom::setClient(WiFiClient& espClient){
  mqttClient.setClient(espClient);
}
void SaIoTCom::registerDevice(String serial,String user,String token,String jsonConf,const char* hostReg,String keys[], int nKeys){
  //#if usedProtocol == MQTT
  while (!mqttClient.connected()){
    Serial.println("Tentando se conectar ao Broker MQTT" );
    if (mqttClient.connect(serial.c_str(),user.c_str(),token.c_str())){
      Serial.println("Conectado!");
      /*Serial.print("JSON CONFIG: ");
      Serial.println(jsonConf);*/
      if (!mqttClient.subscribe(serial.c_str())){
        Serial.println("Error subscribe in Serial topic");
      }
      for(int i=0;i<nKeys;i++){
        if(!mqttClient.subscribe((serial+keys[i]).c_str())){
          Serial.println("Error subscribe in keys topics");
        }
      } 
      //Serial.println(hostReg);
      if(mqttClient.publish(hostReg,jsonConf.c_str())){
          Serial.println("Cadastre o device no SaIoT!");
      }
    }else {
      Serial.println("Falha ao Reconectar");
      Serial.println("Tentando se reconectar em 2 segundos");
      delay(2000);
    }
  }
  //#endif
}

