class SenDHT : public Sensor 
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
  this->setPin(p);
  this->setTemp();
  this->setHum();
}

int   SenDHT::getPin() const {return this->pin; }
float SenDHT::getTemp()const {return this->temp;}
float SenDHT::getHum() const {return this->hum; }

void SenDHT::setPin(int p) { this->pin = p; }

void SenDHT::setTemp() 
{
  dht DHT;

  int chk = DHT.read11(getPin());
  switch (chk) {
    case DHTLIB_OK:
      this->temp = DHT.temperature;
      break;
    case DHTLIB_ERROR_CHECKSUM:
      this->temp = -101;
      break;
    case DHTLIB_ERROR_TIMEOUT:
      this->temp = -102;
      break;
    case DHTLIB_ERROR_CONNECT:
      this->temp = -103;
      break;
    case DHTLIB_ERROR_ACK_L:
      this->temp = -104;
      break;
    case DHTLIB_ERROR_ACK_H:
    default:
      this->temp = -111;
  }
}

void SenDHT::setHum()
{
  dht DHT;
  int chk = DHT.read11(getPin());
  switch (chk) {
    case DHTLIB_OK:
      this->hum = DHT.humidity;
      break;
    case DHTLIB_ERROR_CHECKSUM:
      this->hum = -201;
      break;
    case DHTLIB_ERROR_TIMEOUT:
      this->hum = -202;
      break;
    case DHTLIB_ERROR_CONNECT:
      this->hum = -203;
      break;
    case DHTLIB_ERROR_ACK_L:
      this->hum = -204;
      break;    
    case DHTLIB_ERROR_ACK_H:
    default:
      this->hum = -222;
  }
}

