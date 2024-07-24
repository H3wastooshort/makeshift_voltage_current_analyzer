volatile bool adc_coversion_done = false;
void ARDUINO_ISR_ATTR adcComplete() {
  adc_coversion_done = true;
}


struct data_rec_struct {
  uint8_t pin = 0xFF;
  uint32_t time;
  uint16_t millivolt;
};
using data_rec_t = struct data_rec_struct;



bool blinky = 0;
uint16_t tick = 0;
adc_continuous_data_t *result = NULL;
void handle_adc(File *file) {
  if (adc_coversion_done) {
    adc_coversion_done = false;
    if (analogContinuousRead(&result, 0)) {
      for (uint8_t i = 0; i < sizeof(pins_to_read); i++) {
        data_rec_t rec;
        rec.pin = result[i].pin;
        rec.time = micros();
        rec.millivolt = result[i].avg_read_mvolts;
        file->write((const uint8_t *)(const void *)&rec, sizeof(rec));

        tick++;
        if (tick > actual_sr) {  //every second
          tick = 0;
          file->flush();
          digitalWrite(led_pin, blinky);
          blinky ^= 1;
        }
      }
    }
  }
}
