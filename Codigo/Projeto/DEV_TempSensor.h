struct DEV_TempSensor : Service::TemperatureSensor {
  
  SpanCharacteristic *temp;
  int sensorPin;

  DEV_TempSensor(int sensorPin, float temperatura) : Service::TemperatureSensor() {
    temp = new Characteristic::CurrentTemperature(temperatura);
    temp->setRange(-100,1000);
    temp->setVal(temperatura);
  }

  boolean atualizar_temp(float temperatura) {
    temp->setVal(temperatura);
    return true;
  }

};