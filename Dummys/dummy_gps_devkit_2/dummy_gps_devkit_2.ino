#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

static const int RXPin = 16, TXPin = 17;  // Pinos RX2 e TX2 no ESP32 // OBS: Trocar por defines
static const uint32_t GPSBaud = 9600;  // Taxa de baud do GPS
TinyGPSPlus gps; // O objeto TinyGPSPlus
SoftwareSerial ss(RXPin, TXPin); // A conexão serial com o GPS

// Função customizada para garantir que o GPS seja "alimentado"
static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (ss.available()) {
      gps.encode(ss.read());  // Processa os dados do GPS
    }
  } while (millis() - start < ms);
}

// Função para ajustar o fuso horário
int horaBR(int horaUK) {
  int horaBRASIL;
  if (horaUK < 3) {
    horaBRASIL = 21 + horaUK;
    return horaBRASIL;
  }
  horaBRASIL = horaUK - 3;
  return horaBRASIL;
}

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

GPSData var_gps = {0,0,0,0,0,0,0,0};

void setup() {
  Serial.begin(9600);
  Serial.println("Iniciando...");
  ss.begin(GPSBaud);
  delay(2000);  // Espera 2 segundos para o GPS começar a responder
  Serial.println("Iniciado.");
}

void loop() {
    // Se os dados de localização foram atualizados
  if (gps.location.isUpdated()) {
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
      // Exibe as informações no monitor serial
  char sz[64];
  sprintf(sz, "%02d/%02d/%04d | %02d:%02d:%02d | %f, %f \n", var_gps.dia, var_gps.mes, var_gps.ano, var_gps.hora, var_gps.minuto, var_gps.segundo, var_gps.lat, var_gps.lon);
  Serial.print(sz);  // Imprime as informações de data e localização
  smartDelay(1000);
  // Se não houver dados GPS após 5 segundos
  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println(F("No GPS data received: check wiring"));
  }
}
