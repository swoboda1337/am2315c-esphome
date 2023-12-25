AM2315C driver taken from [https://github.com/RobTillaart/AM2315C](https://github.com/RobTillaart/AM2315C) and ported to an esphome component.

Example yaml to use in esphome device config:

    external_components:
      - source:
          type: local
          path: components

    switch:
      - platform: gpio
        name: "Humidifier"
        pin: 32

    sensor:
      - platform: am2315c
        temperature:
          name: "Duct Temperature"
        humidity:
          name: "Duct Humidity"
        update_interval: 60s

    i2c:
      sda: 21
      scl: 22

