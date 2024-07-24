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

  pinMode(voltage_pin, INPUT);
  pinMode(current_pin, INPUT);
  analogReadResolution(12);
  analogSetAttenuation(ADC_ATTENDB_MAX);
}

bool blinky = 0;
void loop() {
  const data_rec_t rec = make_measurement();
  file.write((const uint8_t*)(const void*)&rec, sizeof(rec));
  digitalWrite(led_pin, blinky);
  blinky ^= 1;
}
