#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <dht.h>
#include <LowPower.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp; // Deklarisanje objekta bmp

#define LUX A0 // Definisanje fotootpornika na analognom pinu 0
const float Rref = 10000; // Otpornost fotootpornika 10K
const float RL10 = 50; // Otpornost fotootpornika na vrijednosti 10 lux
const float GAMMA = 0.7;

#define dataPin 8 // DHT22 senzor na digitalnom pinu 8
dht DHT; // Kreira se DHT objekat

RF24 radio(10, 9); // CE, CSN pinovi za komunikaciju
const byte address[6] = "00001"; // Adresa za komunikaciju sa unutrasnjom jedinicom

char thChar[32] = ""; // Deklarisanje niza od 32 elementa
String thString = ""; // Deklarisanje praznog stringa u koji ce se unositi podaci sa senzora

void setup() {
  // Inicijalizacija komunikacije
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
  bmp.begin(0x76);
}

void loop() {
  int citajPodatke = DHT.read22(dataPin); // Citanje podataka sa senzora
  int t = DHT.temperature; // Uzima se vrijednost temperatura
  int h = DHT.humidity; // Uzima se vrijednost vlaznosti

  int analognaVrijednost = analogRead(LUX); // Citanje analogne vrijednosti sa senzora
  float Vout = analognaVrijednost / 1023.*5; // Racunanje napona na osnovu analogne vrijednosti
  float R = Rref*(1/((5/Vout)-1));
  int lux = pow(RL10*1e3*pow(10,GAMMA)/R,(1/GAMMA)); // Vrijednost luksa

  // Konverzija iz int u string
  thString = String(t) + String(h) + String(int(bmp.readPressure()*0.01+17)) + String(int(bmp.readAltitude(1013.25)+80)) + String(lux);
  thString.toCharArray(thChar, 20);
  
  // Slanje podataka bezicno na unutrasnju jedinicu
  // Podaci se salju tri puta da bi se osiguralo da stignu na unutrasnju jedinicu ukoliko je kontroler zauzet u datom trenutku
  for (int i = 0; i <= 3; i++) {
    radio.write(&thChar, sizeof(thChar));
    delay(1000);
  }
  
  // Rezim niske potrosnje "deep sleep" na 30 minuta, 225 prolazaka kroz petlju * 8s = 1800s
  for (int sleepCounter = 1; sleepCounter > 0; sleepCounter--)
  {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
}