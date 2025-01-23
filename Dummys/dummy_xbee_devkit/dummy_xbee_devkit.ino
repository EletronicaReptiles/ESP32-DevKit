
#define BAUDRATE          9600
#define deltaXbee         50

int8_t velD;
int8_t velT;
int rpm;
int tempCVT = -273;
bool stateLedBox = 0;
bool stateYelwLed = 0;
unsigned long timeoldXbee = 0;

void setup()
{
  Serial.begin(BAUDRATE);  // start Serial for output

   velD = 34;
   velT = 35;
   rpm = 2000;
   tempCVT = 100;
}

void loop()
{
  recebeXbee();

  if (millis() - timeoldXbee >= deltaXbee) {
    enviaXbee();
    timeoldXbee = millis();
  }
}

void recebeXbee() {
  //ALERTA BOX
  if (Serial.available() > 0)
  {
    char valorLido = Serial.read();
    stateLedBox = valorLido - '0';
  }
}

void enviaXbee()
{
  String str;
   str = 'v' +   String(velD)    + String(',') +
         'u' +   String(velT)    + String(',') +
         'r' +   String(rpm)    +String(',') +
         't' +   String(tempCVT)     + String(',') +
         'y' +   String((int)stateYelwLed)    + String(',') ;

  Serial.println(str);
}
