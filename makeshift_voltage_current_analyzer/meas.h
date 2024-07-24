struct data_rec_struct {
  uint32_t time;
  uint16_t voltage_mv;
  uint16_t current_mv;
};
using data_rec_t = struct data_rec_struct;


data_rec_t make_measurement() {
  uint32_t voltage_mv = 0, current_mv = 0;
  for (uint16_t i = 0; i < n_samples_avg; i++) {
    voltage_mv += analogReadMilliVolts(voltage_pin);
    current_mv += analogReadMilliVolts(current_pin);
  }
  voltage_mv /= n_samples_avg;
  current_mv /= n_samples_avg;

  data_rec_t rec;
  rec.time = micros();
  rec.voltage_mv = voltage_mv;
  rec.current_mv = current_mv;

  return rec;
}
