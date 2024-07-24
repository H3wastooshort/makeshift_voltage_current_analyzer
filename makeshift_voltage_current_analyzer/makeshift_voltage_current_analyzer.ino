#include <FS.h>
#include <SD_MMC.h>

#include "conf.h"
#include "meas.h"

File file;

void setup() {
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);
  Serial.begin(115200);
  Serial.print(F("MVCA, compiled "));
  Serial.print(__DATE__);
  Serial.write(' ');
  Serial.println(__TIME__);


  //copied from the SD_Test example
  if (!SD_MMC.begin("/sdcard", true)) {
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
  Serial.println(F("SD Card OK"));

  for (uint8_t i = 0; i < sizeof(pins_to_read); i++) pinMode(pins_to_read[i], INPUT);
  analogContinuousSetWidth(12);
  analogContinuousSetAtten(ADC_11db);
  if (!analogContinuous(pins_to_read, sizeof(pins_to_read), n_samples_avg, sample_rate, &adcComplete)) Serial.println(F("analogContinuous FAILED"));
  if (!analogContinuousStart()) Serial.println(F("analogContinuousStart FAILED"));
  Serial.println(F("started ADC"));

  digitalWrite(led_pin, HIGH);
}


void loop() {
  handle_adc(&file);
}
