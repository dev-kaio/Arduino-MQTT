README – Projetos IoT com Arduino

Ethernet Shield • RFID • LCD • MQTT • Bluetooth • Servo Motor

Este documento descreve quatro protótipos completos, cada um com lista de componentes, montagem, códigos Arduino e passo a passo detalhado para execução.

📌 PROTÓTIPO 1 — Servidor Web + LCD

Arduino atua como servidor web, recebe texto via página HTML e exibe em um display LCD 16x2.

✔ Componentes

Arduino UNO

Ethernet Shield W5100

LCD 16x2 (com potenciômetro)

Jumpers

🛠 Montagem do LCD
LCD	Arduino
VSS	GND
VDD	5V
VO	Pino central do potenciômetro
RS	7
RW	GND
E	8
D4	9
D5	10
D6	11
D7	12
A (LED+)	5V
K (LED–)	GND
💻 Código Arduino (Servidor Web + LCD)
#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,0,50);

EthernetServer server(80);

String msg = "";

void setup() {
  lcd.begin(16, 2);
  lcd.print("Servidor Web");

  Ethernet.begin(mac, ip);
  server.begin();
}

void loop() {
  EthernetClient client = server.available();

  if (client) {
    String req = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        req += c;

        if (req.endsWith("\r\n\r\n")) break;
      }
    }

    if (req.indexOf("GET /?msg=") >= 0) {
      int start = req.indexOf("msg=") + 4;
      int end = req.indexOf(" HTTP/");
      msg = req.substring(start, end);
      msg.replace("%20", " ");
      lcd.clear();
      lcd.print(msg);
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("");
    client.println("<form action='/' method='GET'>");
    client.println("<input type='text' name='msg'>");
    client.println("<button type='submit'>Enviar</button>");
    client.println("</form>");
    
    client.stop();
  }
}

▶️ Passo a passo

Monte o LCD conforme a tabela.

Conecte o Ethernet Shield e ligue-o ao roteador.

Carregue o código no Arduino.

No navegador, abra:
http://192.168.0.50

Digite uma mensagem e clique Enviar.

A mensagem aparecerá no LCD.

📌 PROTÓTIPO 2 — Leitor RFID + API HTTP + LCD
✔ Componentes

Arduino UNO

Ethernet Shield W5100

Módulo RFID RC522

LCD 16x2

Jumpers

🛠 Conexões do RFID RC522
RC522	Arduino
SDA	10
SCK	13
MOSI	11
MISO	12
IRQ	—
GND	GND
RST	9
3.3V	3.3V
💻 Código RFID + API HTTP
#include <SPI.h>
#include <MFRC522.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>

MFRC522 rfid(10, 9);
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

byte mac[] = { 0xDE,0xAD,0xBE,0xEF,0xFE,0xEE };
IPAddress ip(192,168,0,51);

char serverName[] = "te2023-iot-038082f8e478.herokuapp.com";

EthernetClient client;

void setup() {
  SPI.begin();
  rfid.PCD_Init();

  lcd.begin(16,2);
  lcd.print("Aproxime o tag");

  Ethernet.begin(mac, ip);
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  String id = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    id += String(rfid.uid.uidByte[i]);
  }

  lcd.clear();
  lcd.print("Consultando...");

  if (client.connect(serverName, 80)) {
    client.print("GET /access/");
    client.print(id);
    client.println(" HTTP/1.1");
    client.println("Host: te2023-iot-038082f8e478.herokuapp.com");
    client.println("Connection: close");
    client.println();

    delay(500);

    String resposta = "";
    while (client.available()) {
      resposta += client.readString();
    }

    lcd.clear();

    if (resposta.indexOf("ID not found") > 0) {
      lcd.print("Nao encontrado");
    } else {
      int iName = resposta.indexOf("\"name\":\"") + 8;
      int fName = resposta.indexOf("\"", iName);
      String name = resposta.substring(iName, fName);

      int iAcc = resposta.indexOf("\"access\":") + 9;
      bool access = resposta.substring(iAcc, iAcc+4) == "true";

      lcd.print(name);
      lcd.setCursor(0,1);
      lcd.print(access ? "Acesso OK" : "Negado");
    }

    client.stop();
  }

  delay(2000);
  lcd.clear();
  lcd.print("Aproxime o tag");
}

▶️ Passos

Monte o RFID conforme a tabela.

Carregue o código.

Aproxime um cartão RFID.

O Arduino fará uma requisição:
GET /access/{ID}

O LCD exibirá:

Nome do usuário

“Acesso permitido” ou “Acesso negado”

📌 PROTÓTIPO 3 — Sensores + Atuadores via MQTT

Arduino atua como Publisher (distância) e Subscriber (buzzer).

✔ Componentes

Arduino + Ethernet Shield

Sensor ultrassônico HC-SR04

Buzzer

App IoT MQTT Panel (Android)

🛠 Conexões do Ultrassônico HC-SR04
Sensor	Arduino
VCC	5V
GND	GND
Trig	6
Echo	5
🛠 Conexões do Buzzer
Buzzer	Arduino
+	3
–	GND
🟦 Tópicos MQTT
Arduino → App (Publisher)

gX/distancia

App → Arduino (Publisher → Subscriber)

gX/buzzer

💻 Código MQTT Completo
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

byte mac[] = {0xDE,0xAD,0xBE,0xEF,0xFE,0xED};
IPAddress ip(192,168,0,60);
IPAddress mqttServer(192,168,0,10);

EthernetClient ethClient;
PubSubClient client(ethClient);

#define TRIG 6
#define ECHO 5
#define BUZZER 3

void callback(char* topic, byte* payload, unsigned int len) {
  String msg = "";
  for (int i = 0; i < len; i++) msg += (char)payload[i];

  if (String(topic) == "gX/buzzer") {
    if (msg == "on") digitalWrite(BUZZER, HIGH);
    else digitalWrite(BUZZER, LOW);
  }
}

void setup() {
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(BUZZER, OUTPUT);

  Ethernet.begin(mac, ip);
  client.setServer(mqttServer, 1883);
  client.setCallback(callback);
}

void reconnect() {
  while (!client.connected()) {
    client.connect("arduinoClient");
    client.subscribe("gX/buzzer");
  }
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  digitalWrite(TRIG, LOW); delayMicroseconds(2);
  digitalWrite(TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH);
  long distancia = duration * 0.034 / 2;

  char buffer[10];
  dtostrf(distancia, 5, 2, buffer);
  client.publish("gX/distancia", buffer);

  delay(1000);
}

▶️ Configuração no IoT MQTT Panel

Instale o IoT MQTT Panel.

Clique em New Dashboard.

Selecione MQTT Connection.

Configure:

Broker: seu servidor MQTT

Porta: 1883

Nome: Arduino MQTT

Widget 1 – Distância

Tipo: Value Display

Tópico: gX/distancia

Widget 2 – Controle do Buzzer

Tipo: Switch

Tópico: gX/buzzer

Mensagens:

ON → "on"

OFF → "off"

📌 PROTÓTIPO 4 — Controle de Servo via Bluetooth (HC-05)
✔ Componentes

Arduino UNO

Módulo HC-05

Servo Motor SG90

🛠 Ligações do HC-05
HC-05	Arduino
VCC	5V
GND	GND
TXD	RX (0)
RXD	TX (1)
🛠 Ligação do Servo
Servo	Arduino
Laranja (Sinal)	9
Vermelho	5V
Marrom	GND
💻 Código Servo + Bluetooth
#include <Servo.h>

Servo servo;

String buffer = "";

void setup() {
  Serial.begin(9600);
  servo.attach(9);
  servo.write(90);
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      if (buffer.startsWith("ang")) {
        int ang = buffer.substring(3).toInt();
        if (ang >= 0 && ang <= 180) {
          servo.write(ang);
        }
      }
      buffer = "";
    } else {
      buffer += c;
    }
  }
}

▶️ Uso no Serial Bluetooth Terminal

Abra o app.

Pareie com o módulo HC-05 (senha padrão: 1234).

Conecte-se ao dispositivo.

Envie comandos no formato:

ang0
ang45
ang120
