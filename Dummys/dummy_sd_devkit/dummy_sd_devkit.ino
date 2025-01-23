
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

// CONSTANTES DO SD
#define CS_SD 5  // Usar pino 5 para CS_SD para ESP32
#define TAXAQ 200

// STRUCT DO GPS
typedef struct {
    int dia;
    int mes;
    int ano;
    int hora;
    int minuto;
    int segundo;
    double lat;  // Changed to double for better precision
    double lon;  // Changed to double for better precision
} GPS;

GPS var_gps = {26, 8, 2024, 10, 0, 30, 5.0, -4.0};

// VARIAVEIS DO SD
File dataFile;
unsigned long ultimaGravacao = 0;
char name_buffer[16];

int rpm = 1;
int velD = 2;
int rpm_roda = 3;

float acel_x = 4;
float acel_y = 5;
float acel_z = 6;


//                                                           FUNCOES DO SD
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
        Serial.print("Criando arquivo: ");
        Serial.println(fileName);

        // Escreve o cabeçalho
        dataFile.println("Millis,Dia,Hora,Latitude,Longitude,RPM Motor,Velocidade,RPM Roda, Acel X, Acel Y, Acel Z");
        dataFile.close();
        Serial.println("Cabecalho escrito com sucesso!");
    } else {
        Serial.print("Erro ao criar arquivo: ");
        Serial.println(fileName);
    }
}

// Atualizar o arquivo com dados
void updateFile() {
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
        dataFile.print(rpm_roda);
        dataFile.print(", ");
        dataFile.print(acel_x);
        dataFile.print(", ");
        dataFile.print(acel_y);
        dataFile.print(", ");
        dataFile.println(acel_z);
        
        dataFile.close();
        Serial.println("Dados atualizados com sucesso!");
    } else {
        Serial.println("Erro ao abrir o arquivo CSV para escrita.");
    }
}

// Inicializar o SD
void setupSD() {

    if (!SD.begin(CS_SD)) {
        Serial.println("Falha ao montar o cartao SD");  

        if (ultimaGravacao + TAXAQ < millis()) {
          setupSD();
          return;
        }
    }

    Serial.println("Cartao SD montado com sucesso");
    createNewFile();
}

// Loop de gravação no SD
void loopSD() {
    if (ultimaGravacao + TAXAQ < millis()) {
        updateFile(); 
        ultimaGravacao = millis();
    }
}

//                                                                MAIN
void setup(){
  Serial.begin(9600); 
  setupSD();
}
void loop(){
  loopSD();
}
