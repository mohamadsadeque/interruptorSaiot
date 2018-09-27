#include "SaIoTController.h"

SaIoTController::SaIoTController() {}

SaIoTController::~SaIoTController() {
}

SaIoTController::SaIoTController(String _jsonConfig){
  setJsonConfig(_jsonConfig);
}

SaIoTController::SaIoTController(String _key, String _tag, String _tClass)
{
  jConf += ("{\"key\":\"" + _key + "\",\"class\":\"" + _tClass + "\",\"tag\":\"" + _tag + "\"}");
  key = _key;
}

String SaIoTController::getKey(void)
{
  return key;
}

String SaIoTController::getJsonConfig(void){
  return jConf;
}

String SaIoTController::getByField(String _field){
  int idxField = jConf.indexOf(_field); 
	int init, end;
	
	if(idxField != -1){
		init = jConf.indexOf(":", idxField) + 1; 
    if(jConf[init] == '\"'){ 
      init = init+1;
      end = jConf.indexOf("\"", init);
    }else{
      end = jConf.indexOf(",", init);
      if(end == -1){
			    end = jConf.indexOf("}", init);
          }
		  }
    return jConf.substring(init,end); 
  }else{ 
		return "-1";
	}
}

String SaIoTController::getClass(void)
{
  return getByField("class");
}

String SaIoTController::getTag(void)
{
  return getByField("tag");
}

void SaIoTController::setJsonConfig(String _jConf){
  jConf = _jConf;
  key = getByField("key");

}



/*void SaIoTController::setAction(func_pointer _function){
  actFoo = _function;
}*/


