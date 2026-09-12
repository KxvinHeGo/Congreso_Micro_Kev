#include <Wire.h>
#include <ArduinoBLE.h>
#include "Adafruit_BME680.h"
#include "SparkFun_AS7265X.h"

// UUIDs para el servicio y la característica
#define SERVICE_UUID        "19B10000-E8F2-537E-4F6C-D104768A1214"
#define CHARACTERISTIC_UUID "19B10001-E8F2-537E-4F6C-D104768A1214"
#define NOMBRE_DISPOSITIVO  "DV-2017"

AS7265X spectralSensor;
Adafruit_BME680 bme; 

BLEService sensorService(SERVICE_UUID);
BLEStringCharacteristic sensorCharacteristic(
  CHARACTERISTIC_UUID,
  BLERead | BLENotify | BLEWrite,
  512
);

// Contador de secuencia
uint32_t contadorSecuencia = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin();

  // Detección AS7265X
  if (spectralSensor.begin()) {
    Serial.println("[OK] AS7265X detectado.");
  } else {
    Serial.println("[ERROR] AS7265X no detectado.");
    while (1) delay(1000);
  }

  // Detección BME688
  if (bme.begin(0x77) || bme.begin(0x76)) {
    Serial.println("[OK] BME688 detectado.");
    
    // Configuración BME688
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150);
  } else {
    Serial.println("[ERROR] BME688 no detectado.");
    while (1) delay(1000);
  }

  // Configuración BLE
  if (!BLE.begin()) {
    Serial.println("[ERROR] Fallo al iniciar BLE.");
    while (1);
  }

  BLE.setLocalName(NOMBRE_DISPOSITIVO);
  BLE.setDeviceName(NOMBRE_DISPOSITIVO);
  BLE.setAdvertisedService(sensorService);
  sensorService.addCharacteristic(sensorCharacteristic);
  BLE.addService(sensorService);
  sensorCharacteristic.writeValue("{}");
  BLE.advertise();

  Serial.println("Esperando conexión central...");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Cliente conectado: ");
    Serial.println(central.address());

    // REINICIO DEL CONTADOR
    contadorSecuencia = 0;

    while (central.connected()) {
      if (sensorCharacteristic.written()) {
        String comandoRecibido = sensorCharacteristic.value();
        comandoRecibido.trim();

        if (comandoRecibido.startsWith("MEDIR")) {
          Serial.print("Comando recibido: ");
          Serial.println(comandoRecibido);
          ejecutarMedicionCompleta(comandoRecibido);
        }
      }
      BLE.poll();
      delay(10);
    }
    Serial.println("Cliente desconectado.");
    BLE.advertise();
  }
}

void ejecutarMedicionCompleta(String comando) {
  // Parsing del comando: MEDIR;<MODO>;<GANANCIA>;<INTEGRACION>
  int p1 = comando.indexOf(';');
  int p2 = comando.indexOf(';', p1 + 1);
  int p3 = comando.indexOf(';', p2 + 1);

  if (p1 == -1 || p2 == -1 || p3 == -1) {
    Serial.println("ERROR: Formato de comando inválido.");
    return;
  }

  String modo = comando.substring(p1 + 1, p2);
  int gananciaNum = comando.substring(p2 + 1, p3).toInt();
  int tiempoIntegracion = comando.substring(p3 + 1).toInt();

  // Configurar Ganancia AS7265X
  switch (gananciaNum) {
    case 1:  spectralSensor.setGain(AS7265X_GAIN_1X); break;
    case 3:  spectralSensor.setGain(AS7265X_GAIN_37X); break;
    case 16: spectralSensor.setGain(AS7265X_GAIN_16X); break;
    case 64: spectralSensor.setGain(AS7265X_GAIN_64X); break;
    default: spectralSensor.setGain(AS7265X_GAIN_16X); break;
  }
  spectralSensor.setIntegrationCycles(max(1, min(255, tiempoIntegracion)));

  // Lectura Ambiental
  if (!bme.performReading()) {
    Serial.println("ERROR: Fallo al leer BME688.");
  }

  // Lectura Espectral
  if (modo == "WRAW" || modo == "WCALIBRATED") {
    spectralSensor.takeMeasurementsWithBulb(); 
  } else {
    spectralSensor.takeMeasurements();
  }

  uint8_t tempIR  = spectralSensor.getTemperature(0);
  uint8_t tempVIS = spectralSensor.getTemperature(1);
  uint8_t tempUV  = spectralSensor.getTemperature(2);

  contadorSecuencia++;

  String json = "{\"id\":\"" NOMBRE_DISPOSITIVO "\",\"sequence\":" + String(contadorSecuencia) + ",\"modo\":\"" + modo + "\",";

  if (modo == "RAW" || modo == "WRAW") {
    json += "\"A\":" + String(spectralSensor.getA()) + ",\"B\":" + String(spectralSensor.getB()) + ",\"C\":" + String(spectralSensor.getC()) + ",";
    json += "\"D\":" + String(spectralSensor.getD()) + ",\"E\":" + String(spectralSensor.getE()) + ",\"F\":" + String(spectralSensor.getF()) + ",";
    json += "\"G\":" + String(spectralSensor.getG()) + ",\"H\":" + String(spectralSensor.getH()) + ",\"I\":" + String(spectralSensor.getI()) + ",";
    json += "\"J\":" + String(spectralSensor.getJ()) + ",\"K\":" + String(spectralSensor.getK()) + ",\"L\":" + String(spectralSensor.getL()) + ",";
    json += "\"R\":" + String(spectralSensor.getR()) + ",\"S\":" + String(spectralSensor.getS()) + ",\"T\":" + String(spectralSensor.getT()) + ",";
    json += "\"U\":" + String(spectralSensor.getU()) + ",\"V\":" + String(spectralSensor.getV()) + ",\"W\":" + String(spectralSensor.getW()) + ",";
  } else {
    json += "\"A\":" + String(spectralSensor.getCalibratedA(), 2) + ",\"B\":" + String(spectralSensor.getCalibratedB(), 2) + ",";
    json += "\"C\":" + String(spectralSensor.getCalibratedC(), 2) + ",\"D\":" + String(spectralSensor.getCalibratedD(), 2) + ",";
    json += "\"E\":" + String(spectralSensor.getCalibratedE(), 2) + ",\"F\":" + String(spectralSensor.getCalibratedF(), 2) + ",";
    json += "\"G\":" + String(spectralSensor.getCalibratedG(), 2) + ",\"H\":" + String(spectralSensor.getCalibratedH(), 2) + ",";
    json += "\"I\":" + String(spectralSensor.getCalibratedI(), 2) + ",\"J\":" + String(spectralSensor.getCalibratedJ(), 2) + ",";
    json += "\"K\":" + String(spectralSensor.getCalibratedK(), 2) + ",\"L\":" + String(spectralSensor.getCalibratedL(), 2) + ",";
    json += "\"R\":" + String(spectralSensor.getCalibratedR(), 2) + ",\"S\":" + String(spectralSensor.getCalibratedS(), 2) + ",";
    json += "\"T\":" + String(spectralSensor.getCalibratedT(), 2) + ",\"U\":" + String(spectralSensor.getCalibratedU(), 2) + ",";
    json += "\"V\":" + String(spectralSensor.getCalibratedV(), 2) + ",\"W\":" + String(spectralSensor.getCalibratedW(), 2) + ",";
  }

  json += "\"tUV\":" + String(tempUV) + ",\"tVIS\":" + String(tempVIS) + ",\"tIR\":" + String(tempIR) + ",";
  json += "\"temperature\":" + String(bme.temperature, 2) + ",";
  json += "\"humidity\":" + String(bme.humidity, 2) + ",";
  json += "\"pressure\":" + String(bme.pressure / 100.0, 2) + ","; 
  json += "\"gas\":" + String(bme.gas_resistance / 1000.0, 2); 
  json += "}";

  Serial.println("--------------------------------");
  Serial.print("Enviando JSON (");
  Serial.print(json.length());
  Serial.println(" bytes):");
  Serial.println(json);

  enviarJsonPorFragmentos(json);
}

void enviarJsonPorFragmentos(String mensaje) {
  int longitud = mensaje.length();
  int tamanoFragmento = 60;
  
  for (int i = 0; i < longitud; i += tamanoFragmento) {
    String fragmento = mensaje.substring(i, min(i + tamanoFragmento, longitud));
    sensorCharacteristic.writeValue(fragmento);
    delay(35); 
  }
}