
/*
Format:
[32 time][1 n_pins] [1 pin][2 millivolts]... 
2nd half repeats for each pin
*/

constexpr uint8_t ENTRY_SIZE = 5 + (3 * n_pins);

constexpr size_t ADC_BUF_SIZE = ENTRY_SIZE * 1000;
uint8_t adc_buf[ADC_BUF_SIZE] = { 0 };
bool adc_error = false;

size_t adc_buf_write_idx = 0;
adc_continuous_data_t *result = NULL;
void ARDUINO_ISR_ATTR adcComplete() {
  uint32_t time = micros();
  if (analogContinuousRead(&result, 0)) {
    if (adc_buf_write_idx + ENTRY_SIZE >= ADC_BUF_SIZE) adc_buf_write_idx = 0;

    for (uint8_t i = 0; i < 4; i++) adc_buf[adc_buf_write_idx + i] = (time >> (8 * i)) & 0xFF;  //write 4 bytes of time
    adc_buf[adc_buf_write_idx + 4] = n_pins;                                                    //write 1 byte n_pins
    adc_buf_write_idx += 5;                                                                     //header 5 bytes

    for (uint8_t i = 0; i < n_pins; i++) {
      adc_buf[adc_buf_write_idx + 0] = result[i].pin;
      adc_buf[adc_buf_write_idx + 1] = (result[i].pin >> 0) & 0xFF;
      adc_buf[adc_buf_write_idx + 2] = (result[i].pin >> 1) & 0xFF;
      adc_buf_write_idx += 3;  //data 3bytes * n_pins
    }
  }
  else adc_error = true;
}

bool blinky = 0;
uint32_t tick = 0;
size_t adc_buf_read_idx = 0;
constexpr size_t bytes_per_second = actual_sr * ENTRY_SIZE;
void handle_adc(File *file) {
  size_t bytes_this_round = 0;
  while (adc_buf_read_idx != adc_buf_write_idx) {
    if (adc_buf_read_idx + ENTRY_SIZE >= ADC_BUF_SIZE) adc_buf_read_idx = 0;

    file->write(&adc_buf[adc_buf_read_idx], ENTRY_SIZE);

    tick++;
    if (tick > bytes_per_second) {  //every second
      tick = 0;
      digitalWrite(led_pin, blinky);
      blinky ^= 1;
    }

    adc_buf_read_idx++;
    bytes_this_round++;

    if (bytes_this_round > ADC_BUF_SIZE * 10) {
      Serial.println("Written 10 full buffers in one go! SD Card can't keep up with ADC!!");
      ESP.restart();
    }
  }
  Serial.println(bytes_this_round);
}
