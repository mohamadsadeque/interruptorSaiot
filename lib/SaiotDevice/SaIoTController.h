#ifndef SaIoTController_h
#define SaIoTController_h

#include <Arduino.h>

typedef void (*func_pointer)(String); //necessario verif
class SaIoTController
{
private:
  String jConf;
  String key;
  //func_pointer actFoo;
public:
  SaIoTController();
  SaIoTController(String _jsonConfig);
  SaIoTController(String _key, String _tag, String _tClass); //campos obrigatorios p/ o server
  ~SaIoTController();

  String getKey(void); //unico necessário p se reportar ao server, fora o cadastro
  String getClass(void);
  String getTag(void);
  String getByField(String _field);
  
  void setJsonConfig(String _jConfExt); 
  String getJsonConfig(void);

  //void setAction(func_pointer _function);
};
#endif
