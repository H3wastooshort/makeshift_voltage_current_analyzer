void print_sd_stats(Stream &ser) {
  uint64_t cardSize = SD_MMC.cardSize();
  uint16_t totalBytes = SD_MMC.totalBytes();
  uint16_t usedBytes = SD_MMC.usedBytes();
  uint64_t bytes_free = totalBytes - usedBytes;

  ser.printf(F("SD Card Size: %lluMB"), cardSize / (1024 * 1024));
  ser.println();
  ser.printf(F("Total space: %lluMB"), totalBytes / (1024 * 1024));
  ser.println();
  ser.printf(F("Used space: %lluMB"), usedBytes / (1024 * 1024));
  ser.println();

  float mb_per_min = (bytes_per_second / 1E6) * 60;
  ser.print(F("Bitrate [MB/min]: "));
  ser.println(mb_per_min);
  float rec_mins = bytes_free / (bytes_per_second * 60.0);
  ser.print(F("Recording time left [min]: "));
  ser.println(rec_mins);
}

void parse_serial_cmd(Stream &ser) {
  if (!ser.available()) return;

  uint64_t time = micros();

  char cmd = ser.read();
  switch (cmd) {
    case 't':
      ser.print(F("Time [µS]: "));
      ser.println(time);
      break;
    case 's': print_sd_stats(ser); break;
  }
}
