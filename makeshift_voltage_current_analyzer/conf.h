uint8_t pins_to_read[] = { 34, 35 };
constexpr uint8_t n_pins = sizeof(pins_to_read) / sizeof(pins_to_read[0]);

const uint8_t led_pin = 33;

const uint32_t sample_rate = 200000;
const uint32_t n_samples_avg = 20;
constexpr uint32_t actual_sr = sample_rate / n_samples_avg;

const char* filename = "/recording.bin";
