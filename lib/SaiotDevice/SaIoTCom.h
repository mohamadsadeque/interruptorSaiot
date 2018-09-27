#ifndef SaIoTCom_h
#define SaIoTCom_h

#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

#define rotaDateTime "http://api.saiot.ect.ufrn.br/v1/device/history/datetime"

typedef void (*functionPointer)(char*, uint8_t*, unsigned int); //definido assim pelo MQTT_CALLBACK_SIGNATURE;

class SaIoTCom
{
  private:  
    //WiFiClient espClient;
    PubSubClient mqttClient;
  public:
    SaIoTCom();
    SaIoTCom(WiFiClient& espClient);
    String getToken(String hostHttp,String user, String password, String serial);
    static String getDateNow();
    void setServerPort(const char * server, uint16_t port);
    void setCallback(functionPointer f);
    void registerDevice(String serial,String user,String token,String jsonConf,const char* hostReg,String keys[], int nKeys);
    void setClient(WiFiClient& espClient);
    boolean handleCom();
    boolean publishData(String data);
};

#endif