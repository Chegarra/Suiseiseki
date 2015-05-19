//INCLUDES
#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>
#include <dht.h> // Temperatura
#include <Wire.h>   // Comunicación I2C
#include "RTClib.h" // Reloj RTC I2C

// DEFINICIÓN DE PINS CONECTADOS
#define senTemp1 24    // Sensor DHT 1
#define senTemp2 25    // Sensor DHT 2
#define senLum A8      // Fotoresistencia 1
#define rele1 32       // Rele 1 control de Ventilador
#define rele2 33       // Rele 2 control de Riego  
#define led1 40        // Led Rojo
#define led2 41        // Led Amarillo
#define led3 42        // Led Verde
#define buzz 50


// CONSTANTES DE HUMBRALES
const float T_MIN = 15.00;
const float T_MAX = 27.00;

const float H_MIN = 30.00;
const float H_MAX = 50.00;

//  CONSTANTES DE RIEGO
  // Fecha de Control

const int HH = 10;
const int MM = 30;

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


// VARIABLES
  // Estados
boolean estadoRele1 = false;
boolean estadoRele2 = false;
boolean estadoSensor = true;
boolean estadoT = true;
boolean estadoH = true;
boolean programado = true;
boolean alertaTemp = false;
boolean alertaHum = false;
boolean modoManual = false;
  // 

dht DHT;
RTC_DS1307 RTC;
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

byte mac[] = {0xFA, 0x15, 0xAD, 0x15, 0xCF, 0x07};
IPAddress ip(192, 168, 1, 177); //CASA
//IPAddress ip(158, 42, 181, 60); //CLASE
EthernetServer server(80);


void setup() 
{
  Wire.begin();
  RTC.begin();
  //RTC.adjust(DateTime(__DATE__, __TIME__));
  
  Serial.begin(9600);
  
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
  digitalWrite(buzz, LOW);
  
  lcd.begin(16, 2);
}


void resetearValoresLum(struct LUM *l)
{
  l->lumMax = 0.0;
  l->lumMin = 1000.00;
}

void resetearValoresTemp(struct TEMP *t)
{
  t->temperaturaMax = 0.0;
  t->temperaturaMin = 100.00;
}

void resetearValoresHum(struct HUM *h)
{
  h->humedadMax = 0.0;
  h->humedadMin = 100.00;
}

boolean comprobarUmbralTemp (struct TEMP *t1, struct TEMP *t2, int margen)
{
  if ((t1->temperaturaActual + t2->temperaturaActual) / 2.0 > (T_MAX - margen))
  {
    return true;
  }  
  return false;
}

boolean comprobarUmbralHum (struct HUM *h1, struct HUM *h2, int margen)
{
  if ((h1->humedadActual + h2->humedadActual) / 2.0 < (H_MIN + margen) )
  {
    return true;
  }  
  return false;
}

void obtenerDatosLuz(int sensor, struct LUM *l) 
{
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

void estadoManual(boolean estado)
{
  if (estado)
  {
    modoManual = true;
  }
  else {
    modoManual = false;
    alertaTemp = false;
    alertaHum = false;
    estadoRele(rele1,&estadoRele1,false);
    estadoRele(rele2,&estadoRele2,false);
  }  
}

boolean getEstadoManual()
{
  return modoManual;
}

void estadoRele(int rele, boolean *estadoRele, boolean estado)
{
  if (estado)
  {
    *estadoRele = estado;
    digitalWrite(rele, HIGH);
  }
  else
  {
    *estadoRele = estado;
    digitalWrite(rele, LOW);
  }
}
    
  
  


void loop() {
  DateTime now = RTC.now(); // Obtiene la fecha y hora del RTC
  
  obtenerDatosDHT(senTemp1, &temp1, &hum1);
  obtenerDatosDHT(senTemp2, &temp2, &hum2);
  obtenerDatosLuz(senLum, &lum1);
  
  estadoSistema(led1,led2,led3);
  
  lcd.setCursor(0,0);
  lcd.print("Temp: " + String((temp1.temperaturaActual + temp2.temperaturaActual) / 2.0));
  lcd.setCursor(0,1);
  lcd.print("Hum : " + String((hum1.humedadActual + hum2.humedadActual) / 2.0 ));
  // listen for incoming clients
  
 // A LAS 8 DE LA MAÑANA REINICIA VALORES MAX Y MIN 
  if(now.hour() == HH && now.minute() == MM)
  {
    // RESETEAR MAXIMO Y MINIMO
    if (programado)
    {
    resetearValoresLum(&lum1);
    resetearValoresTemp(&temp1);
    resetearValoresTemp(&temp2);
    resetearValoresHum(&hum1);
    resetearValoresHum(&hum2);
    
    programado = false;
    }
    
  }
  else {
    programado = true;
  }
  
  if (!modoManual)
  {
      // SI LA TEMPERATURA ES MAYOR QUE EL UMBRAL PERMITIDO
      if (comprobarUmbralTemp(&temp1, &temp2, 0) && !alertaTemp)
      {
        estadoRele(rele1,&estadoRele1,true);
        //digitalWrite(rele1, HIGH);
        //estadoRele1 = true;
        alertaTemp = true;
      }
      // HASTA QUE LA TEMPERATURA NO BAJE DEL UMBRAL + MARGEN RIEGO CONECTADO
      if (!comprobarUmbralTemp(&temp1, &temp2, 3) && alertaTemp)
      {
        estadoRele(rele1,&estadoRele1,false);
        //digitalWrite(rele1, LOW);
        //estadoRele1 = false;
        alertaTemp = false;
      }
      
        // SI LA TEMPERATURA ES MAYOR QUE EL UMBRAL PERMITIDO
      if (comprobarUmbralHum(&hum1, &hum2, 0) && !alertaHum)
      {
        estadoRele(rele2,&estadoRele2,true);
        //digitalWrite(rele2, HIGH);
        //estadoRele2 = true;
        alertaHum = true;
      }
      // HASTA QUE LA TEMPERATURA NO BAJE DEL UMBRAL + MARGEN RIEGO CONECTADO
      if (!comprobarUmbralHum(&hum1, &hum2, 5) && alertaHum)
      {
        estadoRele(rele2,&estadoRele2,false);
        //digitalWrite(rele2, LOW);
        //estadoRele2 = false;
        alertaHum = false;
      }
   }
  
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
              if (getEstadoManual() && mensaje.indexOf("/on") != -1) {
                estadoRele(rele1,&estadoRele1,true);
                //digitalWrite(rele1, HIGH);
                //ºestadoRele1 = true;
              } else if (mensaje.indexOf("/off") != -1) {
                estadoRele(rele1,&estadoRele1,false);
                //digitalWrite(rele1, LOW);
                //estadoRele1 = false;
              } 
              
              client.print("{\"rele1\":");
              if (estadoRele1) {
                client.print("\"ON\"");
              } else {
                client.print("\"OFF\"");
              } 
              client.println("}");
              
            } else if (mensaje.indexOf("/rele2") != -1) {
              if (getEstadoManual() && mensaje.indexOf("/on") != -1) {
                estadoRele(rele2,&estadoRele2,true);
                //digitalWrite(rele2, HIGH);
                //estadoRele2 = true;
              } else if (mensaje.indexOf("/off") != -1) {
                estadoRele(rele2,&estadoRele2,false);
                //digitalWrite(rele2, LOW);
                //estadoRele2 = false;
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
                  client.print(hum2.humedadActual);
                  client.print(",");
                  client.print("\"humMax\":");
                  client.print(hum2.humedadMax);
                  client.print(",");
                  client.print("\"humMin\":");
                  client.print(hum2.humedadMin);
                  client.println("}}");                         
            }
          }
         else if (mensaje.indexOf("/config") != -1)
         {
           if (mensaje.indexOf("/manual") != -1) {
              if (mensaje.indexOf("/on") != -1) {
                estadoManual(true);
              } else if (mensaje.indexOf("/off") != -1) {
                estadoManual(false);
              } 
              
              client.print("{\"modoManual\":");
              if (modoManual) {
                client.print("\"ON\"");
              } else {
                client.print("\"OFF\"");
              } 
              client.println("}");
              
            }           
         }
         else if (mensaje.indexOf("/time") != -1)
         {
           if (mensaje.indexOf("/set") != -1) 
           {
             int i = mensaje.indexOf("/set");
             String data = mensaje.substring((i+5),(i+19));
             client.println(data);
             /*
             client.println(data.substring(0,4)); // AÑO
             client.println(data.substring(4,6)); // MES
             client.println(data.substring(6,8)); // DIA
             client.println(data.substring(8,10));// HORA
             client.println(data.substring(10,12));// MINUTOS
             */
            RTC.adjust( DateTime (data.substring(0,4).toInt(), data.substring(4,6).toInt(), data.substring(6,8).toInt(), data.substring(8,10).toInt(), data.substring(10,12).toInt(), data.substring(12,14).toInt())); 
            //RTC.adjust(DateTime(__DATE__, __TIME__));
            delay(1000);

           }
            client.print("{\"time\": {");
            client.print("\"fecha\":\"");
            client.print(now.year(), DEC); // Año
            client.print('/');  
            client.print(now.month(), DEC); // Mes
            client.print('/');
            client.print(now.day(), DEC); // Dia
            client.print("\",");
            client.print("\"hora\":\"");
            client.print(now.hour(), DEC); // Horas
            client.print(':');
            client.print(now.minute(), DEC); // Minutos
            client.print(':');
            client.print(now.second(), DEC); // Segundos
            client.println("\"}}"); 
          }
          else if (mensaje.indexOf("/status") != -1)
         {
            client.println("TEMPERATURA Y HUMEDAD ACTUAL");
            client.println("Temp1: " + (String) temp1.temperaturaActual + " :: Hum1: " + (String) hum1.humedadActual);
            client.println("Temp2: " + (String) temp2.temperaturaActual + " :: Hum2: " + (String) hum2.humedadActual);
            client.println();
            client.println("ESTADO DE LOS RELES");
            client.print("Ventilacion: ");
            if(estadoRele1)
              client.println("Encendido");
            else
              client.println("Apagado");
            
            client.print("Riego: ");
            if(estadoRele2)
              client.println("Encendido");
            else
              client.println("Apagado");
            
           client.println();
           client.println("ESTADOS DE CONTROL");
           
           client.print("Estado del Sensor General: ");
           if(estadoSensor)
             client.println("OK");
           else
             client.println("FALLO");

           client.print("Estado del Sensor Temperatura: ");
           if(estadoT)
             client.println("OK");
           else
             client.println("FALLO");
           
           client.print("Estado del Sensor Humedad: ");
           if(estadoH)
             client.println("OK");
           else
             client.println("FALLO");
           
           client.print("Estado Automatico: ");
           if(programado)
             client.println("Activo");
           else
             client.println("Inactivo");
                      
           client.print("Alerta Temperatura: ");
           if(alertaTemp)
             client.println("Activo");
           else
             client.println("Inactivo");
             
           client.print("Alerta Humedad: ");
           if(alertaHum)
             client.println("Activo");
           else
             client.println("Inactivo");
           
           client.print("Modo Manual: ");
           if(modoManual)
             client.println("Activo");
           else
             client.println("Inactivo");
          } 
          else {
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
