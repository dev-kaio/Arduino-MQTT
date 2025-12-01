1) Servidor Web em Arduino — página HTML com campo de texto + botão → mostra mensagem no LCD
Componentes

Arduino Uno (ou similar)

Ethernet Shield W5100 / W5200 (plugado no Arduino)

Display LCD 16x2 (com interface paralela) + potenciómetro 10k para contraste

Cabos jumper

Fonte / cabo USB para alimentação

Observação: se seu LCD for I2C (módulo PCF8574), fiação é mais simples — expliquei a versão paralela (mais comum).

Fiação (LCD 16x2 paralelo)

LCD VSS → GND

LCD VCC → 5V

LCD V0 (contraste) → meio do pot 10k (extremos → 5V e GND)

LCD RS → Arduino D7

LCD RW → GND

LCD E → Arduino D6

LCD D4 → Arduino D5

LCD D5 → Arduino D4

LCD D6 → Arduino D3

LCD D7 → Arduino D2

(Se usar iluminação do LCD) LED+ → 5V via resistor, LED- → GND

Ethernet Shield: encaixado no Arduino (não precisa fiação extra). Plug RJ45 na rede.

Código (Arduino) — Web Server simples

Este sketch cria um servidor HTTP na porta 80. Quando você abre a página no browser, verá um campo de texto e um botão. Ao submeter, o Arduino recebe o texto e exibe no LCD.

#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>

// Configuração do LCD (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

// Defina aqui o MAC e o IP (se quiser IP estático) ou deixe DHCP
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// Se quiser IP fixo: uncomment and set
// IPAddress ip(192,168,1,177);

EthernetServer server(80);

void setup() {
  Serial.begin(9600);
  lcd.begin(16,2);
  lcd.print("Iniciando...");
  delay(1000);

  // Inicia Ethernet com DHCP
  if (Ethernet.begin(mac) == 0) {
    // DHCP falhou, tenta IP fixo comentado acima
    lcd.clear();
    lcd.print("DHCP falhou");
    // Se quiser, definir IP manual:
    // Ethernet.begin(mac, ip);
    while(true);
  }
  delay(500);
  server.begin();
  lcd.clear();
  lcd.print("Servidor Ready");
  Serial.print("IP: ");
  Serial.println(Ethernet.localIP());
}

String readFormValue(String request, String name) {
  // procura "name=value" no corpo (URL encoded)
  int idx = request.indexOf(name + "=");
  if (idx == -1) return "";
  int start = idx + name.length() + 1;
  int amp = request.indexOf('&', start);
  int end = (amp == -1) ? request.length() : amp;
  String val = request.substring(start, end);
  // Decodifica %20 etc
  String decoded = "";
  for (int i=0;i<val.length();i++){
    char c = val[i];
    if (c == '+') decoded += ' ';
    else if (c == '%' && i+2 < val.length()) {
      String hex = val.substring(i+1,i+3);
      char ch = (char) strtol(hex.c_str(), NULL, 16);
      decoded += ch;
      i += 2;
    } else decoded += c;
  }
  return decoded;
}

void loop() {
  EthernetClient client = server.available();
  if (client) {
    boolean currentLineIsBlank = true;
    String req = "";
    // lê requisição
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        req += c;
        // detecta fim do header (linha em branco)
        if (c == '\n' && currentLineIsBlank) break;
        if (c == '\n') currentLineIsBlank = true; 
        else if (c != '\r') currentLineIsBlank = false;
      }
    }

    // Se for POST, leia body adicional (Content-Length)
    int contentLength = 0;
    // encontra header Content-Length
    int idxCL = req.indexOf("Content-Length:");
    if (idxCL != -1) {
      int start = idxCL;
      int lineEnd = req.indexOf("\r\n", start);
      String clLine = req.substring(start, lineEnd);
      int colon = clLine.indexOf(':');
      if (colon != -1) {
        String num = clLine.substring(colon+1);
        num.trim();
        contentLength = num.toInt();
      }
    }

    String body = "";
    if (contentLength > 0) {
      // lê o corpo exatamente
      while (client.available() < contentLength) {
        delay(1);
      }
      for (int i=0;i<contentLength;i++){
        body += (char)client.read();
      }
    }

    // responde com página
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html; charset=UTF-8");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE html><html><head><meta charset='utf-8'><title>Arduino LCD</title></head><body>");
    client.println("<h3>Envie mensagem para o LCD</h3>");
    client.println("<form method='POST' action='/'>");
    client.println("<input type='text' name='mensagem' maxlength='32' style='width:300px;'>");
    client.println("<input type='submit' value='Enviar'>");
    client.println("</form>");
    client.println("</body></html>");

    // Se veio POST com campo mensagem, exibe no LCD
    if (body.length() > 0) {
      String msg = readFormValue(body, "mensagem");
      msg.trim();
      if (msg.length() > 0) {
        // Mostra até 32 caracteres (dois linhas)
        lcd.clear();
        if (msg.length() <= 16) {
          lcd.setCursor(0,0);
          lcd.print(msg);
        } else {
          // divide em duas linhas
          lcd.setCursor(0,0);
          lcd.print(msg.substring(0,16));
          lcd.setCursor(0,1);
          int rem = msg.length() - 16;
          int take = min(16, rem);
          lcd.print(msg.substring(16,16+take));
        }
      }
    }

    delay(1);
    client.stop();
  }
}

Teste

Faça upload do sketch ao Arduino com Ethernet Shield.

Abra o Serial Monitor — note o IP obtido por DHCP (Ethernet.localIP()).

No navegador do seu computador (na mesma rede), acesse http://<IP> — verá formulário.

Escreva mensagem e clique Enviar — mensagem aparecerá no LCD.

2) RFID + consulta a API HTTP (Arduino como cliente HTTP) → exibe nome e “Acesso permitido/negado” no LCD
Componentes

Arduino Uno

Ethernet Shield W5100

Display LCD 16x2

Leitor RFID RC522 (ou similar) + tags/cards

Cabos jumper

Fonte

Importante (HTTPS): o endpoint fornecido é https://te2023-iot-038082f8e478.herokuapp.com/access/{id-RFID} (HTTPS). O Ethernet Shield padrão não suporta TLS/HTTPS nativamente. Existem 2 opções práticas:

Proxy HTTP local (recomendado): rode um pequeno servidor HTTP local (Node.js) na sua rede que receba requisições HTTP do Arduino e repasse para o endpoint HTTPS. Forneço código de proxy abaixo. O Arduino consulta o proxy via http://<IP-proxy>/access/{id}.

Usar uma placa com suporte TLS (ex.: ESP32) — se quiser, posso converter o sketch depois.

Vou fornecer aqui a solução com proxy Node.js + sketch Arduino HTTP simples.

Proxy Node.js (opção rápida)

Coloque este arquivo proxy.js em um PC/raspberry na mesma rede do Arduino. Ele recebe GET /access/{id} e faz requisição HTTPS ao endpoint remoto e retorna a resposta ao Arduino como JSON.

// proxy.js
const http = require('http');
const https = require('https');

const PORT = 3000; // porta do proxy
const TARGET_HOST = 'te2023-iot-038082f8e478.herokuapp.com';

const server = http.createServer((req, res) => {
  const urlParts = req.url.split('/');
  // espera /access/{id}
  if (urlParts.length >= 3 && urlParts[1] === 'access') {
    const id = urlParts.slice(2).join('/');
    const options = {
      hostname: TARGET_HOST,
      path: '/access/' + encodeURIComponent(id),
      method: 'GET'
    };
    const proxyReq = https.request(options, (proxyRes) => {
      let data = '';
      proxyRes.on('data', chunk => data += chunk);
      proxyRes.on('end', () => {
        res.writeHead(proxyRes.statusCode, {'Content-Type': 'application/json'});
        res.end(data);
      });
    });
    proxyReq.on('error', (e) => {
      res.writeHead(500);
      res.end(JSON.stringify({ error: e.message}));
    });
    proxyReq.end();
  } else {
    res.writeHead(404);
    res.end(JSON.stringify({ details: "Not found" }));
  }
});

server.listen(PORT, () => {
  console.log(`Proxy rodando na porta ${PORT}`);
});


Como usar:

No PC com Node.js instalado: node proxy.js

Anote o IP do PC (ex.: 192.168.1.50) e porta 3000.

O Arduino fará requisições para http://192.168.1.50:3000/access/{id}.

Fiação (RC522 - SPI)

RC522 pins → Arduino Uno:

SDA (SS) → D10

SCK → D13

MOSI → D11

MISO → D12

RST → D9

3.3V → 3.3V (não alimentar com 5V!)

GND → GND

LCD: mesma fiação do protótipo 1.

Ethernet shield encaixado sobre Arduino.

Bibliotecas necessárias

MFRC522 (GitHub / Library Manager)

SPI

Ethernet

Código Arduino (RFID + HTTP GET via proxy)
#include <SPI.h>
#include <MFRC522.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>

#define RST_PIN 9
#define SS_PIN 10

MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal lcd(7,6,5,4,3,2);

// Ethernet (ajuste MAC e, se quiser, IP)
byte mac[] = { 0xDE,0xAD,0xBE,0xEF,0xFE,0xED };
EthernetClient client;

const char* proxyHost = "192.168.1.50"; // IP do PC rodando proxy (alterar)
const int proxyPort = 3000;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  lcd.begin(16,2);
  lcd.print("RFID & API");
  delay(1000);

  if (Ethernet.begin(mac) == 0) {
    lcd.clear();
    lcd.print("Ethernet failed");
    while(true);
  }
  lcd.clear();
  lcd.print("Pronto, coloque tag");
}

String uidToString(MFRC522::Uid uid) {
  String s = "";
  for (byte i = 0; i < uid.size; i++) {
    if (uid.uidByte[i] < 0x10) s += "0";
    s += String(uid.uidByte[i], HEX);
  }
  s.toUpperCase();
  return s;
}

void loop() {
  // procura por nova tag
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  String uid = uidToString(mfrc522.uid);
  Serial.print("UID: ");
  Serial.println(uid);
  lcd.clear();
  lcd.print("Lendo...");
  lcd.setCursor(0,1);
  lcd.print(uid);

  // consulta proxy: GET /access/{uid}
  if (client.connect(proxyHost, proxyPort)) {
    String path = "/access/" + uid;
    client.print(String("GET ") + path + " HTTP/1.1\r\n" +
                 "Host: " + proxyHost + "\r\n" +
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 4000) {
        client.stop();
        lcd.clear();
        lcd.print("Timeout proxy");
        delay(2000);
        return;
      }
    }
    // lê resposta HTTP e extrai JSON
    String response = "";
    while (client.available()) {
      response += (char)client.read();
    }
    client.stop();
    // encontra o início do JSON (procura por '{')
    int jstart = response.indexOf('{');
    if (jstart != -1) {
      String json = response.substring(jstart);
      Serial.println("JSON: " + json);
      // simples parsing sem biblioteca
      if (json.indexOf("\"access\":true") != -1) {
        // obtém name se possível
        int nstart = json.indexOf("\"name\":\"");
        String name = "Usuario";
        if (nstart != -1) {
          int ns = nstart + 8;
          int ne = json.indexOf('"', ns);
          if (ne > ns) name = json.substring(ns, ne);
        }
        lcd.clear();
        lcd.print(name);
        lcd.setCursor(0,1);
        lcd.print("Acesso permitido");
      } else if (json.indexOf("\"details\":") != -1 || response.indexOf("404") != -1) {
        lcd.clear();
        lcd.print("ID nao encontrado");
        lcd.setCursor(0,1);
        lcd.print("Acesso negado");
      } else {
        lcd.clear();
        lcd.print("Resposta invalida");
      }
    } else {
      lcd.clear();
      lcd.print("JSON nao encontrado");
    }
  } else {
    lcd.clear();
    lcd.print("Erro conexao");
  }

  delay(3000);
  // descarta card atual antes de nova leitura
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

Testes

Rode node proxy.js no PC (ver IP).

Faça upload do sketch para o Arduino. Coloque o RFID sobre o leitor.

Ao aproximar tag, o LCD mostrará o UID e depois o nome + "Acesso permitido" ou "Acesso negado".

Se quiser testar sem tags, use curl no proxy: curl http://localhost:3000/access/1111111111 para ver JSON de exemplo.

3) Monitoramento com MQTT (Ultrassônico publisher → gX/distancia; Arduino subscriber gX/buzzer) — uso do app IoT MQTT Panel
Componentes

Arduino Uno

Ethernet Shield W5100

Sensor ultrassônico HC-SR04

Buzzer (passivo ou ativo) + resistor

Cabos

Fonte

Fiação

HC-SR04:

Vcc → 5V

GND → GND

Trig → D8

Echo → D9

Buzzer → D5 (saída digital) → GND. Se buzzer passivo, use resistor e gerar tom; se ativo, apenas digitalWrite HIGH/LOW.

Ethernet Shield encaixado.

Bibliotecas

Ethernet

PubSubClient (para MQTT)

Broker MQTT

Você pode usar um broker público (ex.: test.mosquitto.org porta 1883) ou instalar um broker local (Mosquitto). Aqui usarei test.mosquitto.org como exemplo público. Se preferir um broker com autenticação, substitua credenciais no sketch.

Tópicos (conforme enunciado)

Arduino publica: gX/distancia → valores de distância (em cm)

Arduino subscreve: gX/buzzer → mensagens para acionar o buzzer (ex.: "ON", "OFF", ou número de milissegundos)

Código Arduino (MQTT)
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Ultrassom
const int trigPin = 8;
const int echoPin = 9;

// Buzzer
const int buzzerPin = 5;

byte mac[] = { 0xDE,0xAD,0xBE,0xEF,0xFE,0xED };
EthernetClient ethClient;
// Broker público Mosquitto (exemplo)
const char* mqtt_server = "test.mosquitto.org"; 
const int mqtt_port = 1883;
PubSubClient client(ethClient);

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed Ethernet");
    while(true);
  }
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);

  reconnect();
}

long readUltrasonicCM() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000); // timeout 30ms
  long cm = duration / 29 / 2;
  if (duration == 0) return -1; // out of range
  return cm;
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (unsigned int i=0; i<length; i++) msg += (char)payload[i];
  Serial.print("Recebido no topico: "); Serial.println(topic);
  Serial.print("Msg: "); Serial.println(msg);

  if (String(topic) == "gX/buzzer") {
    msg.toUpperCase();
    if (msg == "ON") {
      digitalWrite(buzzerPin, HIGH);
    } else if (msg == "OFF") {
      digitalWrite(buzzerPin, LOW);
    } else {
      // se for número (ms): toca por esse tempo
      int t = msg.toInt();
      if (t > 0) {
        digitalWrite(buzzerPin, HIGH);
        delay(t);
        digitalWrite(buzzerPin, LOW);
      }
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando MQTT...");
    if (client.connect("arduinoClient")) {
      Serial.println("conectado");
      client.subscribe("gX/buzzer");
    } else {
      Serial.print("falha, rc=");
      Serial.print(client.state());
      Serial.println(" tenta de novo em 5s");
      delay(5000);
    }
  }
}

unsigned long lastPublish = 0;
void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  unsigned long now = millis();
  if (now - lastPublish > 2000) { // publica a cada 2s
    lastPublish = now;
    long cm = readUltrasonicCM();
    String payload;
    if (cm == -1) payload = String("out_of_range");
    else payload = String(cm);
    client.publish("gX/distancia", payload.c_str());
    Serial.print("Publicado: "); Serial.println(payload);
  }
}

Como configurar o app IoT MQTT Panel (Android) — passo a passo

Instale o app IoT MQTT Panel da Play Store.

Abra o app → menu de configurações de broker:

Broker address = test.mosquitto.org

Port = 1883

Client ID = qualquer (ex.: panelClient1)

Não usar username/password para este broker público.

Clique Connect — verá "Connected" se ok.

Criar widget para receber distâncias (subscriber):

Adicione novo widget do tipo Text ou Gauge.

Em Topic coloque gX/distancia.

Salve; o widget será atualizado sempre que Arduino publicar.

Criar botão para controlar buzzer (publisher):

Adicione novo widget do tipo Button.

Em Topic coloque gX/buzzer.

Defina Payload ON = ON e Payload OFF = OFF.

Salve.

Teste:

No app, observe valores chegando no widget gX/distancia.

Pressione botão para enviar ON/OFF. Arduino (subscriber) receberá e acionará buzzer.

Se você usar um broker local (Mosquitto), coloque IP e porta correspondentes no app.

4) Controle de Servo via Bluetooth HC-05 (comandos "ang") — Serial Bluetooth Terminal
Componentes

Arduino Uno

Módulo HC-05 (Bluetooth)

Servo motor (ex.: SG90)

Fonte (se servo consumir mais que USB, use fonte externa 5V)

Cabos jumpers

Fiação

Servo: Vcc → 5V (ou fonte 5V externa comum), GND → GND (compartilhado com Arduino), Signal → D9 (PWM)

HC-05:

Vcc → 5V

GND → GND

TXD → Arduino D10 (uso SoftwareSerial RX)

RXD → Arduino D11 (uso SoftwareSerial TX) ATENÇÃO: RX do HC-05 espera 3.3V; o Arduino TX dá 5V. Recomenda-se um divisor de tensão (2 resistores) ou um conversor de nível para proteger HC-05 RX.

KEY/EN não é necessário para operação padrão.

Biblioteca

Servo.h

SoftwareSerial.h

Protocolo de comando

Envie strings via Bluetooth no formato: ang 90 (sem aspas) → move servo para 90°

Aceita ângulos 0 a 180

Código Arduino
#include <Servo.h>
#include <SoftwareSerial.h>

Servo myservo;
SoftwareSerial bt(10, 11); // RX, TX (Arduino RX->10 from BT TX, Arduino TX->11 to BT RX)

void setup() {
  Serial.begin(9600);
  bt.begin(9600); // padrão HC-05
  myservo.attach(9);
  myservo.write(90); // posição inicial
  Serial.println("Pronto. Conecte pelo app Serial Bluetooth Terminal.");
  bt.println("Pronto");
}

String input = "";

void loop() {
  // Lê dados do bluetooth
  while (bt.available()) {
    char c = bt.read();
    if (c == '\n' || c == '\r') {
      if (input.length() > 0) {
        processCommand(input);
        input = "";
      }
    } else {
      input += c;
    }
  }
  // opcional: também aceitar via USB serial
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (input.length() > 0) {
        processCommand(input);
        input = "";
      }
    } else input += c;
  }
}

void processCommand(String cmd) {
  cmd.trim();
  cmd.toLowerCase();
  Serial.print("Recebido: "); Serial.println(cmd);
  bt.print("Recebido: "); bt.println(cmd);

  if (cmd.startsWith("ang")) {
    int sp = cmd.indexOf(' ');
    if (sp == -1) {
      bt.println("Erro: formato 'ang <0-180>'");
      return;
    }
    String num = cmd.substring(sp+1);
    int ang = num.toInt();
    if (ang < 0) ang = 0;
    if (ang > 180) ang = 180;
    myservo.write(ang);
    bt.print("Servo -> ");
    bt.println(ang);
    Serial.print("Servo pos: "); Serial.println(ang);
  } else {
    bt.println("Comando desconhecido");
  }
}

Teste com app Serial Bluetooth Terminal (Android)

No Android, instale Serial Bluetooth Terminal.

Emparelhe com o HC-05 (senha padrão 1234 ou 0000).

Abra conexão no app com o HC-05.

No app, envie ang 45 → servo deve ir para 45°. Envie ang 180 etc.

Observações, dicas e troubleshooting geral
Sobre o proxy HTTPS (RFID)

Se você não puder rodar o proxy em PC, outra opção é hospedar um pequeno servidor (Heroku, Glitch) que aceite HTTP e faça proxy para o endpoint HTTPS. No entanto, rodar localmente é mais simples para testes.

Sobre power para servos e buzzer

Servos exigem corrente; não alimente múltiplos servos pelo 5V do Arduino se consumir muito. Use fonte externa 5V comum e conecte GND em comum.

Sobre bibliotecas e instalação

Abra Arduino IDE → Sketch → Include Library → Manage Libraries → procure por MFRC522, PubSubClient, etc., e instale.

Para Ethernet Shield W5100 use biblioteca padrão Ethernet.h. Se tiver W5200 ou ENC28J60, ajuste.

Mensagens de erro comuns

Ethernet.begin falha → verifique cabo RJ45, DHCP no roteador, MAC duplicado.

HC-SR04 leitura 0 → verifique conexão e timeout; sensor pode precisar de alimentação estável.

MQTT não conecta → verifique broker e porta; teste com MQTT Explorer ou mosquitto_sub.
