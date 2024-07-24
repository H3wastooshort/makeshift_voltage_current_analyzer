#include <FS.h>
#include <SD.h>
#include <SPI.h>

#include "conf.h"
#include "meas.h"

File file;

void setup() {
  Serial.begin(115200);
  Serial.print(F("MVCA, compiled "));
  Serial.print(__DATE__);
  Serial.write(' ');
  Serial.println(__TIME__);

  pinMode(voltage_pin, INPUT);
  pinMode(current_pin, INPUT);
  analogReadResolution(12);
  analogSetAttenuation(ADC_ATTEN_DB_11);

  //copied from the SD_Test example
  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));

  file = SD.open(filename, FILE_APPEND);
  if (!file) {
    Serial.println(F("Failed to open file for appending"));
    return;
  }
  //end of copied section
}

void loop() {
}
