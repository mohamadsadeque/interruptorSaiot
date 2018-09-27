# SaIoTDevicelib
Biblioteca genérica para gerenciamento de dispositivos IoT

## Contém:
- Gerenciamento de *WiFi* (WiFiManager);
- Gerenciamento de JSONs (ArduinoJson);
- Suporte a diferentes tipos de dispositivos (Analógicos, Digitais, Acumuladores, dentre outros)

## Como usar:
### Incluindo  no projeto
1. Para usar basta, primeiramente, fazer o *download* do código, extrair e colar na pasta de bibliotecas do seu projeto;
2. Em seguida, deve-se incluí-la em seu projeto e criar um objeto conforme exemplificado abaixo;
  ```c++
 #include  <SaIoTDeviceLib.h>
SaIoTDeviceLib myDevice;
```
### Configurando
Uma vez incluída, a biblioteca facilitará várias etapas do seu projeto. Aproveitando o objeto exemplo criado no item anterior temos os seguintes comandos para:

#### Iniciar conexão com a internet e com o servidor
```c++
void setup(){
	mydevice.start(SERIAL,HOST,PORT,POST)
}
`````
sendo:

| NOME |TIPO  | DESCRIÇÃO
|:------:|:------:|:-------:|
| Serial | char* |*em construção*|
|Host|char*|*em construção*|
|Port|int|Porta de comunicação |
|Post|int|Nome do evento que irá receber os dados

**OBS.:** Caso seja a primeira conexão, o dispositivo entrará no modo AP. Para conectar, basta então, acessar a rede gerada pelo microcontrolador e configurá-la. No mais, o dispositivo tentará se reconectar com a rede salva anteriormente.

### Adicionar sensores
Os sensores do seu dispositivo podem ser adicionados por meio do seguinte método
```c++
void setup(){
[...]
	mydevice.addSensors(key,isSync,deadband,timeout,isDigital,isAcumm,label,valor,unit);
}
```
onde

| NOME |TIPO  | DESCRIÇÃO
|:----:|:----:|:-----:|
| Key| char* |*em construção*|
|Is Sync|bool|*true* se o dispositivo se comunicar com o servidor de forma síncrona.|
|Deadband|int|*em construção*|
|Timeout| int|*em construção*|
|Is Digital| bool|*true* se o dispositivo for digital, *false* se for analógico.|
|Is Acumm|bool| *true* se o dispositivo for do tipo acumulador.|
|Label|char*|*em construção*|
|Valor| double| Dado coletado do ambiente pelo sensor.|
|Unit|char*| Unidade referente ao valor enviado.|

### Loop
Para o correto funcionamento da biblioteca, o seguinte método deve ser executado no `void loop()`

```c++
void loop(){
	mydevice.handle();
}
```
