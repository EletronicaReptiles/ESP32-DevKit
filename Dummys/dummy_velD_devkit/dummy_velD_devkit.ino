//                                                                CONSTANTES DA VELOCIDADE
#define VELPIN 32 //GPIO 32
#define QTDPARAFUSO 4
#define MEUPI 3.14159265359
#define RAIO  0.248031  

//                                                                VARIAVEIS DA VELOCIDADE
int8_t velD = 0;
int8_t  rpm_roda = 0;
const unsigned long ZeroTimeoutVel = 5565 ;
byte trocasD = 0;
byte stateVelD = 0;
byte oldStateVelD = 1;
unsigned long timeoldVelD = 0;
unsigned long timeoldInputD = 0;
const unsigned long ZeroTimeoutD = ZeroTimeoutVel / QTDPARAFUSO;
unsigned long ZeroDebouncingExtraD;
const uint8_t numReadings = QTDPARAFUSO; 
const float velTransConst = 3600 * 2 * MEUPI * RAIO;

//                                                                FUNCOES DA VELOCIDADE

void setupVel(){
  pinMode(VELPIN, INPUT_PULLUP);
} 

void loopVel() {
   stateVelD = digitalRead(VELPIN);
  // Serial.println(stateVelD);
   unsigned long now = millis();

   if (stateVelD == 0 && oldStateVelD == 1)
  {
    trocasD++;
    timeoldInputD = now;
    //Serial.println("Pulso!");
  }
  unsigned long deltatVelD = now - timeoldVelD;

  if (deltatVelD >= 1500){
      rpm_roda = 0;
    }

  if (trocasD >= numReadings)
  {

    rpm_roda = (60000/(QTDPARAFUSO * deltatVelD));
    velD = velTransConst * trocasD / (deltatVelD * QTDPARAFUSO);
    trocasD = 0;
    timeoldVelD = now;
  }
  else if ( (now - timeoldInputD > ZeroTimeoutD - ZeroDebouncingExtraD)) {
    velD = 0;
    ZeroDebouncingExtraD = ZeroTimeoutD * 2 / 100;
  }
  else {
    ZeroDebouncingExtraD = 0;
  }
  oldStateVelD = stateVelD;

   Serial.print("Velocidade: ");
   Serial.println(velD);

} 

//                                                                 MAIN DA VELOCIDADE

void setup() {  
  Serial.begin(9600);
  setupVel();
}

void loop() {
  loopVel();
}
