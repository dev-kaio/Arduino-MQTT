#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>

// LCD nos pinos digitais
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// MAC Address único
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// Servidor web na porta 80
EthernetServer server(80);

String receivedText = "";

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.print("Inicializando...");
  
  // Inicia Ethernet via DHCP
  if (Ethernet.begin(mac) == 0) {
    lcd.clear();
    lcd.print("DHCP falhou!");
    Serial.println("DHCP falhou!");
    while(true); // trava aqui se falhar
  }

  // Inicia servidor
  server.begin();
  
  // Mostra IP no LCD
  lcd.clear();
  lcd.print("IP:");
  lcd.setCursor(0, 1);
  lcd.print(Ethernet.localIP());
  Serial.print("IP obtido: ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  EthernetClient client = server.available();
  if (client) {
    Serial.println("Cliente conectado");
    String request = "";
    bool currentLineIsBlank = true;

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request += c;
        Serial.write(c);

        // Fim do cabeçalho HTTP
        if (c == '\n' && currentLineIsBlank) {
          
          int pos = request.indexOf("msg=");
          if (pos > 0) {
            receivedText = request.substring(pos + 4);
            receivedText.replace("+", " "); // substitui espaços
            int fim = receivedText.indexOf(" ");
            if (fim > 0) receivedText = receivedText.substring(0, fim);

            // Mostra no LCD
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Recebido:");
            lcd.setCursor(0,1);
            lcd.print(receivedText.substring(0,16));
            Serial.print("Mensagem recebida: ");
            Serial.println(receivedText);
          }

          // --- ENVIA PÁGINA HTML ---
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();
          client.println("<html><body>");
          client.println("<h2>Enviar mensagem ao Arduino</h2>");
          client.println("<form action='/' method='GET'>");
          client.println("Mensagem: <input type='text' name='msg'>");
          client.println("<input type='submit' value='Enviar'>");
          client.println("</form>");
          client.print("<p>Ultima mensagem: ");
          client.print(receivedText);
          client.println("</p>");
          client.println("</body></html>");
          break;
        }

        if (c == '\n') currentLineIsBlank = true;
        else if (c != '\r') currentLineIsBlank = false;
      }
    }
  }
}
