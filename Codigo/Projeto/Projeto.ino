#include <WiFi.h>
#include <HomeSpan.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "DEV_LED.h"
#include "DEV_TempSensor.h"

// Definir o nome da rede Wi-Fi e senha
/*const char *ssid = "MEO-96D1F0";
const char *password = "39befe4f0f";*/

const char *ssid = "MEO-89AC60";
const char *password = "0b21fc321b";

int   despesa     = 0;
float temperatura = 0.0;
int   bloquear    = 0;
bool  buttonState = false;
int   nbutton     = 1;

#define TFT_CS     5
#define led_pin    12     // LED vermelho
#define relay_pin1 13
#define relay_pin2 14
#define relay_pin3 15
#define TFT_DC     16
#define TFT_RST    17
// O PINO 18 ESTA EM CONECTADO AO SCK DO TFT!!!!
#define BUTTON_PIN 21
#define sensor_pin 36     // sensor temperatura

WiFiServer servidor(80);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

DEV_LED* f1;
DEV_LED* f2;
DEV_LED* f3;
DEV_TempSensor* t;

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  delay(100);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado à rede Wi-Fi");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  servidor.begin();

  homeSpan.setPortNum(1201);
  // colocar ligaçao ao Wifi

  homeSpan.begin(Category::Bridges,"HomeSpan");

  new SpanAccessory();
    new Service::AccessoryInformation();
      new Characteristic::Identify();
    
  new SpanAccessory();
    new Service::AccessoryInformation();
      new Characteristic::Identify();
      new Characteristic::Name("Ficha 1");
    f1 = new DEV_LED(relay_pin1, bloquear);

  new SpanAccessory();
    new Service::AccessoryInformation();
      new Characteristic::Identify();
      new Characteristic::Name("Ficha 2");
    f2 = new DEV_LED(relay_pin2, bloquear);
  
  new SpanAccessory();
    new Service::AccessoryInformation();
      new Characteristic::Identify();
      new Characteristic::Name("Ficha 3");
    f3 = new DEV_LED(relay_pin3, bloquear);

  new SpanAccessory();
    new Service::AccessoryInformation();
      new Characteristic::Identify();
      new Characteristic::Name("Temperatura");
    t = new DEV_TempSensor(sensor_pin, temperatura);

  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST77XX_WHITE);
  tft.setTextColor(ST77XX_BLACK);
  tft.setRotation(1);

  // Para começar ativa  
  f1->atualizar_estado(1);
  f2->atualizar_estado(1);
  f3->atualizar_estado(1);

}


void loop() {
  delay(100);

  homeSpan.poll();

  buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW) {
    nbutton = nbutton + 1;
    if (nbutton > 3) {
      nbutton = 1;
    }
  }

  temperatura = (analogRead(sensor_pin) * 0.001221 * 100);
  t->atualizar_temp(temperatura);
  if(temperatura > 70.0) {
    bloquear = 1;
  } else {
    bloquear = 0;
  }

  WiFiClient cliente = servidor.available();

  if (cliente) {
    String requisicao = "";

    while (cliente.connected()) {
      if (cliente.available()) {
        char caractere = cliente.read();
        requisicao += caractere;
        int numIndex = requisicao.indexOf("number=");

        if (caractere == '\n') {
          if (requisicao.indexOf("1-ON") != -1) {
            f1_on();
          } else if (requisicao.indexOf("1-OFF") != -1) {
            f1_off();
          } else if (requisicao.indexOf("2-ON") != -1) {
            f2_on();
          } else if (requisicao.indexOf("2-OFF") != -1) {
            f2_off();
          } else if (requisicao.indexOf("3-ON") != -1) {
            f3_on();
          } else if (requisicao.indexOf("3-OFF") != -1) {
            f3_off();
          } else if (numIndex != -1) {
            despesa = requisicao.substring(numIndex + 7).toFloat();
          }

          cliente.println("HTTP/1.1 200 OK");
          cliente.println("Content-type:text/html");
          cliente.println("");

          int sec = millis() / 1000;
          int min = sec / 60;
          int hr = min / 60;
          int preco = 200;

          cliente.println(
            "<html>\
            <head>\
              <meta charset='UTF-8' http-equiv='refresh' content='30; url=http://");
          cliente.print(WiFi.localIP());
          cliente.print("/'>\
              <title>Controlo das fichas</title>\
              <style>\
                body {font-family: Arial, Helvetica, sans-serif;margin: 0;padding: 0;}\
                h1 {text-align: center;margin-top: 30px;}\
                .container {display: flex;flex-wrap: wrap;justify-content: center;margin-top: 50px;}\
                .card {background-color: #f1f1f1;margin: 20px;padding: 20px;border-radius: 5px;box-shadow: 0px 0px 5px #888;width: 300px;}\
                .card a {display: inline-block;padding: 10px 20px;background-color: #808080;color: white;text-align: center;font-size: 16px;border-radius: 5px;text-decoration: none;}\                
              </style>\
            </head>\
            <body>\
              <h1>Controlo das fichas</h1>\
              <div class='container'>\
                <div class='card'>\
                  <h2>Ficha 1</h2>\
                  <ul> <li>Corrente: 10 A</li> <li>Tensao: 10 V</li> <li>Potencia: 10 W</li> </ul>\
                  <a href=\"/1-ON\">ON</a>\
                  <a href=\"/1-OFF\">OFF</a>");
          if(f1->estado_atual() == 1) {
            //LIGADA
            cliente.print("<div style='display: inline-block;padding: 10px 20px; vertical-align: middle;border-radius: 5px;width: 28px;height: 18px;margin-left: 10px;background-color: #00ff00'></div>");
          } else if (f1->estado_atual() == 0) {
            // DESLIGADA
            cliente.print("<div style='display: inline-block;padding: 10px 20px; vertical-align: middle;border-radius: 5px;width: 28px;height: 18px;margin-left: 10px;background-color: #ff0000'></div>");
          }                  
          cliente.println("</div>\
                <div class='card'>\
                  <h2>Ficha 2</h2>\
                  <ul> <li>Corrente: 10 A</li> <li>Tensao: 10 V</li> <li>Potencia: 10 W</li> </ul>\
                  <a href=\"/2-ON\">ON</a>\
                  <a href=\"/2-OFF\">OFF</a>");
          if(f2->estado_atual() == 1) {
            //LIGADA
            cliente.print("<div style='display: inline-block;padding: 10px 20px; vertical-align: middle;border-radius: 5px;width: 28px;height: 18px;margin-left: 10px;background-color: #00ff00'></div>");
          } else if (f2->estado_atual() == 0) {
            // DESLIGADA
            cliente.print("<div style='display: inline-block;padding: 10px 20px; vertical-align: middle;border-radius: 5px;width: 28px;height: 18px;margin-left: 10px;background-color: #ff0000'></div>");
          }                  
          cliente.println("</div>\
                <div class='card'>\
                  <h2>Ficha 3</h2>\
                  <ul> <li>Corrente: 10 A</li> <li>Tensao: 10 V</li> <li>Potencia: 10 W</li> </ul>\
                  <a href=\"/3-ON\">ON</a>\
                  <a href=\"/3-OFF\">OFF</a>");
          if(f3->estado_atual() == 1) {
            //LIGADA
            cliente.print("<div style='display: inline-block;padding: 10px 20px; vertical-align: middle;border-radius: 5px;width: 28px;height: 18px;margin-left: 10px;background-color: #00ff00'></div>");
          } else if (f3->estado_atual() == 0) {
            // DESLIGADA
            cliente.print("<div style='display: inline-block;padding: 10px 20px; vertical-align: middle;border-radius: 5px;width: 28px;height: 18px;margin-left: 10px;background-color: #ff0000'></div>");
          }                  
          cliente.println("</div>\
                <p><br></p>\
                <div class='card'>\
                  <p>Tempo decorrido desde a ativação:<br>");
          cliente.println(hr);
          cliente.println(":");
          cliente.println(min%60);
          cliente.println(":");
          cliente.println(sec%60);
          cliente.println("</p>\
                <form>\
                  <input type=\'number\' name=\'number\'placeholder=\'");
          cliente.print(despesa);
          cliente.println("\'>\
                  <input type=\"submit\">\
                </form>\
                  <p>O valor a pagar é:");
          cliente.println(preco);
          cliente.println("</p>\
                </div>\
              </div>\
            </body>\
            </html>"
          );
          break;
        }
      }
    }

    cliente.stop();
  }

  if (nbutton == 1) {
    // Controlo de ficha 1
    tft.fillScreen(ST77XX_WHITE);
    tft.setCursor(5, 5);
    tft.setTextSize(2);
    tft.println("Ficha 1");
    tft.println("");
    tft.setTextSize(1);
    tft.println("");
    tft.print("  * Tensao: ");
    int tensao = 10;
    tft.print(tensao);
    tft.println(" V");
    tft.print("  * Corrente: ");
    int corrente = 10;
    tft.print(corrente);
    tft.println(" A");
    tft.print("  * Potencia: ");
    int potencia = 10;
    tft.print(potencia);
    tft.println(" W");
    tft.println("");
    tft.println(" IP adress:");
    tft.print("   ");
    tft.println(WiFi.localIP());
    if (f1->estado_atual() == 1) {
      // FICHA LIGADA
      tft.fillRect(52, 104, 32, 12, ST77XX_GREEN);
    } else if (f1->estado_atual() == 0) {
      // FICHA DESLIGADA
      tft.fillRect(52, 104, 32, 12, ST77XX_BLUE);
    }
  }
  else if (nbutton == 2) {
    // Controlo de ficha 2
    tft.fillScreen(ST77XX_WHITE);
    tft.setCursor(5, 5);
    tft.setTextSize(2);
    tft.println("Ficha 2");
    tft.println("");
    tft.setTextSize(1);
    tft.println("");
    tft.print("  * Tensao: ");
    int tensao = 10;
    tft.print(tensao);
    tft.println(" V");
    tft.print("  * Corrente: ");
    int corrente = 10;
    tft.print(corrente);
    tft.println(" A");
    tft.print("  * Potencia: ");
    int potencia = 10;
    tft.print(potencia);
    tft.println(" W");
    tft.println("");
    tft.println(" IP adress:");
    tft.print("   ");
    tft.println(WiFi.localIP());
    if (f2->estado_atual() == 1) {
      // FICHA LIGADA
      tft.fillRect(52, 104, 32, 12, ST77XX_GREEN);
    } else if (f2->estado_atual() == 0) {
      // FICHA DESLIGADA
      tft.fillRect(52, 104, 32, 12, ST77XX_BLUE);
    }
  } else if (nbutton == 3) {
    // Controlo de ficha 3
    tft.fillScreen(ST77XX_WHITE);
    tft.setCursor(5, 5);
    tft.setTextSize(2);
    tft.println("Ficha 3");
    tft.println("");
    tft.setTextSize(1);
    tft.println("");
    tft.print("  * Tensao: ");
    int tensao = 10;
    tft.print(tensao);
    tft.println(" V");
    tft.print("  * Corrente: ");
    int corrente = 10;
    tft.print(corrente);
    tft.println(" A");
    tft.print("  * Potencia: ");
    int potencia = 10;
    tft.print(potencia);
    tft.println(" W");
    tft.println("");
    tft.println(" IP adress:");
    tft.print("   ");
    tft.println(WiFi.localIP());
    if (f3->estado_atual() == 1) {
      // FICHA LIGADA
      tft.fillRect(52, 104, 32, 12, ST77XX_GREEN);
    } else if (f3->estado_atual() == 0) {
      // FICHA DESLIGADA
      tft.fillRect(52, 104, 32, 12, ST77XX_BLUE);
    }
  }




}

void f1_on(){
  if(bloquear == 0){
    // AQUI VAMOS LIGAR A FICHA 1
    // sabemos que viemos para aqui pois no site o utilizar quis liga-la
    // precisamos de saber se esta ligada ou desligada na app
    int state = f1->estado_atual();
    if (state == 0) {
      // esta desligada na app
      digitalWrite(relay_pin1, HIGH);
      f1->atualizar_estado(1);
    }
  }
}

void f1_off(){
  if(bloquear == 0) {
    // AQUI VAMOS DESLIGAR A FICHA 1
    // sabemos que viemos para aqui pois no site o utilizar quis desliga-la
    // precisamos de saber se esta ligada ou desligada na app
    int state = f1->estado_atual();
    if (state == 1) {
      // esta ligada na app
      digitalWrite(relay_pin1, LOW);
      f1->atualizar_estado(0);
    }
  }
}

void f2_on(){
  if(bloquear == 0) {
    // AQUI VAMOS LIGAR A FICHA 1
    // sabemos que viemos para aqui pois no site o utilizar quis liga-la
    // precisamos de saber se esta ligada ou desligada na app
    int state = f2->estado_atual();
    if (state == 0) {
      // esta desligada na app
      digitalWrite(relay_pin2, HIGH);
      f2->atualizar_estado(1);
    }
  }
}

void f2_off(){
  if(bloquear == 0) {
    // AQUI VAMOS DESLIGAR A FICHA 1
    // sabemos que viemos para aqui pois no site o utilizar quis desliga-la
    // precisamos de saber se esta ligada ou desligada na app
    int state = f2->estado_atual();
    if (state == 1) {
      // esta ligada na app
      digitalWrite(relay_pin2, LOW);
      f2->atualizar_estado(0);
    }
  }
}

void f3_on(){
  if(bloquear == 0) {
    // AQUI VAMOS LIGAR A FICHA 1
    // sabemos que viemos para aqui pois no site o utilizar quis liga-la
    // precisamos de saber se esta ligada ou desligada na app
    int state = f3->estado_atual();
    if (state == 0) {
      // esta desligada na app
      digitalWrite(relay_pin3, HIGH);
      f3->atualizar_estado(1);
    }
  }
}

void f3_off(){
  if(bloquear == 0) {
    // AQUI VAMOS DESLIGAR A FICHA 1
    // sabemos que viemos para aqui pois no site o utilizar quis desliga-la
    // precisamos de saber se esta ligada ou desligada na app
    int state = f3->estado_atual();
    if (state == 1) {
      // esta ligada na app
      digitalWrite(relay_pin3, LOW);
      f3->atualizar_estado(0);
    }
  }
}
