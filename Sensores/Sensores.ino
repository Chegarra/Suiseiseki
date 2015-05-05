#include <dht.h>
#include  "SenDHT.h"
#include  "Sensor.h"

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:

}

void loop() {
  SenDHT s1(24);
  SenDHT s2(25);
  Serial.println("Sensor 1:\tT:" + (String)s1.getTemp() + " H: " + (String)s1.getHum());
  Serial.println("Sensor 2:\tT:" + (String)s2.getTemp() + " H: " + (String)s2.getHum());
  delay(10000);
  // put your main code here, to run repeatedly:

}
