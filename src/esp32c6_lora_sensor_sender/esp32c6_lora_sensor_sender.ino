#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>

// -------- Pin configuration --------
// Update these pin assignments so they match your ESP32-C6 wiring.
static const int LORA_SCK_PIN = 6;   // GPIO6  -> SX1278 SCK
static const int LORA_MISO_PIN = 5;  // GPIO5  -> SX1278 MISO
static const int LORA_MOSI_PIN = 7;  // GPIO7  -> SX1278 MOSI
static const int LORA_SS_PIN = 4;    // GPIO4  -> SX1278 NSS (CS)
static const int LORA_RST_PIN = 8;   // GPIO8  -> SX1278 RESET
static const int LORA_DIO0_PIN = 3;  // GPIO3  -> SX1278 DIO0

static const int DHT_PIN = 18;       // GPIO18 -> DHT22 data
static const int TDS_PIN = 1;        // GPIO1  -> Analog input for TDS sensor
static const int PH_PIN = 2;         // GPIO2  -> Analog input for pH sensor

// Choose the correct DHT sensor model for your build.
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// LoRa modem configuration
static const long LORA_FREQUENCY = 433E6;     // 433 MHz
static const long LORA_SIGNAL_BW = 125E3;     // 125 kHz bandwidth
static const int LORA_SPREADING_FACTOR = 9;   // SF9 strikes a balance between range and speed
static const int LORA_TX_POWER = 17;          // dBm (respect regional regulations)

// TDS sensor calibration
static const float TDS_VREF = 3.3f;           // ESP32 ADC reference voltage
static const float TDS_AREF = 4095.0f;        // 12-bit ADC
static const float TDS_COMPENSATION_COEFF = 0.02f;

// pH sensor calibration values (adjust to your probe)
static const float PH_NEUTRAL_VOLTAGE = 1.65f;  // Voltage at pH 7
static const float PH_SLOPE = -0.18f;           // Volts per pH unit (example value)

unsigned long lastSendMillis = 0;
const unsigned long sendIntervalMs = 3000;  // Send every 3 seconds

float readWaterTemperatureC()
{
  // If you have a dedicated water temperature sensor, read it here.
  // For now, use the air temperature from the DHT as an approximation.
  return dht.readTemperature();
}

float readTDS()
{
  const int raw = analogRead(TDS_PIN);
  const float voltage = (raw / TDS_AREF) * TDS_VREF;

  // Simple water temperature compensation using the DHT temperature
  const float waterTemp = readWaterTemperatureC();
  const float compensationCoefficient = 1.0 + TDS_COMPENSATION_COEFF * (waterTemp - 25.0f);
  const float compensatedVoltage = voltage / compensationCoefficient;

  // Empirical conversion from voltage to TDS in ppm
  float tds = (133.42f * compensatedVoltage * compensatedVoltage * compensatedVoltage
               - 255.86f * compensatedVoltage * compensatedVoltage
               + 857.39f * compensatedVoltage);
  tds = tds < 0 ? 0 : tds;
  return tds;
}

float readPH()
{
  const int raw = analogRead(PH_PIN);
  const float voltage = (raw / TDS_AREF) * TDS_VREF;
  // Convert voltage to pH value using calibration constants
  const float pH = 7.0f + (voltage - PH_NEUTRAL_VOLTAGE) / PH_SLOPE;
  return pH;
}

bool initLoRa()
{
  SPI.begin(LORA_SCK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN, LORA_SS_PIN);
  LoRa.setPins(LORA_SS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);

  if (!LoRa.begin(LORA_FREQUENCY)) {
    Serial.println(F("[LoRa] Failed to start LoRa modem"));
    return false;
  }

  LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR);
  LoRa.setSignalBandwidth(LORA_SIGNAL_BW);
  LoRa.setTxPower(LORA_TX_POWER);

  Serial.println(F("[LoRa] Modem initialised"));
  return true;
}

void setup()
{
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println(F("ESP32-C6 LoRa sensor sender"));

  dht.begin();

  if (!initLoRa()) {
    while (true) {
      Serial.println(F("Retrying LoRa init in 5 seconds"));
      delay(5000);
      if (initLoRa()) {
        break;
      }
    }
  }
}

void loop()
{
  if (millis() - lastSendMillis < sendIntervalMs) {
    return;
  }
  lastSendMillis = millis();

  const float airTemperature = dht.readTemperature();
  const float humidity = dht.readHumidity();
  const float tds = readTDS();
  const float pH = readPH();

  if (isnan(airTemperature) || isnan(humidity)) {
    Serial.println(F("[Sensors] Failed to read DHT sensor"));
    return;
  }

  const float waterTemperature = readWaterTemperatureC();

  String payload = "{";
  payload += "\"air_temp_c\":" + String(airTemperature, 2) + ",";
  payload += "\"humidity_pct\":" + String(humidity, 2) + ",";
  payload += "\"water_temp_c\":" + String(waterTemperature, 2) + ",";
  payload += "\"tds_ppm\":" + String(tds, 0) + ",";
  payload += "\"ph\":" + String(pH, 2);
  payload += "}";

  LoRa.beginPacket();
  LoRa.print(payload);
  LoRa.endPacket();

  Serial.print(F("[LoRa] Sent payload: "));
  Serial.println(payload);
}
