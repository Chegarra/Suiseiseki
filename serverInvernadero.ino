//INCLUDES
#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>
#include <dht.h> // Temperatura

// DEFINICIÓN DE PINS CONECTADOS
#define senTemp1 24
#define senTemp2 25
#define senLum A8
#define rele1 32
#define rele2 33
#define led1 40
#define led2 41
#define led3 42
#define DHT22_PIN 34 // Sensor Temp y Humedad


// VARIABLES INICIALES
  // TEMPERATURA
float temperaturaActual = 0.0;
float temperaturaMax = 0.0;
float temperaturaMin = 100.0;
  // HUMEDAD
float humedadActual = 0.0;
float humedadMax = 0.0;
float humedadMin = 100.0;
  // LUMINOSIDAD
int luminosidad = 0;
  // ETHERNET (MAC, IP y PUERTO)
byte mac[] = {0xFA, 0x15, 0xAD, 0x15, 0xCF, 0x07};
IPAddress ip(158, 42, 181, 60);
EthernetServer server(80);
  // LCD
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
  // TEMPERATURA
dht DHT;
boolean error = false;



void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  pinMode(rele1,OUTPUT);
  pinMode(rele2,OUTPUT);
  
  pinMode(led1,OUTPUT);
  pinMode(led2,OUTPUT);
  pinMode(led3,OUTPUT);
  lcd.begin(16, 2);
}

void obtenerDatosTempHum(int sensor) {
  int chk = DHT.read11(sensor);
  switch (chk) {
    case DHTLIB_OK:
      humedadActual = DHT.humidity;
      if (humedadActual < humedadMin) {
        humedadMin = humedadActual;
      } else if (humedadActual > humedadMax) {
        humedadMax = humedadActual;
      }
      
      temperaturaActual = DHT.temperature;
      if (temperaturaActual < temperaturaMin) {
        temperaturaMin = temperaturaActual;
      } else if (temperaturaActual > temperaturaMax) {
        temperaturaMax = temperaturaActual;
      }
     
      break;
    case DHTLIB_ERROR_CHECKSUM:
    case DHTLIB_ERROR_TIMEOUT:
    case DHTLIB_ERROR_CONNECT:
    case DHTLIB_ERROR_ACK_L:
    case DHTLIB_ERROR_ACK_H:
    default:
      error = true;
      Serial.println( "SENSOR TEMPERATURA ( " + String(sensor) + " ): Error del sistema");
      break;
   }
   delay(500);
}



void loop() {
  obtenerDatosTempHum(senTemp1);
  lcd.setCursor(0,0);
  lcd.print("T1: " + String(temperaturaActual));
  obtenerDatosTempHum(senTemp2);
  lcd.setCursor(0,1);
  lcd.print("H1: " + String(humedadActual));
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    String mensaje = "";
    boolean peticion = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);

        if (c != '\n' && peticion) {
          mensaje += c;
        } else if (c == '\n' && peticion) {
          peticion = false;
        }
        
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          //client.println("HTTP/1.1 200 OK");
          //client.println("Content-Type:text/plain");
          //client.println("Access-Control-Allow-Origin: http://158.42.181.60:80");
          //client.println("Access-Control-Allow-Credentials: true");
          //client.println("Access-Control-Expose-Headers: sensores, reles");
          //client.println("Connection: close");  // the connection will be closed after completion of the response
          if (mensaje.indexOf("/reles") != -1) {
            client.println("Accion reles");
            if (mensaje.indexOf("/rele1")  != -1) {
              client.print("reles1");
            }
            else if (mensaje.indexOf("/rele2") != -1) {
              client.print("rele2");
            }            
          } else if (mensaje.indexOf("/sensores")  != -1) {
            if (mensaje.indexOf("lum")  != -1) {
                  client.print("{'luminosidad':");
                  client.print(luminosidad);
                  client.println("}");
            } else if (mensaje.indexOf("/temp1")  != -1) {
                  client.print("{'sensorTemp1':{");
                  client.print("'tempActual':");
                  client.print(temperaturaActual);
                  client.print(",");
                  client.print("'tempMax':");
                  client.print(temperaturaMax);
                  client.print(",");
                  client.print("'tempMin':");
                  client.print(temperaturaMin);
                  client.println("}}");
             } else if (mensaje.indexOf("/temp2")  != -1) {
                  client.print("{'sensorTemp1':{");
                  client.print("'tempActual':");
                  client.print(temperaturaActual);
                  client.print(",");
                  client.print("'tempMax':");
                  client.print(temperaturaMax);
                  client.print(",");
                  client.print("'tempMin':");
                  client.print(temperaturaMin);
                  client.println("}}");
            } else if (mensaje.indexOf("/hum1") != -1) {
                  client.print("{'sensorHum1':{");
                  client.print("'humActual':");
                  client.print(humedadActual);
                  client.print(",");
                  client.print("'humMax':");
                  client.print(humedadMax);
                  client.print(",");
                  client.print("'humMin':");
                  client.print(humedadMin);
                  client.println("}}");  
            } else if (mensaje.indexOf("/hum2") != -1) {
                  client.print("{'sensorHum1':{");
                  client.print("'humActual':");
                  client.print(humedadActual);
                  client.print(",");
                  client.print("'humMax':");
                  client.print(humedadMax);
                  client.print(",");
                  client.print("'humMin':");
                  client.print(humedadMin);
                  client.println("}}");                       
            }
          }
          else {
                  client.println("INICIO");
                 }
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

