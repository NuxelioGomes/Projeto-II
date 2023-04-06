struct DEV_LED : Service::LightBulb {               // First we create a derived class from the HomeSpan LightBulb Service

  int relayPin;                                       // this variable stores the pin number defined for this LED
  int bloquear;
  SpanCharacteristic *power;                        // here we create a generic pointer to a SpanCharacteristic named "power" that we will use below

  // Next we define the constructor for DEV_LED.  Note that it takes one argument, ledPin,
  // which specifies the pin to which the LED is attached.
  
  DEV_LED(int relayPin, int bloquear) : Service::LightBulb(){
    power = new Characteristic::On();
    this->relayPin = relayPin;
    this->bloquear = bloquear;
    pinMode(relayPin, OUTPUT);
  } 

  boolean update(){
    // AQUI VAI ATUALIZAR(ON E OFF) VINDO DA APP
    if(this->bloquear == 0) {
      digitalWrite(relayPin, power->getNewVal());
      return(true);
    }
    return(false);
  }

  int estado_atual(){
    // precisamos de saber se esta ligada ou desligada
    return(power->getVal());
  }

  void atualizar_estado(int valor) {
    power->setVal(valor);
  }

};