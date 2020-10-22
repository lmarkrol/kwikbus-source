//Data Suhu
char *fixtemp() {
    static char temp[6];
    dtostrf(mlx.readAmbientTempC(), 0, 2, temp);
    return temp;
}
//Data Longitude
char *fixlon() {
    static char lon[12];
    dtostrf(fix.longitude(), 0, 6, lon);
    return lon;
}
//Data latitude
char *fixlat() {
    static char lat[12];
    dtostrf(fix.latitude(), 0, 6, lat);
    return lat;
}
//Data kecepatan
char *fixspd() {
    static char spd[4];
    dtostrf(fix.speed_kph(), 0, 0, spd);
    return spd;
}

//Data RFID
char *fixrfid() {
    String rfid;
    static String bufferdata;
    static char bufferrfid[12];
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        for (byte i = 0; i < mfrc522.uid.size; i++) {
            if (mfrc522.uid.uidByte[i] < 0x10) {
                    rfid += '0';
            }
        rfid += String(mfrc522.uid.uidByte[i], HEX);
        }
        } else {
        bufferdata.toCharArray(bufferrfid, 12);
        return bufferrfid;
        }
       if (rfid == "4300082e") {
      rfid = "Shelter+A";
    } else if (rfid == "5937c25a") {
      rfid = "On+going";
    }
      else if (rfid == "d9dcdb59") {
      rfid = "Shelter+B";
      }
      else if (rfid == "493cc45a") {
      rfid = "On+going";
      }
    bufferdata = rfid;
    rfid.toCharArray(bufferrfid, 12);
    return bufferrfid;
}

//Data kursi
bool *fixseat() {
    uint32_t timeseat;
    static bool seatstat[11];
    if (timeseat > millis()) {
        timeseat = 0;
    }
    if (millis() - timeseat > 2000) {
        for (uint8_t n = 0; n < 11; n++) {
            if (digitalRead(seatpin[n]) == LOW) {
                seatstat[n] = 1;
            } else {
                seatstat[n] = 0;
            }
        }
        timeseat = millis();
    }
    return seatstat;
}

//Data jumlah penumpang
uint8_t ircount(char counter, uint8_t ir) {
    if (ir == 0 && counter < 0) {
      ir = 0;
        return ir;
    } 
    else {
      if (ir >= 0) {
      ir += counter;
      return ir;
      }
    } 
}

//Data total penumpang
uint16_t irtambah(char tambah, uint16_t total) {
    if (total >= 0) {
    total += tambah;
    return total;
    }
}

//Data asap
uint8_t fixasap(int analogSensor, bool asap) {
  const uint8_t buzzer = 38;
  analogSensor = analogRead(A2);
      if(analogSensor>400){
        asap=1;
        digitalWrite(buzzer, HIGH);
        }
      else{
        asap=0;
        digitalWrite(buzzer, LOW);
        }
        return asap;
}
