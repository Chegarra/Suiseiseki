//INCLUDES
#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>
#include <dht.h> // Temperatura

// DEFINICIÓN DE PINS CONECTADOS
#define senTemp1 24    // Sensor DHT 1
#define senTemp2 25    // Sensor DHT 2
#define senLum A8      // Fotoresistencia 1
#define rele1 32       // Rele 1 control de Ventilador
#define rele2 33       // Rele 2 control de Riego
#define led1 40        // Led Rojo
#define led2 41        // Led Amarillo
#define led3 42        // Led Verde
/*
#define senTemp1 34
#define senTemp2 35
#define senLum A9
#define rele1 30
#define rele2 32
#define led1 36
#define led2 38
#define led3 40
*/

// CONSTANTES DE HUMBRALES
const float T_MIN = 10.00;
const float T_MAX = 24.00;

const float H_MIN = 30.00;
const float H_MAX = 70.00;


dht DHT;

  // TEMPERATURA
struct TEMP
{
float temperaturaActual;
float temperaturaMax;
float temperaturaMin;
} temp1 = { 0.0,0.0,100}, temp2 = { 0.0,0.0,100};

  // HUMEDAD
struct HUM
{
float humedadActual;
float humedadMax;
float humedadMin;
} hum1 = { 0.0,0.0,100}, hum2 = { 0.0,0.0,100};

  // LUMINOSIDAD
struct LUM
{
int lumActual;
int lumMax;
int lumMin;
} lum1 = {0,0,100};


  //Estados
boolean estadoRele1 = false;
boolean estadoRele2 = false;
boolean estadoSensor = true;
boolean estadoT = true;
boolean estadoH = true;

  // ETHERNET (MAC, IP y PUERTO)
byte mac[] = {0xFA, 0x15, 0xAD, 0x15, 0xCF, 0x07};
IPAddress ip(192, 168, 1, 177); //CASA
//IPAddress ip(158, 42, 181, 60); //CLASE
EthernetServer server(80);

  // LCD
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
 
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  
  Serial.println("server is at " + Ethernet.localIP());
  
  pinMode(rele1, OUTPUT);
  pinMode(rele2, OUTPUT);
  digitalWrite(rele1, LOW);
  digitalWrite(rele2, LOW);
  
  pinMode(led1,OUTPUT);
  pinMode(led2,OUTPUT);
  pinMode(led3,OUTPUT);
  
  lcd.begin(16, 2);
}

void obtenerDatosLuz(int sensor, struct LUM *l) {
  l->lumActual = analogRead(sensor);
  
  if (l->lumActual < l->lumMin)
  {
    l->lumMin = l->lumActual;
  }
  else if ( l->lumActual > l->lumMax)
  {
    l->lumMax = l->lumActual;
  }
}
  
void obtenerDatosDHT(int sensor, struct TEMP *t, struct HUM *h )
{
  int chk = DHT.read11(sensor);
  //int chk = DHT.read22(&sensor);
  
  switch (chk) 
  {
    case DHTLIB_OK:
      t->temperaturaActual = DHT.temperature;
      
      if (t->temperaturaActual < t->temperaturaMin)
      {
        t->temperaturaMin = t->temperaturaActual;
      }
      else if (t->temperaturaActual > t->temperaturaMax)
      {
        t->temperaturaMax = t->temperaturaActual;
      }
      
      if (T_MIN < t->temperaturaActual  && t->temperaturaActual < T_MAX)
      {
        estadoT = true;
      }
      else {
        estadoT = false;
      }     
      
      h->humedadActual = DHT.humidity;
      if (h->humedadActual < h->humedadMin) 
      {
        h->humedadMin = h->humedadActual;
      } 
      else if (h->humedadActual > h->humedadMax) 
      {
        h->humedadMax = h->humedadActual;
      }
        
      if (H_MIN < h->humedadActual && h->humedadActual < H_MAX)
      {
        estadoH = true;
      }
      else {
        estadoH = true;
      }
      
      estadoSensor = true;
      break;
      
    case DHTLIB_ERROR_CHECKSUM:
    case DHTLIB_ERROR_TIMEOUT:
    case DHTLIB_ERROR_CONNECT:
    case DHTLIB_ERROR_ACK_L:
    case DHTLIB_ERROR_ACK_H:
    default:
      //Serial.println( "SENSOR TEMPERATURA: Error del sistema");
      estadoSensor = false;
      break;
  }
  delay(100);
}

void estadoSistema(int rojo, int amarillo, int verde)
{
  
  if (estadoSensor)
  {
    digitalWrite(verde,HIGH);
    digitalWrite(rojo, LOW);
  }
  else 
  {
    digitalWrite(verde, LOW);
    digitalWrite(rojo, HIGH);
  }
  
  if (estadoRele1 || estadoRele2)
  {
    digitalWrite(amarillo, HIGH);
  }
  else
  {
    digitalWrite(amarillo, LOW);
  }
  
  if ( (estadoT && estadoH)  && estadoSensor)
  {
    digitalWrite(rojo, LOW);
  }
  else if ( (estadoT && estadoH) && !estadoSensor)
  {
    digitalWrite(rojo, HIGH);
  }

      
  
}

void loop() {
  
  obtenerDatosDHT(senTemp1, &temp1, &hum1);
  obtenerDatosDHT(senTemp2, &temp2, &hum2);
  obtenerDatosLuz(senLum, &lum1);
  
  estadoSistema(led1,led2,led3);
  
  lcd.setCursor(0,0);
  lcd.print("T1: " + String(temp1.temperaturaActual));
  lcd.setCursor(0,1);
  lcd.print("T2: " + String(temp2.temperaturaActual));
  // listen for incoming clients
  
  /*
  SI  Son las 8.00AM
    -> Resetear MAX y MIN de los sensores
  SI  La hora es 
  SI  Temperatura supera el Maximo Permitido:
    -> Conectar Ventilador
  SI  Temperatura disminuye 2 grados del Maximo Permitido
    -> Desconectar Ventilador
  SI  Humedad esta por debajo del Minimo Permitido
    -> Conectar Riego
  SI  Humedad sube 5 puntos porcentuales desconectar riego
  
  
  */
  
  
  
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
          /*client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/plain");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          client.println();*/
          if (mensaje.indexOf("/reles") != -1) {
            
            if (mensaje.indexOf("/rele1") != -1) {
              if (mensaje.indexOf("/on") != -1) {
                digitalWrite(rele1, HIGH);
                estadoRele1 = true;
              } else if (mensaje.indexOf("/off") != -1) {
                digitalWrite(rele1, LOW);
                estadoRele1 = false;
              } 
              
              client.print("{\"rele1\":");
              if (estadoRele1) {
                client.print("\"ON\"");
              } else {
                client.print("\"OFF\"");
              } 
              client.println("}");
              
            } else if (mensaje.indexOf("/rele2") != -1) {
              if (mensaje.indexOf("/on") != -1) {
                digitalWrite(rele2, HIGH);
                estadoRele2 = true;
              } else if (mensaje.indexOf("/off") != -1) {
                digitalWrite(rele2, LOW);
                estadoRele2 = false;
              }
              
              client.print("{\"rele2\":");
              if (estadoRele2) {
                client.print("\"ON\"");
              } else {
                client.print("\"OFF\"");
              } 
              client.println("}");
            }            
          } else if (mensaje.indexOf("/sensores") != -1) {
            if (mensaje.indexOf("/lum1") != -1) {                 
                  client.print("{\"sensorLum1\": {");
                  client.print("\"lumActual\":");
                  client.print(lum1.lumActual);
                  client.print(",");
                  client.print("\"lumMax\":");
                  client.print(lum1.lumMax);
                  client.print(",");
                  client.print("\"lumMin\":");
                  client.print(lum1.lumMin);
                  client.println("}}");
            } else if (mensaje.indexOf("/temp1") != -1) {
                 client.print("{\"sensorTemp1\": {");
                  client.print("\"tempActual\":");
                  client.print(temp1.temperaturaActual);
                  client.print(",");
                  client.print("\"tempMax\":");
                  client.print(temp1.temperaturaMax);
                  client.print(",");
                  client.print("\"tempMin\":");
                  client.print(temp1.temperaturaMin);
                  client.println("}}");
             } else if (mensaje.indexOf("/temp2") != -1) {
                  client.print("{\"sensorTemp2\": {");
                  client.print("\"tempActual\":");
                  client.print(temp2.temperaturaActual);
                  client.print(",");
                  client.print("\"tempMax\":");
                  client.print(temp2.temperaturaMax);
                  client.print(",");
                  client.print("\"tempMin\":");
                  client.print(temp2.temperaturaMin);
                  client.println("}}");
            } else if (mensaje.indexOf("/hum1") != -1) {
                  client.print("{\"sensorHum1\": {");
                  client.print("\"humActual\":");
                  client.print(hum1.humedadActual);
                  client.print(",");
                  client.print("\"humMax\":");
                  client.print(hum1.humedadMax);
                  client.print(",");
                  client.print("\"humMin\":");
                  client.print(hum1.humedadMin);
                  client.println("}}");  
            } else if (mensaje.indexOf("/hum2") != -1) {
                  client.print("{\"sensorHum2\": {");
                  client.print("\"humActual\":");
                  client.print(hum1.humedadActual);
                  client.print(",");
                  client.print("\"humMax\":");
                  client.print(hum1.humedadMax);
                  client.print(",");
                  client.print("\"humMin\":");
                  client.print(hum1.humedadMin);
                  client.println("}}");                         
            }
          } else {
            client.print("{\"error\": \"ruta incorrecta\"}");
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
    delay(100);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}
