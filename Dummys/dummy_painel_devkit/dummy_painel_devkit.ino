
#include <Wire.h>
#include <SPI.h>
#include <max6675.h>
#include <SoftwareSerial.h>

#define painelAdress      0x08
#define tamanhoVetor      4

bool stateYelwLed = 0;
bool stateLedBox = 0;
byte Vet[tamanhoVetor] = {0, 0, 0, 0};

//VARIAVEIS PARA TESTE
int8_t velD = 0;

void enviaPainel()
{
  Vet[0] = 0;
  Vet[1] = 0;
  Vet[2] = velD & 0xFF;
  Vet[3] = (stateLedBox << 1) | stateYelwLed; 

  Wire.beginTransmission(painelAdress);
  Wire.write(Vet, tamanhoVetor);
  Wire.endTransmission(); 
}

//                                                                MAIN
void setup(){
  Serial.begin(9600); 
  Wire.begin();
}

void loop(){
  enviaPainel();
  
  velD+=1;
  
  if(stateLedBox==1){
    stateLedBox = 0;
  }
  else{
    stateLedBox = 1;
  }

  delay(1000);

}

