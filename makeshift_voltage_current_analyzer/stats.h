bool no_update_stats = false;
struct pin_stats_t {
  uint16_t min = 0xFFFF;
  uint16_t max = 0;

  uint64_t sum = 0;
  uint32_t n = 0;
};

pin_stats_t pin_stats[n_pins];

inline void update_stats(uint8_t idx, uint16_t mv) {
  if (no_update_stats) return;
  pin_stats[idx].min = min(pin_stats[idx].min, mv);
  pin_stats[idx].max = max(pin_stats[idx].max, mv);
  pin_stats[idx].sum += mv;
  pin_stats[idx].n++;
}

void output_stats() {
  no_update_stats = true;
  for (uint8_t i = 0; i < n_pins; i++) {
    uint16_t min = pin_stats[i].min;
    uint16_t max = pin_stats[i].max;
    uint64_t sum = pin_stats[i].sum;
    uint32_t n = pin_stats[i].n;

    pin_stats[i].min = 0xFFFF;
    pin_stats[i].max = 0;
    pin_stats[i].sum = 0;
    pin_stats[i].n = 0;

    uint32_t avg = 0;
    if (n > 0) avg = sum / n;

    uint8_t pin = pins_to_read[i];

    Serial.printf("Pin %02u: %04umV (%04umV-%04umV) over %u samples", pin, avg, min, max, n);
    Serial.println();
  }
  no_update_stats = false;
}
