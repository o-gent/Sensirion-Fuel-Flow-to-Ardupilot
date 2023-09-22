# Sensirion slf3s-4000b to Ardupilot

A hacky project to relay information from a SLF3s-4000b fuel flow sensor to Ardupilot via an ESP32-S3. The ESP reads in the i2c data from the sensor, then just sends a GPIO pulse to ardupilot for every milli-litre consumed

Also meant to record detailed data to an SD card but haven't got that bit working yet.
