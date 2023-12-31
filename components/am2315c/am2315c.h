#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace am2315c {

class AM2315C : public PollingComponent, public i2c::I2CDevice {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;
  void update() override;

  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { temperature_sensor_ = temperature_sensor; }
  void set_humidity_sensor(sensor::Sensor *humidity_sensor) { humidity_sensor_ = humidity_sensor; }

 protected:
  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *humidity_sensor_{nullptr};

 private:
  uint8_t read_status();
  uint8_t reset_sensor(); 
  bool reset_register(uint8_t reg);
  bool convert(uint8_t *data, float &humidity, float &temperature);
  uint8_t crc8(uint8_t *data, uint8_t len);
  uint32_t last_read;
};

}  // namespace am2320
}  // namespace esphome