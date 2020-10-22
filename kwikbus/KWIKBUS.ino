/*
 * Program KWIKBUS.ino
 * Finalisasi program 8:57PM 02 Agustus 2019.
 * Program ini menggunakan sensor GPS, DS12B20 (suhu), dan Switch
 * Dibuat untuk sistem kwikbus.id sebagai pendektesi lokasi bus, suhu di dalam bus, jumlah penumpang, posisi halte bus, pendeteksi penumpang di bangku, 
   dan pendeteksi asap. 
 * Bukan komersil dan dapat dipergunakan secara gratis.
 * Program ini dikhususkan untuk kwikbus.id, apabila digunakan untuk sistem lain,
   tidak ada jaminan program ini dapat berjalan dengan sempurna.  
 */

#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_RX_BUFFER 512
#include <NMEAGPS.h>
#include <GPSport.h>
#include <Streamers.h>
#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
#include <Wire.h>  
#include <Adafruit_MLX90614.h>
#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN 53
#define RST_PIN 48
#define SerialAT Serial2          
#define DUMP_AT_COMMANDS
#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, Serial);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif

static NMEAGPS  gps;
static gps_fix  fix;

const char apn[] = "internet";
const char user[] = "";
const char pass[] = "";
const char server[] = "kwikbus.id";
const char resource[] = "/src/add.php?";
const int port = 80;

TinyGsmClient client(modem);
HttpClient http(client, server, port);

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

MFRC522 mfrc522(SS_PIN, RST_PIN);

const uint8_t seatpin[] = {22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 42};
const uint8_t buzzer = 38;
bool stat;
void setup() {
  portbegin();
  modemkonek();
}

void portbegin() {
  Serial.begin(115200);
  SerialAT.begin(115200);
  gpsPort.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  mlx.begin();  
  for (uint8_t n = 0; n < 11; n++) {
      pinMode(seatpin[n], INPUT_PULLUP);
  }
  pinMode(3, INPUT_PULLUP);
  digitalWrite(3, HIGH);
  attachInterrupt(digitalPinToInterrupt(3), changestat, FALLING);
  pinMode(9, INPUT);
  pinMode(buzzer, OUTPUT);
 }

void loop() {
    char datafix[200], counter = 0, tambah = 0;
    uint16_t n, i;
    static uint8_t ir = 0;
    static uint16_t total = 0;
    int analogSensor = 0;
    bool *seat = fixseat(), lock = 0, asap = 0;
    
    while (i < 2000 && stat == 1) {
      if (digitalRead(9) == LOW) {
        counter = 1;
        tambah = 1;
        stat = 0;
      } else if (stat == 1) {
        counter = -1;
        lock = 1;
        i++;
      }
      delay(1);
    } 

    if (lock == 1) {
      ir = ircount(counter, ir);
      total = irtambah(tambah, total);
      stat = 0;
      }
      
    while(gps.available(gpsPort) && stat == 0) {
      fix = gps.read();
      Serial.print(".");
      if ((!fix.longitude() == 0 ) || (!fix.latitude() == 0)){
       asap = fixasap(analogSensor, asap);
       snprintf(datafix, sizeof(datafix), "%slat=%s&lon=%s&spd=%s&rfid=%s&temp=%s&a1=%d&a2=%d&b1=%d&b2=%d&c1=%d&c2=%d&d1=%d&d2=%d&e1=%d&f1=%d&f2=%d&ir=%d&total=%d&asap=%d", resource, fixlat(), fixlon(), fixspd(), fixrfid(), fixtemp(), *(seat + 0), *(seat + 1), *(seat + 2), *(seat + 3), *(seat + 4), *(seat + 5), *(seat + 6), *(seat + 7), *(seat + 8), *(seat + 9), *(seat + 10), ir, total, asap);
       Serial.println(datafix); 
       sendhttp(datafix);
       }
    }
}

void changestat() {
  stat = 1;
}

void modemkonek() {
  if (!modem.waitForNetwork()) {
        Serial.println(F(" Modem gagal terhubung ke jaringan selular"));
        delay(10000);
        return;
    }
    if (!modem.gprsConnect(apn, user, pass)) {
        Serial.println(F(" Modem gagal terhubung ke jaringan internet"));
        delay(10000);
        return;
    }
}
void sendhttp(char *datafix) {
    client.connect(server, port);
    if (!client.connected()) {
        Serial.println(F("modem disconnected, waiting for resolving"));
        modemkonek();
    } else {
        int err = http.get(datafix);
        if (err > 0) {
           Serial.println("failed to connect");
           delay(2000);
           return;
        }
        client.flush();
        http.stop();
    }
}
