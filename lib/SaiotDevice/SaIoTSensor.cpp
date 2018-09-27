#include "SaIoTSensor.h"

SaIoTSensor::~SaIoTSensor() {}

SaIoTSensor::SaIoTSensor(String _jsonConfig){
    setJsonConfig(_jsonConfig);
}
SaIoTSensor::SaIoTSensor(String _key, String _tag, String _unit, String _type){
    jConf += ("{\"key\":\"" + _key + "\",\"unit\":\"" + _unit + "\",\"type\":\"" + _type + "\",\"tag\":\"" + _tag + "\"}");
    key = _key;
}
String SaIoTSensor::getJsonConfig(void){
    return jConf;
}
String SaIoTSensor::getKey(void){
    return key;
}
String SaIoTSensor::getSerial(void){
    return serial;
}
String SaIoTSensor::getValueStr(void){
    return valueStr;
}
double SaIoTSensor::getValue(void){
    return value;
}
bool SaIoTSensor::getReport(){
    if(reportMe == 1){
        reportMe = 0;
        return 1;
    }else{
        return reportMe;
    }  
}
String SaIoTSensor::getByField(String _field){
    int idxField = jConf.indexOf(_field); //field
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

String SaIoTSensor::getUnit(void){
    return getByField("unit");
}
String SaIoTSensor::getTag(void){
    return getByField("tag");
}
String SaIoTSensor::getType(void){
    return getByField("type");
}
String SaIoTSensor::getLastDate(void){
    return lastDate;
}
String SaIoTSensor::getString(void){
    return valueStr;
}

//METODOS SET 

void SaIoTSensor::setJsonConfig(String _jsonConfig){
    jConf = _jsonConfig;
    key = getByField("key");
    serial = getByField("serial");
}

/*void SaIoTSensor::sendData(double _value){
    //reportMe = 1;
    return this->sendData(_value,SaIoTCom::getDateNow()); //requisição http pra cada envio??? Gerenciar hora!

}*/
void SaIoTSensor::sendData(double _value, String dateTime){
    reportMe = 1;
    value = _value;
    lastDate = dateTime;
}
void SaIoTSensor::sendData(String _value, String dateTime){
    reportMe = 1;
    valueStr = _value;
    lastDate = dateTime;
}

//FUNCIONALIDADES FUTURAS

/*void SaIoTSensor::setValue(double _value){
    value = _value;
}*/

/*void SaIoTSensor::verify(void){
    
    if(deadband != NULL_VALUE){
        if(exceededDeadband()){
            sendValue();//usar aqui o ponteiro pra função
        }
    }
    if(timeout != NULL_VALUE){
        if(exceededTimeout()){
            sendValue();
        }
    }
    if(resolution != NULL_VALUE){
        if(exceededResolution()){
            sendValue();
        }
    }
}

bool SaIoTSensor::exceededDeadband(){
    bool ret = value>deadband ? true : false;
    return ret;
}
bool SaIoTSensor::exceededTimeout(){
    unsigned long int currentTime = millis();
    if(lastTimeout == 0){
        lastTimeout = currentTime; //equivalente ao start
        return false;
    }
    bool ret = (currentTime - lastTimeout) > timeout ? true : false;
    return ret;
}
bool SaIoTSensor::exceededResolution(){
 //pensar em como mensurar
 return false;
}
*/
/*int SaIoTSensor::sendValue(void){ //essa função n existirá, o usuário que vai definir o que acontece. Então chamaria o ponteiro pra função 
    lastTimeout = millis();
    //enviar
    Serial.println("ENVIOU");
    return 0;
}

void SaIoTSensor::setTimeout(long int _timeout){
 timeout = _timeout;
}
void SaIoTSensor::setDeadBand(long int _deadband){
 deadband = _deadband;
}*/