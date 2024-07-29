#include <FS.h>
#include <SD_MMC.h>
#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

#include "conf.h"
#include "stats.h"
#include "meas.h"

File file;
void setup() {
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);
  pinMode(0, INPUT);
  Serial.begin(115200);
  Serial.print(F("MVCA, compiled "));
  Serial.print(__DATE__);
  Serial.write(' ');
  Serial.println(__TIME__);


  //copied from the SD_Test example
  if (!SD_MMC.begin("/sdcard", false)) {
    Serial.println(F("Card Mount Failed"));
    while (1) {
      digitalWrite(led_pin, LOW);
      delay(100);
      digitalWrite(led_pin, HIGH);
      delay(100);
    }
  }

  uint8_t cardType = SD_MMC.cardType();
  Serial.print(F("SD Card Type: "));
  switch (cardType) {
    case CARD_NONE:
      Serial.println(F("No SD card attached"));
      break;
    case CARD_MMC:
      Serial.println(F("MMC"));
      break;
    case CARD_SD:
      Serial.println(F("SDSC"));
      break;
    case CARD_SDHC:
      Serial.println(F("SDHC"));
      break;
    default:
      Serial.println(F("UNKNOWN"));
      break;
  }

  uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  Serial.printf("Total space: %lluMB\n", SD_MMC.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024));

  file = SD_MMC.open(filename, FILE_APPEND);
  if (!file) {
    Serial.println(F("Failed to open file for appending"));
    while (1) {
      digitalWrite(led_pin, LOW);
      delay(200);
      digitalWrite(led_pin, HIGH);
      delay(200);
    }
  }
  //end of copied section
  uint64_t bytes_free = SD_MMC.totalBytes() - SD_MMC.usedBytes();
  float mb_per_min = (bytes_per_second / 1E6) * 60;
  Serial.print(F("Bitrate [MB/min]: "));
  Serial.println(mb_per_min);
  float rec_mins = bytes_free / (bytes_per_second * 60.0);
  Serial.print(F("Recording time left [min]: "));
  Serial.println(rec_mins);

  for (uint8_t i = 0; i < HEADER_SIZE; i++) file.write((uint8_t)0x00);  //zeroed-out header means reboot
  Serial.println(F("SD Card OK"));


  delay(1000);
  if (!digitalRead(0)) {
    SerialBT.begin("MVCA");
    while (!digitalRead(0)) {  //pairing crashed the ADC interrupt i think, hold GPIO0 low to enable and pair BT.
      digitalWrite(led_pin, HIGH);
      delay(50);
      digitalWrite(led_pin, LOW);
      delay(50);
    }
    Serial.println(F("started BT"));
  }

  for (uint8_t i = 0; i < sizeof(pins_to_read); i++) pinMode(pins_to_read[i], INPUT);
  analogContinuousSetWidth(12);
  analogContinuousSetAtten(ADC_11db);
  if (!analogContinuous(pins_to_read, sizeof(pins_to_read), n_samples_avg, sample_rate, &adcComplete)) Serial.println(F("analogContinuous FAILED"));
  if (!analogContinuousStart()) Serial.println(F("analogContinuousStart FAILED"));
  Serial.println(F("started ADC"));

  digitalWrite(led_pin, HIGH);
}

uint64_t last_stats = 0;
void loop() {
  handle_adc(&file);
  if (millis() - last_stats > 500) {
    last_stats = millis();
    output_stats();
  }
}
