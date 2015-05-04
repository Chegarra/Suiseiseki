#include <dht.h>

class SenDHT 
{
  private:
    int   pin;
    float temp;
    float hum;
    
  public:
  
    SenDHT(int);
    
    void setPin (int);
    void setTemp(void);
    void setHum (void);
    
    int   getPin   (void) const;
    float getTemp  (void) const;
    float getHum   (void) const;
  
};

SenDHT::SenDHT(int p)
{
  this.setPin(p);
  this.setTemp();
  this.setHum();
}

int   SenDHT::getPin() const {return this->pin; }
float SenDHT::getTemp()const {return this->temp;}
float SenDHT::getHum() const {return this->hum; }

void SenDHT::setPin(int t) { this->temp = t; }

void SenDHT::setTemp() 
{
  int chk = DHT.read22(this.getPin());
  switch (chk) {
    case DHTLIB_OK:
      this.temp = DHT.temperature;
      break;
    case DHTLIB_ERROR_CHECKSUM:
    case DHTLIB_ERROR_TIMEOUT:
    case DHTLIB_ERROR_CONNECT:
    case DHTLIB_ERROR_ACK_L:
    case DHTLIB_ERROR_ACK_H:
    default:
      this->temp = -111;
  }
}

void SenDHT::setHum()
{
  int chk = DHT.read22(this.getPin());
  switch (chk) {
    case DHTLIB_OK:
      this.hum = DHT.humidity;
      break;
    case DHTLIB_ERROR_CHECKSUM:
    case DHTLIB_ERROR_TIMEOUT:
    case DHTLIB_ERROR_CONNECT:
    case DHTLIB_ERROR_ACK_L:
    case DHTLIB_ERROR_ACK_H:
    default:
      this->hum = -111;
  }
}
