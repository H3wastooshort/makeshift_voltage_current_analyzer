volatile bool adc_coversion_done = false;
volatile bool adc_coversion_overflow = false;
void ARDUINO_ISR_ATTR adcComplete() {
  if (adc_coversion_done) adc_coversion_overflow = true;
  adc_coversion_done = true;
}

bool blinky = 0;
uint16_t tick = 0;
adc_continuous_data_t *result = NULL;
void handle_adc(File *file) {
  if (adc_coversion_done) {
    adc_coversion_done = false;
    if (analogContinuousRead(&result, 0)) {
      for (uint8_t i = 0; i < sizeof(pins_to_read) / sizeof(pins_to_read[0]); i++) {
        uint16_t time = micros();

        uint8_t flags = 0;
        if (adc_coversion_overflow) flags |= 0b00000001;

        //[16 millivolts] [8 flags] [8 pin] [16 timecode]
        uint32_t buf = 0;
        buf |= result[i].avg_read_mvolts & 0xFFFF;
        buf |= (result[i].pin & 0xFF) << 16;
        buf |= (flags & 0xFF) << 24;

        file->write((uint8_t *)&buf, 4);
        file->write((uint8_t *)&time, 2);

        tick++;
        if (tick > actual_sr) {  //every second
          tick = 0;
          file->flush();
          digitalWrite(led_pin, blinky);
          blinky ^= 1;
        }
      }
    } else Serial.println("ADC read error.");


    if (adc_coversion_overflow) {
      Serial.println('O');
      adc_coversion_overflow = false;
    }
  }
}
