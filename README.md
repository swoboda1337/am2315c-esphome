AM2315C driver taken from [https://github.com/RobTillaart/AM2315C](https://github.com/RobTillaart/AM2315C) and ported to an esphome component.

Example yaml to use in esphome device config:

    external_components:
      - source:
          type: git
          url: https://github.com/swoboda1337/am2315c-esphome
          ref: main

    sensor:
      - platform: am2315c
        temperature:
          name: "Temperature"
        humidity:
          name: "Humidity"
        update_interval: 60s

    i2c:
      sda: 21
      scl: 22

