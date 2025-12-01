1 - Desenvolva um protótipo com a plataforma Arduino que atue como um Servidor Web, disponibilizando uma página HTML com um campo de texto e um botão. Quando o botão de envio é pressionado, a mensagem inserida no campo de texto deve ser recebida pela plataforma Arduino e apresentada em um Display LCD.
Componentes necessários: 
Ethernet Shield
1 (um) display LCD



2 - Desenvolva um protótipo que leia identificações de tags/cartões RFID e consulte as permissões de acesso por meio de consultas a uma API. Para este protótipo, a plataforma Arduino atuará como um Cliente HTTP. Como resposta, o protótipo deve apresentar em um Display LCD o nome do usuário da tag/cartão RFID e a mensagem “Acesso negado” ou “Acesso permitido”.
Componentes necessários: 
Ethernet Shield
1 (um) display LCD

	Detalhamento da API:
Endpoint: https://te2023-iot-038082f8e478.herokuapp.com/access/{id-RFID}
Método: GET
Exemplo de resposta:
Exemplo-1: ID encontrado
Exemplo-2: ID não encontrado
Status code: 200 (OK)
Status code: 404 (Not Found)
Body:

{
  "id":"1111111111",
  "name":"Tiago Santos",
  "access":true
}
Body:

{
  "details": "ID not found"
}



3 - Desenvolva um protótipo que monitore sensores e controle atuadores, de acordo com as mensagens recebidas e enviadas por meio do protocolo MQTT. Para monitoramento dos dados, bem como para o envio de comandos, o app IoT MQTT Panel (ou equivalente) deve ser utilizado. 
Componentes necessários: 
Ethernet Shield
1 (um) sensor ultrassônico
1 (um) buzzer

Devem ser utilizados os seguintes tópicos para os seguintes componentes:
Arduino: Publisher
	App: Subscriber
Tópico: gX/distancia
Dados de distância lidos a partir do sensor Ultrassônico.
	
	Arduino: Subscriber
	App: Publisher
Tópico: gX/buzzer
Dados relativos ao acionamento de um buzzer.

Para o teste do protótipo, uma dashboard deve ser configurada no aplicativo IoT MQTT Panel para envio/recebimento das mensagens por meio do protocolo MQTT.



4 - Desenvolva um protótipo que controle a posição de um servo motor. Os comandos relativos ao controle dos componentes devem ser recebidos pela plataforma Arduino por uma conexão Bluetooth. 
Componentes necessários: 
Módulo bluetooth HC-05
1 (um) servo motor

Segue a lista dos comandos que devem ser suportados:
Comando
Descrição
ang
Movimenta o servo motor para posição ang [0, 180]


Para o envio dos comandos, o app Serial Bluetooth Terminal deve ser utilizado para o pareamento com o módulo HC-05.
