
//                                                                         INCLUDES
#include <Wire.h>
#include <SPI.h>
#include <max6675.h>
#include <SoftwareSerial.h>
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <TinyGPSPlus.h>

//                                                                      STRUCT DO GPS
typedef struct {
    int dia;
    int mes;
    int ano;
    int hora;
    int minuto;
    int segundo;
    double lat;
    double lon;
} GPSData;


//                                                                         DEFINES
// DEFINES DO PAINEL
#define painelAdress      0x08
#define tamanhoVetor      4

// DEFINES DA VELOCIDADE
#define VELPIN 32 //GPIO 32
#define QTDPARAFUSO 4
#define MEUPI 3.14159265359
#define RAIO  0.248031  

// DEFINES DO XBEE
#define BAUDRATE          9600
#define deltaXbee         50

// DEFINES DO SD
#define CS_SD 5  // Usar pino 5 para CS_SD para ESP32
#define TAXAQ 50

// DEFINES DO GPS
#define deltaGPS        1000  //1 segundo


//                                                                         VARIAVEIS
// VARIAVEIS DO PAINEL
bool stateYelwLed = 0;
bool stateLedBox = 0;
byte Vet[tamanhoVetor] = {0, 0, 0, 0};

// VARIAVEIS DA VELOCIDADE
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

// VARIAVEIS DO XBEE
int8_t velT;
int tempCVT = -273;
unsigned long timeoldXbee = 0;

// VARIAVEIS DO SD
File dataFile;
unsigned long ultimaGravacao = 0;
char name_buffer[16];
bool stateSD = false;

// VARIAVEIS DO GPS
static const int RXPinGPS = 16, TXPinGPS = 17;  // Pinos RX2 e TX2 no ESP32 // OBS: Trocar por defines
static const uint32_t GPSBaud = 9600;  
TinyGPSPlus gps; // O objeto TinyGPSPlus
SoftwareSerial ss(RXPinGPS, TXPinGPS); // A conexão serial com o GPS
GPSData var_gps;
unsigned long timeoldGPS = 0;
int a;
int b;
int c;
int d;
int e;
int f;
int g;
int h;

// VARIAVEIS DO RPM
int rpm = 0;


//                                                                         FUNCOES
// FUNCOES DO PAINEL
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

// FUNCOES DA VELOCIDADE
void setupVel(){
  pinMode(VELPIN, INPUT_PULLUP);
} 

void loopVel() {
   stateVelD = digitalRead(VELPIN);
   unsigned long now = millis();

   if (stateVelD == 0 && oldStateVelD == 1)
  {
    trocasD++;
    timeoldInputD = now;
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

} 

// FUNCOES DO XBEE
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

//FUNCOES DO SD
// Função para contar o número de arquivos no diretório
int contaArquivos(String dirName) {
    File dir = SD.open(dirName);
    int nArq = 0;

    while (true) {
        File entry = dir.openNextFile();
        if (!entry) {
            break;
        }
        if (!entry.isDirectory()) {
            nArq++;
        }
        entry.close();
    }
    return nArq;
}

// Função de criação de nome de arquivo com base no número de arquivos
String createFileName() {
    int nArq = contaArquivos("/");
    nArq++;  // Incrementa para o novo arquivo

    sprintf(name_buffer, "/TST%02d.CSV", nArq);
    return String(name_buffer);
}

// Criar novo arquivo com cabeçalho
void createNewFile() {
    String fileName = createFileName();
    dataFile = SD.open(fileName, FILE_WRITE);

    if (dataFile) {
        dataFile.println("Millis,Dia,Hora,Latitude,Longitude,RPM Motor,Velocidade,RPM Roda");
        dataFile.close();
    } else {
        stateSD = false; // Atualiza o estado do SD para evitar gravações futuras
    }
}

void updateFile() {
    if (!stateSD) return; // Ignorar se o SD não está operacional

    dataFile = SD.open(name_buffer, FILE_APPEND);

    if (dataFile) {
        dataFile.print(millis());
        dataFile.print(", ");
        
        // Formatação com dois dígitos para dia, mês, ano, hora, minuto e segundo
        char buffer[20];
        sprintf(buffer, "%02d/%02d/%04d", var_gps.dia, var_gps.mes, var_gps.ano);
        dataFile.print(buffer);
        dataFile.print(", ");
        
        sprintf(buffer, "%02d:%02d:%02d", var_gps.hora, var_gps.minuto, var_gps.segundo);
        dataFile.print(buffer);
        dataFile.print(", ");
        
        dataFile.print(var_gps.lat, 6);  // 6 casas decimais para precisão
        dataFile.print(", ");
        dataFile.print(var_gps.lon, 6);
        dataFile.print(", ");
        dataFile.print(rpm);
        dataFile.print(", ");
        dataFile.print(velD);
        dataFile.print(", ");
        dataFile.println(rpm_roda);
        
        dataFile.close();
    } else {
        stateSD = false; // Marcar o SD como inativo
    }
}

// Loop de gravação no SD com rechecagem
void loopSD() {
    static unsigned long lastAttempt = 0;

    if (!stateSD) {
        // Tenta reconfigurar o SD após 5 segundos
        if (millis() - lastAttempt > 5000) {
            lastAttempt = millis();
            setupSD(); // Tenta reconfigurar
        }
    } else {
        // Tenta gravar os dados no SD
        if (millis() - ultimaGravacao >= TAXAQ) {
            updateFile(); 
            ultimaGravacao = millis();
        }
    }
}

// Setup do SD com estado seguro
void setupSD() {
    if (!SD.begin(CS_SD)) {
        stateSD = false;
        return;
    }

    stateSD = true;
    createNewFile(); 
}


// FUNCOES DO GPS
int horaBR(int horaUK) { // Função para ajustar o fuso horário

  int horaBRASIL;
  if (horaUK < 3) {
    horaBRASIL = 21 + horaUK;
    return horaBRASIL;
  }
  horaBRASIL = horaUK - 3;
  return horaBRASIL;
}

static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (ss.available()) {
      gps.encode(ss.read());  // Processa os dados do GPS
    }
  } while (millis() - start < ms);
}

void loopGPS() {
  if (gps.location.isUpdated()) { // Se os dados de localização foram atualizado

    stateYelwLed = 1;

    var_gps = {
      gps.date.day(),
      gps.date.month(),
      gps.date.year(),
      horaBR(gps.time.hour()),
      gps.time.minute(),
      gps.time.second(),
      gps.location.lat(),
      gps.location.lng()
    };
  }
  else{
    stateYelwLed = 0;
  }

  char sz[64];
  sprintf(sz, "%02d/%02d/%04d | %02d:%02d:%02d | %f, %f \n", var_gps.dia, var_gps.mes, var_gps.ano, var_gps.hora, var_gps.minuto, var_gps.segundo, var_gps.lat, var_gps.lon);
  Serial.print(sz);  // Imprime as informações de data e localização
  smartDelay(1000);


}


void loopGPSPrint() {
  Serial.println("A");
  if (gps.location.isUpdated()) { // Se os dados de localização foram atualizado
  Serial.println("B");

    stateYelwLed = 1;

    var_gps = {
      gps.date.day(),
      gps.date.month(),
      gps.date.year(),
      horaBR(gps.time.hour()),
      gps.time.minute(),
      gps.time.second(),
      gps.location.lat(),
      gps.location.lng()
  };
  Serial.println("C");
  }
  else{
    Serial.println("D");
    stateYelwLed = 0;
  }
  Serial.println("E");
  char sz[64];
  sprintf(sz, "%02d/%02d/%04d | %02d:%02d:%02d | %f, %f \n", var_gps.dia, var_gps.mes, var_gps.ano, var_gps.hora, var_gps.minuto, var_gps.segundo, var_gps.lat, var_gps.lon);
  Serial.print(sz);  // Imprime as informações de data e localização
  Serial.println("F");
}

void loopGPSTESTE() {
  if ((millis()>5000)&&(millis()<10000)) { // Se os dados de localização foram atualizado

    stateYelwLed = 1;

    var_gps = {
      a++,
      b++,
      c++,
      horaBR(d++),
      e++,
      f++,
      g++,
      h++
  };
  }
  else{
    stateYelwLed = 0;
  }
  char sz[64];
  sprintf(sz, "%02d/%02d/%04d | %02d:%02d:%02d | %f, %f \n", var_gps.dia, var_gps.mes, var_gps.ano, var_gps.hora, var_gps.minuto, var_gps.segundo, var_gps.lat, var_gps.lon);
  Serial.print(sz);  // Imprime as informações de data e localização
}

//                                                                            MAIN
void setup(){
  Serial.begin(BAUDRATE); 
  Wire.begin();
  ss.begin(GPSBaud);
  setupVel();
  //setupRPM();
  setupSD(); 
}

void loop(){
  //loopRPM();
  loopVel();
  
  if (millis() - timeoldGPS >= deltaGPS) {
    loopGPS();
    timeoldGPS = millis();
  }

  enviaPainel();
  loopSD();
  recebeXbee();

  if (millis() - timeoldXbee >= deltaXbee) {
    enviaXbee();
    timeoldXbee = millis();
  }


}

