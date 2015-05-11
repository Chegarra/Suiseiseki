#include <SPI.h>
#include <Ethernet.h>
#include <dht.h>

dht DHT;

#define DHT22_PIN 34
#define pinLedV 36
#define pinLedR 40
#define pinLedA 38
#define pinRele1 30
#define pinRele2 32
#define pinLuz A8

struct
{
uint32_t total;
uint32_t ok;
uint32_t crc_error;
uint32_t time_out;
uint32_t connect;
uint32_t ack_l;
uint32_t ack_h;
uint32_t unknown;
} stat = { 0,0,0,0,0,0,0,0};

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 177);

EthernetServer server(80);

boolean error = false;

float temperaturaActual = 0.0;
float temperaturaMax = 0.0;
float temperaturaMin = 100.0;

float humedadActual = 0.0;
float humedadMax = 0.0;
float humedadMin = 100.0;

int luminosidad = 0;

boolean currentLineIsBlank = true;
boolean peticion = true;
String peticionGet = "";

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  
  Ethernet.begin(mac, ip);
  server.begin();
  
  Serial.print("Servidor en ");
  Serial.println(Ethernet.localIP());
  
  pinMode(pinLedA, OUTPUT);
  pinMode(pinLedR, OUTPUT);
  pinMode(pinLedV, OUTPUT);
  digitalWrite(pinLedA, LOW);
  digitalWrite(pinLedR, LOW);
  digitalWrite(pinLedV, LOW);
  
  pinMode(pinRele1, OUTPUT);
  pinMode(pinRele2, OUTPUT);
  digitalWrite(pinRele1, LOW);
  digitalWrite(pinRele2, LOW);
}

String splitPath(String mensaje, boolean sensores) {
  String msg = mensaje;
  int inicio = msg.indexOf(' ');
  if (inicio != -1) {
    msg = msg.substring(inicio+1);
    inicio = msg.indexOf(' ');
    msg = msg.substring(1, inicio);
  } else if (!sensores) {
    inicio = msg.indexOf('/');
    msg = msg.substring(0 ,inicio);
  } else if (sensores) {   
    inicio = msg.indexOf('/');
    msg = msg.substring(inicio+1, msg.length());
    inicio = msg.indexOf('/');
    if (inicio == -1) {
      msg = msg.substring(0 ,inicio);    
    }
  }
  return msg; 
}

void obtenerDatoLuminosidad() {
  luminosidad = analogRead(pinLuz);
}

void obtenerDatosTempHum() {
  int chk = DHT.read22((DHT22_PIN));
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
      Serial.print("Error en el sistema");
      break;
   }
   delay(500);
}

void loop() {
  // listen for incoming clients
  
  obtenerDatoLuminosidad();
  obtenerDatosTempHum();
  
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    peticion = true;
    peticionGet = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        if (c != '\n' && peticion) {
          peticionGet += c;
        } else if (c == '\n' && peticion) {
          peticion = false;
        }
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header

          String ruta = splitPath(peticionGet, false);
          String accion = splitPath(ruta, false);
          String sensor = splitPath(ruta, true);
        
          Serial.println(peticionGet + " " + ruta + " " + accion + " " + sensor);
          
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/plain");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          if (accion.equals("reles")) {
            client.println("Accion reles");
          } else if (accion.equals("sensores")) {
            if (sensor.equals("lum")) {
                  client.print("{'luminosidad':");
                  client.print(luminosidad);
                  client.println("}");
            } else if (sensor.equals("temp1")) {
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
            } else if (sensor.equals("hum1")) {
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

