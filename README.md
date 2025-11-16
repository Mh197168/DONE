# LoRa Sensor Test Sketches

This repository contains two Arduino sketches that let you test LoRa communication between an ESP32-C6 equipped with an RA-02 (SX1278) LoRa module and a TTGO LoRa32 433 MHz board. The ESP32-C6 node reads a DHT temperature/humidity sensor together with analog TDS and pH probes, packages the readings into a JSON payload, and transmits them via LoRa. The LoRa32 board acts as a receiver and prints the payload details to the serial monitor.

## Contents

- `src/esp32c6_lora_sensor_sender/esp32c6_lora_sensor_sender.ino` – Sensor node for the ESP32-C6 + SX1278 setup.
- `src/lora32_receiver/lora32_receiver.ino` – Receiver node for the TTGO LoRa32 board.

## Required Arduino libraries

Install the following libraries via the Arduino Library Manager (or your preferred package manager):

- **LoRa** by Sandeep Mistry
- **DHT sensor library** by Adafruit
- **Adafruit Unified Sensor** by Adafruit (dependency for the DHT library)

## Wiring summary

### ESP32-C6 sensor node

Adjust the pin assignments in the sketch so that they match your wiring. The defaults assume:

| ESP32-C6 pin | Peripheral      |
|--------------|-----------------|
| GPIO6        | SX1278 SCK      |
| GPIO5        | SX1278 MISO     |
| GPIO7        | SX1278 MOSI     |
| GPIO4        | SX1278 NSS/CS   |
| GPIO8        | SX1278 RESET    |
| GPIO3        | SX1278 DIO0     |
| GPIO18       | DHT22 data      |
| GPIO1 (ADC)  | TDS sensor      |
| GPIO2 (ADC)  | pH sensor       |

> ℹ️ *Reassign these pins if your module is wired differently or if any of the chosen GPIOs conflict with your design.*

### TTGO LoRa32 receiver

The receiver sketch uses the built-in LoRa pin mapping for the TTGO LoRa32 433/470 MHz board:

| LoRa32 pin | SX1278 signal |
|------------|----------------|
| 5          | SCK            |
| 19         | MISO           |
| 27         | MOSI           |
| 18         | NSS/CS         |
| 23         | RESET          |
| 26         | DIO0           |

## Usage

1. Open the sender sketch in the Arduino IDE, select the appropriate ESP32-C6 board definition, and upload it to the microcontroller connected to the sensors and external RA-02 module.
2. Open the receiver sketch, choose the TTGO LoRa32 board profile, and upload it to the receiving board.
3. Open the Serial Monitor (115200 baud) on both devices to observe sensor readings and received packets.
4. Fine-tune the TDS and pH calibration constants within the sender sketch based on your probe calibration data.

The transmitted payload is a JSON document that includes air temperature, relative humidity, an estimated water temperature (currently mirroring the air temperature), TDS (ppm), and pH. The receiver prints the raw payload together with packet RSSI and SNR values.

## Customisation tips

- Increase the `sendIntervalMs` constant to reduce airtime usage once testing is complete.
- Update `PH_NEUTRAL_VOLTAGE` and `PH_SLOPE` after calibrating your pH sensor with buffer solutions.
- If you add a dedicated water temperature sensor, replace `readWaterTemperatureC()` with a routine that reads from the appropriate pin.
- To add acknowledgement or downlink support, extend the receiver sketch to transmit responses when `LoRa.parsePacket()` detects incoming data.
