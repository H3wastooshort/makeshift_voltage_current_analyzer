struct stats_stuct {
  uint16_t min = 0xFF;
  uint16_t max = 0x00;

  uint64_t sum = 0;
  uint64_t n = 0;
};
using pin_stats_t = struct stats_stuct;

pin_stats_t pin_stats[n_pins];

inline void update_stats(uint8_t idx, uint16_t mv) {
  pin_stats[idx].min = min(pin_stats[idx].min, mv);
  pin_stats[idx].max = max(pin_stats[idx].max, mv);
  pin_stats[idx].sum += mv;
  pin_stats[idx].n++;
}

void output_stats() {
  for (uint8_t i = 0; i < n_pins; i++) {
    auto min = pin_stats[i].min;
    auto max = pin_stats[i].max;
    auto sum = pin_stats[i].sum;
    auto n = pin_stats[i].n;

    pin_stats[i].min = 0;
    pin_stats[i].max = 0xFF;
    pin_stats[i].sum = 0;
    pin_stats[i].n = 0;

    uint16_t avg = pin_stats[i].sum / pin_stats[i].n;

    uint8_t pin = pins_to_read[i];

    char buf[256];
    snprintf(buf, sizeof(buf), "Pin %02d: %04dmV (%04dmV-%04dmV) over %d samples", pin, avg, min, max, n);
    Serial.println(buf);
  }
}
