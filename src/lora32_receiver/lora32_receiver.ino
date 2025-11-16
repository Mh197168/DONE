#include <SPI.h>
#include <LoRa.h>

// Pin mapping for the TTGO LoRa32 (433 MHz) board
static const int LORA_SCK_PIN = 5;
static const int LORA_MISO_PIN = 19;
static const int LORA_MOSI_PIN = 27;
static const int LORA_SS_PIN = 18;
static const int LORA_RST_PIN = 23;
static const int LORA_DIO0_PIN = 26;

static const long LORA_FREQUENCY = 433E6;

void setup()
{
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println(F("LoRa32 receiver"));

  SPI.begin(LORA_SCK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN, LORA_SS_PIN);
  LoRa.setPins(LORA_SS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);

  if (!LoRa.begin(LORA_FREQUENCY)) {
    Serial.println(F("[LoRa] Failed to initialise radio"));
    while (true) {
      delay(1000);
    }
  }

  Serial.println(F("[LoRa] Receiver ready"));
}

void loop()
{
  int packetSize = LoRa.parsePacket();
  if (!packetSize) {
    return;
  }

  String payload;
  while (LoRa.available()) {
    payload += (char)LoRa.read();
  }

  const long rssi = LoRa.packetRssi();
  const float snr = LoRa.packetSnr();

  Serial.println(F("--- Incoming packet ---"));
  Serial.print(F("Payload: "));
  Serial.println(payload);
  Serial.print(F("RSSI: "));
  Serial.print(rssi);
  Serial.print(F(" dBm, SNR: "));
  Serial.print(snr);
  Serial.println(F(" dB"));
  Serial.println();
}
