#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <dht.h>
#include <DS3231.h>
#include <U8g2lib.h>
#include <Wire.h>

#define dataPin 8 // DHT22 senzor na digitalnom pinu 8
dht DHT; // Kreira se DHT objekat
DS3231  rtc(SDA, SCL); // Kreiranje objekta tipa DS3231
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); // Inicijalizacija OLED displeja koristeci I2C komunikaciju

RF24 radio(10, 9); // CE, CSN pinovi za komunikaciju
const byte address[6] = "00001"; // Adresa za komunikaciju sa vanjskom jedinicom

char text[18] = "";
int citajDHT22, t, h;
String unuTemp, unuVlaz, vanTemp, vanVlaz, pritisak, nadmVisina, lux;
String rtcVrijeme, rtcDatum;
int ispisEkran = 0;

unsigned long prethodnoVrijemeDisplej = 0;
unsigned long prethodnoVrijemeDHT22 = 0;
unsigned long prethodnoVrijemeSerial = 0;
long intervalDisplej = 3000; // Podesavanje intervala za ispis na displeju (3 sekunde)
long intervalDHT22 = 120000; // Podesavanje intervala za slanje podataka sa unutrasnjeg DHT22 senzora (2 minute na OLED displej)
long intervalSerial = 5000;  // Podesavanje intervala za slanje svih podataka preko serijskog porta u MongoDB bazu (30 minuta)

#define Temperatura_Ikona_sirina 27
#define Temperatura_Ikona_visina 47
// Bitmap kod za ikonicu
static const unsigned char Temperatura_Ikona[] U8X8_PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x80, 0x7f, 0x00, 0x00,
  0xc0, 0xe1, 0x00, 0x00, 0xe0, 0xc0, 0x01, 0x00, 0x60, 0x80, 0xf9, 0x03,
  0x60, 0x80, 0x01, 0x00, 0x60, 0x80, 0x01, 0x00, 0x60, 0x80, 0x79, 0x00,
  0x60, 0x80, 0x01, 0x00, 0x60, 0x80, 0x01, 0x00, 0x60, 0x80, 0xf9, 0x03,
  0x60, 0x80, 0x01, 0x00, 0x60, 0x80, 0x01, 0x00, 0x60, 0x8c, 0x79, 0x00,
  0x60, 0x9e, 0x01, 0x00, 0x60, 0x9e, 0x01, 0x00, 0x60, 0x9e, 0xf9, 0x03,
  0x60, 0x9e, 0x01, 0x00, 0x60, 0x9e, 0x01, 0x00, 0x60, 0x9e, 0x79, 0x00,
  0x60, 0x9e, 0x01, 0x00, 0x60, 0x9e, 0x01, 0x00, 0x60, 0x9e, 0xf9, 0x03,
  0x60, 0x9e, 0x01, 0x00, 0x60, 0x9e, 0x01, 0x00, 0x60, 0x9e, 0x01, 0x00,
  0x70, 0x9e, 0x03, 0x00, 0x38, 0x1e, 0x07, 0x00, 0x18, 0x3e, 0x0e, 0x00,
  0x1c, 0x3f, 0x0c, 0x00, 0x0c, 0x7f, 0x18, 0x00, 0x8c, 0xff, 0x18, 0x00,
  0x8e, 0xff, 0x38, 0x00, 0xc6, 0xff, 0x31, 0x00, 0xc6, 0xff, 0x31, 0x00,
  0xc6, 0xff, 0x31, 0x00, 0x8e, 0xff, 0x38, 0x00, 0x8c, 0xff, 0x18, 0x00,
  0x0c, 0x7f, 0x1c, 0x00, 0x3c, 0x1c, 0x0e, 0x00, 0x78, 0x00, 0x06, 0x00,
  0xe0, 0x80, 0x07, 0x00, 0xe0, 0xff, 0x03, 0x00, 0x80, 0xff, 0x00, 0x00,
  0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


#define Vlaznost_Ikona_sirina 27
#define Vlaznost_Ikona_visina 47
static const unsigned char Vlaznost_Ikona[] U8X8_PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00,
  0x00, 0x70, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0xdc, 0x00, 0x00,
  0x00, 0xdc, 0x01, 0x00, 0x00, 0x8e, 0x01, 0x00, 0x00, 0x86, 0x03, 0x00,
  0x00, 0x06, 0x03, 0x00, 0x00, 0x03, 0x07, 0x00, 0x80, 0x03, 0x06, 0x00,
  0x80, 0x01, 0x0c, 0x00, 0xc0, 0x01, 0x1c, 0x00, 0xc0, 0x00, 0x18, 0x00,
  0xe0, 0x00, 0x38, 0x00, 0x60, 0x00, 0x30, 0x00, 0x70, 0x00, 0x70, 0x00,
  0x30, 0x00, 0xe0, 0x00, 0x38, 0x00, 0xc0, 0x00, 0x18, 0x00, 0xc0, 0x01,
  0x1c, 0x00, 0x80, 0x01, 0x0c, 0x00, 0x80, 0x03, 0x0e, 0x00, 0x80, 0x03,
  0x06, 0x00, 0x00, 0x03, 0x06, 0x00, 0x00, 0x03, 0x07, 0x00, 0x00, 0x07,
  0x03, 0x00, 0x00, 0x06, 0x03, 0x00, 0x00, 0x06, 0x03, 0x00, 0x00, 0x06,
  0x63, 0x00, 0x00, 0x06, 0x63, 0x00, 0x00, 0x06, 0x63, 0x00, 0x00, 0x06,
  0xe3, 0x00, 0x00, 0x06, 0xc7, 0x00, 0x00, 0x06, 0xc6, 0x01, 0x00, 0x07,
  0x86, 0x03, 0x00, 0x03, 0x0e, 0x1f, 0x00, 0x03, 0x0e, 0x1e, 0x80, 0x01,
  0x1c, 0x00, 0xc0, 0x01, 0x38, 0x00, 0xe0, 0x00, 0x78, 0x00, 0x70, 0x00,
  0xf0, 0x00, 0x38, 0x00, 0xe0, 0x07, 0x1f, 0x00, 0x80, 0xff, 0x0f, 0x00,
  0x00, 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00
};

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
  u8g2.begin();
  rtc.begin();
  //rtc.setDOW(MONDAY);     // Podesavanje dana u sedmici
  //rtc.setTime(14, 55, 00);     // Podesavanje vremena 12:00:00 (24h format)
  //rtc.setDate(25, 12, 2023);   // Podesavanje datuma 25 Decembar 2023
}

void loop() {
  if (radio.available()) {
    radio.read(&text, sizeof(text)); // Citanje primljenih podataka
    vanTemp = String(text[0]) + String(text[1]) + char(176) + "C";
    vanVlaz = String(text[2]) + String(text[3]) + "%";
    pritisak = String(text[4]) + String(text[5]) + String(text[6]) + String(text[7]) + " mbar";
    nadmVisina = String(text[8]) + String(text[9]) + String(text[10]) + " m";
    lux = String (text[11]) + String(text[12]) + String(text[13]) + String (text[14]) + String(text[15]) + String(text[16]) + String(text[17]);
  }

  unsigned long trenutnoVrijemeDisplej = millis();
  if (trenutnoVrijemeDisplej - prethodnoVrijemeDisplej > intervalDisplej) {
    prethodnoVrijemeDisplej = trenutnoVrijemeDisplej;
    azurirajDisplej();
  }

  unsigned long trenutnoVrijemeDHT22 = millis();
  if (trenutnoVrijemeDHT22 - prethodnoVrijemeDHT22 > intervalDHT22) {
    prethodnoVrijemeDHT22 = trenutnoVrijemeDHT22;
    DHT.read22(dataPin);
  }
  unsigned long trenutnoVrijemeSerial = millis();
  if (trenutnoVrijemeSerial - prethodnoVrijemeSerial > intervalSerial) {
    prethodnoVrijemeSerial = trenutnoVrijemeSerial;

  // Slanje preko serijske komunikacije - Python skripta prima podatke u obliku XY, XY, XY, ...
  //Serial.print("Unutrasnja temperatura");
  Serial.print(t);
  Serial.print(", ");
  //Serial.print("Unutrasnja vlaznost");
  Serial.print(h);
  Serial.print(", ");
  //Serial.print("Vanjska temperatura");
  Serial.print(String(text[0]) + String(text[1]));
  Serial.print(", ");
  //Serial.print("Vanjska vlaznost");
  Serial.print(String(text[2]) + String(text[3]));
  Serial.print(", ");
  //Serial.print("Pritisak");
  Serial.print(String(text[4]) + String(text[5]) + String(text[6]) + String(text[7]));
  Serial.print(", ");
  //Serial.print("Nivo osvjetljenosti");
  Serial.print(String (text[11]) + String(text[12]) + String(text[13]) + String (text[14]) + String(text[15]) + String(text[16]) + String(text[17]));
  Serial.print("\n");
  }
}

void azurirajDisplej() {
  u8g2.firstPage();
  do {
    switch (ispisEkran) {
      case 0: ispisRTC(); break;
      case 1: ispisUnutrasnjeTemp(); break;
      case 2: ispisUnutrasnjeVlaz(); break;
      case 3: ispisVanjskeTemp(); break;
      case 4: ispisVanjskeVlaz(); break;
      case 5: ispisPritiska(); break;
      case 6: ispisNadmVis(); break;
      case 7: ispisLux(); break;
    }
  } while (u8g2.nextPage());
  ispisEkran++;
  if (ispisEkran > 7) {
    ispisEkran = 0;
  }
}
  
void ispisRTC() {
  String dowa = rtc.getDOWStr();
  dowa.remove(3);
  rtcDatum = dowa + " " + rtc.getDateStr(); // DS3231 RTC datum
  u8g2.setFont(u8g2_font_helvB12_tr);
  u8g2.setCursor(3, 15);
  rtcVrijeme = rtc.getTimeStr(); // DS3231 RTC vrijeme
  rtcVrijeme.remove(5);
  u8g2.print(rtcDatum);
  u8g2.setFont(u8g2_font_logisoso30_tf);
  u8g2.setCursor(20, 58);
  u8g2.print(rtcVrijeme);
}

void ispisUnutrasnjeTemp() {
  citajDHT22 = DHT.read22(dataPin); // Citanje podataka sa unutrasnjeg senzora
  t = DHT.temperature; // Preuzimanje vrijednosti unutrasnje temperature
  unuTemp = String(t) + char(176) + "C";
  u8g2.setFont(u8g2_font_helvB12_tr);
  u8g2.setCursor(5, 15);
  u8g2.print("UNUTRASNJA");
  u8g2.setFont(u8g2_font_logisoso30_tf);
  u8g2.setCursor(36, 58);
  u8g2.print(unuTemp);
  u8g2.drawXBMP( 0, 17, Temperatura_Ikona_sirina, Temperatura_Ikona_visina, Temperatura_Ikona);
}

void ispisUnutrasnjeVlaz() {
  h = DHT.humidity; // Preuzimanje vrijednosti unutrasnje vlaznosti
  unuVlaz = String(h) + "%";
  u8g2.setFont(u8g2_font_helvB12_tr);
  u8g2.setCursor(5, 15);
  u8g2.print("UNUTRASNJA");
  u8g2.setFont(u8g2_font_logisoso30_tf);
  u8g2.setCursor(36, 58);
  u8g2.print(unuVlaz);
  u8g2.drawXBMP( 0, 17, Vlaznost_Ikona_sirina, Vlaznost_Ikona_visina, Vlaznost_Ikona);
}

void ispisVanjskeTemp() {
  u8g2.setFont(u8g2_font_helvB12_tr);
  u8g2.setCursor(25, 15);
  u8g2.print("VANJSKA");
  u8g2.setFont(u8g2_font_logisoso30_tf);
  u8g2.setCursor(36, 58);
  u8g2.print(vanTemp);
  u8g2.drawXBMP( 0, 17, Temperatura_Ikona_sirina, Temperatura_Ikona_visina, Temperatura_Ikona);
}
void ispisVanjskeVlaz() {
  u8g2.setFont(u8g2_font_helvB12_tr);
  u8g2.setCursor(25, 15);
  u8g2.print("VANJSKA");
  u8g2.setFont(u8g2_font_logisoso30_tf);
  u8g2.setCursor(36, 58);
  u8g2.print(vanVlaz);
  u8g2.drawXBMP( 0, 17, Vlaznost_Ikona_sirina, Vlaznost_Ikona_visina, Vlaznost_Ikona);
}

void ispisPritiska(){
  u8g2.setFont(u8g2_font_helvB12_tr);
  u8g2.setCursor(25, 15);
  u8g2.print("VANJSKA");
  u8g2.setFont(u8g2_font_helvB12_tr);
  u8g2.setCursor(25, 32);
  u8g2.print("PRITISAK:");
  u8g2.setFont(u8g2_font_logisoso18_tr);
  u8g2.setCursor(5, 58); //20
  u8g2.print(pritisak);
}

void ispisNadmVis(){
  u8g2.setFont(u8g2_font_helvB12_tr);
  u8g2.setCursor(25, 15);
  u8g2.print("VANJSKA");
  u8g2.setFont(u8g2_font_helvB12_tr);
  u8g2.setCursor(5, 32);
  u8g2.print("NADM. VISINA:");
  u8g2.setFont(u8g2_font_logisoso18_tr);
  u8g2.setCursor(30, 58); //28
  u8g2.print(nadmVisina);
}

void ispisLux(){
  u8g2.setFont(u8g2_font_helvB12_tr);
  u8g2.setCursor(25, 15);
  u8g2.print("VANJSKA");
  u8g2.setFont(u8g2_font_helvB12_tr);
  u8g2.setCursor(45, 32);
  u8g2.print("LUX:");
  u8g2.setFont(u8g2_font_logisoso18_tr);
  u8g2.setCursor(35, 58);
  u8g2.print(lux);
}