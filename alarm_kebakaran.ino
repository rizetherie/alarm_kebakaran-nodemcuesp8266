#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <ThingerESP8266.h>
#include <ESP8266WiFi.h>

// Konfigurasi WiFi
#define SSID "V"
#define PASSWORD "123456789"

// Konfigurasi Thinger.io
#define USERNAME "bilqis"
#define DEVICE_ID "alarm_kebakaran"
#define DEVICE_CREDENTIAL "E89L5N_9nZcyB5v7"

// Inisialisasi objek hd44780_I2Cexp dengan alamat I2C dan ukuran LCD
hd44780_I2Cexp lcd;

const int mq2Pin = A0;     // Pin sensor MQ-2
const int flamePin = D0;   // Pin sensor flame
const int buzzerPin = D3;  // Pin buzzer

ThingerESP8266 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

// Variabel untuk menyimpan status dan nilai-nilai
String statusText = "Safe";
int mq2Value = 0;
int flameValue = 0;

void setup() {
  Serial.begin(9600);
  
  lcd.begin(16, 2);  // Inisialisasi LCD dengan 16 kolom dan 2 baris
  lcd.print("Prototipe Fire Alarm NodeMCU");  // Teks selamat datang
  lcd.setCursor(0, 1);
  lcd.print("ESP8266 dengan Notifikasi Telegram");
  delay(2000);
  
  pinMode(mq2Pin, INPUT);
  pinMode(flamePin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  
  lcd.clear();  // Bersihkan layar LCD
  lcd.print("Ready");
  lcd.setCursor(0, 1);
  lcd.print("For Detect");
  delay(2000);

  // Menghubungkan ke jaringan WiFi
  thing.add_wifi(SSID, PASSWORD);
  
  // Menambahkan resource status ke Thinger.io
  thing["status"] >> [](pson& out) {
    out["statusText"] = statusText;
    out["mq2Value"] = mq2Value;
    out["flameValue"] = flameValue;
  };
}

void loop() {
  lcd.clear();  // Bersihkan layar LCD

  Serial.print("MQ-2 Value: ");
  mq2Value = analogRead(mq2Pin);       // Baca nilai analog dari sensor MQ-2
  Serial.println(mq2Value);

  Serial.print("Flame Value: ");
  flameValue = digitalRead(flamePin);  // Baca nilai digital dari sensor flame
  Serial.println(flameValue);
  
  if (mq2Value >= 500) {
    // Terdeteksi asap
    statusText = "Gas Detected";
    lcd.print("Gas Detected");
    digitalWrite(buzzerPin, HIGH);  // Nyalakan buzzer
    thing.call_endpoint("FireAlarm");
    delay(1000);
    digitalWrite(buzzerPin, LOW);   // Matikan buzzer
    delay(1000);
  } else if (flameValue == HIGH) {
    // Terdeteksi Api
    statusText = "Fire Detected";
    lcd.print("Fire Detected");
    digitalWrite(buzzerPin, HIGH);  // Nyalakan buzzer
    thing.call_endpoint("flame_detect");
    delay(1000);
    digitalWrite(buzzerPin, LOW);   // Matikan buzzer
    delay(1000);
  } else {
    // Tidak ada deteksi
    statusText = "Safe";
    lcd.print("Safe");
    delay(1000);
  }

  // Kirim data ke Thinger.io
  thing.handle();
  
  delay(1000);
}
