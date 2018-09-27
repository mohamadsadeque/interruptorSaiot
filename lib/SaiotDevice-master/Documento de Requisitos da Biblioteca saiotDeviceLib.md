# Documento de Requisitos

Cada produto (unidade comercializável) pode conter um ou mais dispositivos contendo ao menos um sensor ou um atuador.

Os dispositivos podem conter nenhum ou vários sensores, nenhum ou vários controladores.

Para cada sensor o usuário deve atualalizar o conteúdo da variável que guarda o valor a ser enviado à central e setar a variável sinalizadora da atualização para o sistema com o valor verdadeiro.

Controladores recebem uma função que realiza a tarefa de acionar uma saída e os limites de valores que podem ser usados nessa saída.

A biblioteca deve

Monitorar a conexão com a central;
Monitorar o status dos sensores e fazer o envio das informações;
Receber informações da central e atualizar as configurações automatimante;
Realizar as chamadas às funções refententes aos controladores acionados pela central;

permitir:
Incluir novos sensores, controladores e dispositivos por meio dos construtores;
Recuperar as informações dos atributos dos sensores, controladores e dispositivos;
Alterar as configurações dos dispositivos, sensores e atuadores,
