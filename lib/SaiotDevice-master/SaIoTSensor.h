#ifndef SaIoTSensor_h
#define SaIoTSensor_h

#include <Arduino.h>
#include "SaIoTCom.h"

#define NULL_VALUE -1
//FALTA OUTROS MÉTODOS PRO TIMEOUT: EX O TIMEOUT DE SEGUNDOS OU DE MICROSEGUNDOS (?)
class SaIoTSensor
{
private:

  String jConf;
  String key;
  String serial;
  String lastDate;
  bool reportMe;
  double value;
  String valueStr;
  //Pensar em atributos que vão fora do json de configuração! 
  /*long int deadband = NULL_VALUE,
                    timeout = NULL_VALUE,
                    lastResolution = 0,
                    resolution = NULL_VALUE;
  double value;

  unsigned long lastTimeout = 0; 
  */
  //bool isAcumm = true , isSync = true, isDigital=true; PRECISARIA DISSO?
  // typedef void (*verifyfunct)(void); PONTEIRO P FUNÇÃO DE RETORNO
public:
  SaIoTSensor();
  SaIoTSensor(String _jsonConfig);
  SaIoTSensor(String _key, String _tag, String _unit, String _type); //campos obrigatorios p o server
  ~SaIoTSensor();

  /*****************************************************************************************************************************************************
     Sensor JSON Functions
  ******************************************************************************************************************************************************/

  String getKey(void); //1
  String getSerial(void); //2
  //1 e 2: campos necessários sempre que for enviar dados ao server 

  String getByField(String _field); 
  String getUnit(void);
  String getType(void);
  String getTag(void);
  String getJsonConfig(void);
  String getLastDate(void);
  String getString(void);
  String getValueStr(void);
  double getValue(void);
  bool getReport();

  void setJsonConfig(String _jConf);
  //void sendData(double _value);
  void sendData(double _value, String dateTime);
  void sendData(String _value, String dateTime);
  /*void setValue(double _value);
  void setTimeout(long int _timeout);
  void setDeadBand(long int _deadband);*/

  /****************************************************************
   * Sensor Reading Functions: Verification and interruptions
  ****************************************************************/

  /*void verify(void);
  bool exceededDeadband(void);
  bool exceededTimeout(void);
  bool exceededResolution(void);
  */

   /****************************************************************
   * Sensor Communication Functions: sending data
  ****************************************************************/
  //int sendValue(void);
};
#endif
