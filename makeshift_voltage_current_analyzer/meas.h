
/*
Format:
[32 time][8 n_pins] [8 pin][16 millivolts]... 
2nd half repeats for each pin
*/

const uint8_t HEADER_SIZE = 5;
constexpr uint16_t ENTRY_SIZE = HEADER_SIZE + (3 * n_pins);

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
      adc_buf[adc_buf_write_idx + 1] = result[i].avg_read_mvolts & 0xFF;
      adc_buf[adc_buf_write_idx + 2] = (result[i].avg_read_mvolts >> 1) & 0xFF;
      adc_buf_write_idx += 3;  //data 3bytes * n_pins

      update_stats(i, result[i].avg_read_mvolts);
    }
  } else adc_error = true;
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

    tick += ENTRY_SIZE;
    if (tick > bytes_per_second) {  //every second
      tick = 0;
      file->flush();
      digitalWrite(led_pin, blinky);
      blinky ^= 1;
    }

    adc_buf_read_idx += ENTRY_SIZE;
    bytes_this_round += ENTRY_SIZE;

    if (bytes_this_round == ADC_BUF_SIZE) Serial.println("ADC OVERFLOW");
    if (bytes_this_round > ADC_BUF_SIZE * 3) {
      Serial.println("Written 3 full buffers in one go! SD Card can't keep up with ADC!!");
      break;
    }
  }
  if (adc_error) {
    Serial.println("ADC ERROR");
    adc_error = false;
  }
}
