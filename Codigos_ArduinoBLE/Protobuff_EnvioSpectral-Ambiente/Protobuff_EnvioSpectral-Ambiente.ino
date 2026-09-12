#include <Wire.h>
#include <ArduinoBLE.h>
#include "Adafruit_BME680.h"
#include "SparkFun_AS7265X.h"

// ARCHIVOS DE PROTOBUF 
#include "pb_encode.h"
#include "pb_common.h"
#include "sensor_data.pb.h"

// CONFIGURACIÓN
#define NOMBRE_DISPOSITIVO  "DV-2017"
#define SERVICE_UUID        "19B10000-E8F2-537E-4F6C-D104768A1214"
#define CHARACTERISTIC_UUID "19B10001-E8F2-537E-4F6C-D104768A1214"

AS7265X spectralSensor;
Adafruit_BME680 bme; 

BLEService sensorService(SERVICE_UUID);
BLECharacteristic sensorCharacteristic(CHARACTERISTIC_UUID, BLERead | BLENotify | BLEWrite, 256);

uint32_t contadorSecuencia = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin();

  if (spectralSensor.begin()) Serial.println("[OK] AS7265X");
  if (bme.begin(0x77) || bme.begin(0x76)) {
    Serial.println("[OK] BME688");
    bme.setGasHeater(320, 150);
  }

  if (BLE.begin()) {
    BLE.setLocalName(NOMBRE_DISPOSITIVO);
    BLE.setAdvertisedService(sensorService);
    sensorService.addCharacteristic(sensorCharacteristic);
    BLE.addService(sensorService);
    BLE.advertise();
    Serial.println("BLE Activo con Sync 0xAA");
  }
}

void loop() {
  BLEDevice central = BLE.central();
  if (central) {
    Serial.print("Conectado: "); Serial.println(central.address());
    contadorSecuencia = 0;
    while (central.connected()) {
      if (sensorCharacteristic.written()) {
        int len = sensorCharacteristic.valueLength();
        const uint8_t* val = sensorCharacteristic.value();
        char cmd[len + 1]; memcpy(cmd, val, len); cmd[len] = '\0';
        String comando = String(cmd); comando.trim();
        if (comando.startsWith("MEDIR")) {
          
          Serial.print("Comando recibido: "); Serial.println(comando);
          ejecutarMedicionProtobuf(comando);
        }
      }
      BLE.poll();
    }
    Serial.println("Desconectado");
  }
}

void ejecutarMedicionProtobuf(String comando) {
  int p1 = comando.indexOf(';');
  int p2 = comando.indexOf(';', p1 + 1);
  int p3 = comando.indexOf(';', p2 + 1);
  if (p1 == -1 || p2 == -1 || p3 == -1) return;

  String modo = comando.substring(p1 + 1, p2);
  int ganancia = comando.substring(p2 + 1, p3).toInt();
  int tiempo = comando.substring(p3 + 1).toInt();

  switch (ganancia) {
    case 1: spectralSensor.setGain(AS7265X_GAIN_1X); break;
    case 16: spectralSensor.setGain(AS7265X_GAIN_16X); break;
    case 64: spectralSensor.setGain(AS7265X_GAIN_64X); break;
    default: spectralSensor.setGain(AS7265X_GAIN_16X); break;
  }
  spectralSensor.setIntegrationCycles(max(1, min(255, tiempo)));

  if (!bme.performReading()) Serial.println("Err BME");
  if (modo.startsWith("W")) spectralSensor.takeMeasurementsWithBulb(); 
  else spectralSensor.takeMeasurements();

  contadorSecuencia++;
  SpectralMeasurement msg = SpectralMeasurement_init_default;
  strncpy(msg.id, NOMBRE_DISPOSITIVO, 15);
  msg.sequence = contadorSecuencia;
  strncpy(msg.modo, modo.c_str(), 15);

  bool isCal = modo.endsWith("CALIBRATED");
  if (isCal) {
    msg.ch_A = spectralSensor.getCalibratedA(); msg.ch_B = spectralSensor.getCalibratedB();
    msg.ch_C = spectralSensor.getCalibratedC(); msg.ch_D = spectralSensor.getCalibratedD();
    msg.ch_E = spectralSensor.getCalibratedE(); msg.ch_F = spectralSensor.getCalibratedF();
    msg.ch_G = spectralSensor.getCalibratedG(); msg.ch_H = spectralSensor.getCalibratedH();
    msg.ch_I = spectralSensor.getCalibratedI(); msg.ch_J = spectralSensor.getCalibratedJ();
    msg.ch_K = spectralSensor.getCalibratedK(); msg.ch_L = spectralSensor.getCalibratedL();
    msg.ch_R = spectralSensor.getCalibratedR(); msg.ch_S = spectralSensor.getCalibratedS();
    msg.ch_T = spectralSensor.getCalibratedT(); msg.ch_U = spectralSensor.getCalibratedU();
    msg.ch_V = spectralSensor.getCalibratedV(); msg.ch_W = spectralSensor.getCalibratedW();
  } else {
    msg.ch_A = spectralSensor.getA(); msg.ch_B = spectralSensor.getB();
    msg.ch_C = spectralSensor.getC(); msg.ch_D = spectralSensor.getD();
    msg.ch_E = spectralSensor.getE(); msg.ch_F = spectralSensor.getF();
    msg.ch_G = spectralSensor.getG(); msg.ch_H = spectralSensor.getH();
    msg.ch_I = spectralSensor.getI(); msg.ch_J = spectralSensor.getJ();
    msg.ch_K = spectralSensor.getK(); msg.ch_L = spectralSensor.getL();
    msg.ch_R = spectralSensor.getR(); msg.ch_S = spectralSensor.getS();
    msg.ch_T = spectralSensor.getT(); msg.ch_U = spectralSensor.getU();
    msg.ch_V = spectralSensor.getV(); msg.ch_W = spectralSensor.getW();
  }

  msg.temp_uv = spectralSensor.getTemperature(2);
  msg.temp_vis = spectralSensor.getTemperature(1);
  msg.temp_ir = spectralSensor.getTemperature(0);
  msg.ambient_temp = bme.temperature;
  msg.ambient_hum = bme.humidity;
  msg.ambient_pres = bme.pressure / 100.0;
  msg.ambient_gas = bme.gas_resistance / 1000.0;

  // IMPRIMIR COMO SE VERIA EN JSON PARA UNA MEJOR LECTURA 
  Serial.println("--------------------------------");
  Serial.println("Enviando Protobuf (Vista JSON):");
  Serial.print("{\"id\":\""); Serial.print(msg.id);
  Serial.print("\",\"sequence\":"); Serial.print(msg.sequence);
  Serial.print(",\"modo\":\""); Serial.print(modo);
  Serial.print("\",\"A\":"); Serial.print(msg.ch_A, 2); Serial.print(",\"B\":"); Serial.print(msg.ch_B, 2);
  Serial.print(",\"C\":"); Serial.print(msg.ch_C, 2); Serial.print(",\"D\":"); Serial.print(msg.ch_D, 2);
  Serial.print(",\"E\":"); Serial.print(msg.ch_E, 2); Serial.print(",\"F\":"); Serial.print(msg.ch_F, 2);
  Serial.print(",\"G\":"); Serial.print(msg.ch_G, 2); Serial.print(",\"H\":"); Serial.print(msg.ch_H, 2);
  Serial.print(",\"I\":"); Serial.print(msg.ch_I, 2); Serial.print(",\"J\":"); Serial.print(msg.ch_J, 2);
  Serial.print(",\"K\":"); Serial.print(msg.ch_K, 2); Serial.print(",\"L\":"); Serial.print(msg.ch_L, 2);
  Serial.print(",\"R\":"); Serial.print(msg.ch_R, 2); Serial.print(",\"S\":"); Serial.print(msg.ch_S, 2);
  Serial.print(",\"T\":"); Serial.print(msg.ch_T, 2); Serial.print(",\"U\":"); Serial.print(msg.ch_U, 2);
  Serial.print(",\"V\":"); Serial.print(msg.ch_V, 2); Serial.print(",\"W\":"); Serial.print(msg.ch_W, 2);
  Serial.print(",\"tUV\":"); Serial.print(msg.temp_uv); Serial.print(",\"tVIS\":"); Serial.print(msg.temp_vis);
  Serial.print(",\"tIR\":"); Serial.print(msg.temp_ir); Serial.print(",\"temperature\":"); Serial.print(msg.ambient_temp, 2);
  Serial.print(",\"humidity\":"); Serial.print(msg.ambient_hum, 2); Serial.print(",\"pressure\":"); Serial.print(msg.ambient_pres, 2);
  Serial.print(",\"gas\":"); Serial.print(msg.ambient_gas, 2); Serial.println("}");

  // CODIFICAR Y ENVIAR
  uint8_t p_buf[240];
  pb_ostream_t p_str = pb_ostream_from_buffer(p_buf, sizeof(p_buf));
  if (!pb_encode(&p_str, SpectralMeasurement_fields, &msg)) return;
  size_t p_sz = p_str.bytes_written;

  uint8_t out[250];
  out[0] = 0xAA; out[1] = (uint8_t)p_sz;
  memcpy(&out[2], p_buf, p_sz);
  sensorCharacteristic.writeValue(out, p_sz + 2);
}