#include "am2315c.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace am2315c {

static const char *const TAG = "am2315c";

uint8_t AM2315C::crc8(uint8_t *data, uint8_t len) {
  uint8_t crc = 0xFF;
  while(len--) {
    crc ^= *data++;
    for (uint8_t i = 0; i < 8; i++) {
      if (crc & 0x80) {
        crc <<= 1;
        crc ^= 0x31;
      } else {
        crc <<= 1;
      }
    }
  }
  return crc;
}

uint8_t AM2315C::read_status() {
  uint8_t data = 0;
  if (this->read(&data, 1) != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "Read failed!");
    return false;
  }
  return data;
}

uint8_t AM2315C::reset_sensor() {
  // see datasheet 7.4 point 1, use with care.
  uint8_t count = 255;
  if ((read_status() & 0x18) != 0x18)
  {
    count++;
    if (reset_register(0x1B)) count++;
    if (reset_register(0x1C)) count++;
    if (reset_register(0x1E)) count++;
    delay(10);
  }
  return count;
}

bool AM2315C::reset_register(uint8_t reg) {
  //  code based on demo code sent by www.aosong.com
  //  no further documentation.
  //  0x1B returned 18, 0, 4
  //  0x1C returned 18, 65, 0
  //  0x1E returned 18, 8, 0
  //    18 seems to be status register
  //    other values unknown.
  uint8_t data[3];
  data[0] = reg;
  data[1] = 0;
  data[2] = 0;
  ESP_LOGD(TAG, "Reset register: 0x%02x", reg);
  if (this->write(data, 3) != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "Write failed!");
    return false;
  }
  delay(5);
  if (this->read(data, 3) != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "Read failed!");
    return false;
  }
  delay(10);
  data[0] = 0xB0 | reg;
  if (this->write(data, 3) != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "Write failed!");
    return false;
  }
  delay(5);
  return true;
}

bool AM2315C::convert(uint8_t *data, float &humidity, float &temperature) {
  uint32_t raw;
  raw = (data[1] << 12) | (data[2] << 4) | (data[3] >> 4);
  humidity = raw * 9.5367431640625e-5;
  raw = ((data[3] & 0x0F) << 16) | (data[4] << 8) | data[5];
  temperature = raw * 1.9073486328125e-4 - 50;
  return crc8(data, 6) == data[6];
}

void AM2315C::update() {
  //  do not read too fast
  if ((last_read > 0) && (esphome::millis() - last_read < 2000)) {
    ESP_LOGW(TAG, "Reading too often!");
    return;
  }
  last_read = esphome::millis();

  // reset
  reset_sensor();

  // request
  uint8_t data[3];
  data[0] = 0xAC;
  data[1] = 0x33;
  data[2] = 0x00;
  if (this->write(data, 3) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Write failed!");
    this->mark_failed();
    return;
  }
  
  // wait
  this->set_timeout(160, [this]() {
    // check
    if ((read_status() & 0x80) == 0x80) {
      ESP_LOGE(TAG, "HW still busy!");
      this->mark_failed();
      return;
    }
    
    // read
    uint8_t data[7];
    if (this->read(data, 7) != i2c::ERROR_OK) {
      ESP_LOGE(TAG, "Read failed!");
      this->mark_failed();
      return;
    }
  
    // check for all zeros
    bool zeros = true;
    for (int i = 0; i < 7; i++) {
      zeros = zeros && (data[i] == 0);
    }
    if (zeros) {
      ESP_LOGW(TAG, "Data all zeros!");
      this->status_set_warning();
      return;
    }  
    
    // convert
    float temperature = 0.0;
    float humidity = 0.0;
    if (convert(data, humidity, temperature)) {
      if (this->temperature_sensor_ != nullptr)
        this->temperature_sensor_->publish_state(temperature);
      if (this->humidity_sensor_ != nullptr)
        this->humidity_sensor_->publish_state(humidity);
      this->status_clear_warning();
    } else {
      ESP_LOGW(TAG, "CRC failed!");
      this->status_set_warning();
    }
  });
}

void AM2315C::setup() {
  ESP_LOGCONFIG(TAG, "Setting up AM2315C...");
  last_read = 0;
}

void AM2315C::dump_config() {
  ESP_LOGD(TAG, "AM2315C:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with AM2315C failed!");
  }
  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
  LOG_SENSOR("  ", "Humidity", this->humidity_sensor_);
}

float AM2315C::get_setup_priority() const { return setup_priority::DATA; }

}  // namespace am2320
}  // namespace esphome
