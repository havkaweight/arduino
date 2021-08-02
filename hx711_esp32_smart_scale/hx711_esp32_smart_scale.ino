#include <HX711.h>


#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
BLEAdvertising* pAdvertising = NULL;


float calibration_factor = -423.45;
bool deviceConnected = false;
bool oldDeviceConnected = false;
float units;
float grams;

// Сайт для генерирования UUID:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "f5ff08da-50b0-4a3e-b5e8-83509e584475"
//#define CHARACTERISTIC_UUID "25dbb242-e59b-452c-9a04-c37bdb92e00a"
#define CHARACTERISTIC_UUID "c455ab3b-f6e0-4002-ae3e-f109d03b1cd9"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};


// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 33;
const int LOADCELL_SCK_PIN = 32;
 
HX711 scale;

void set_advertising() {
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  std::string ass_data = "lolkek";
  oAdvertisementData.setServiceData(BLEUUID(SERVICE_UUID), ass_data);
  pAdvertising->setAdvertisementData(oAdvertisementData);
}

void setup() {
  Serial.begin(115200);
  
  // HX711 
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.tare();  //Reset the scale to 0
  scale.set_scale(calibration_factor);

  // BLE device
  BLEDevice::init("MyESP32"); // создаем BLE-устройство:

  pServer = BLEDevice::createServer(); // Создаем BLE-сервер
  pServer->setCallbacks(new MyServerCallbacks()); // Callback на подключение

  BLEService *pService = pServer->createService(SERVICE_UUID); // Создаем BLE-сервис
  
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ   |
      BLECharacteristic::PROPERTY_WRITE  |
      BLECharacteristic::PROPERTY_NOTIFY |
      BLECharacteristic::PROPERTY_INDICATE
  ); // Создаем BLE-характеристику

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  pCharacteristic->addDescriptor(new BLE2902()); // создаем BLE-дескриптор

  pService->start(); // запускаем сервис
  
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(BLEUUID(SERVICE_UUID));
  set_advertising(); 
  pAdvertising->start(); // запускаем оповещения (advertising)
  Serial.println("Waiting a client connection to notify...");  //  "Ждем подключения клиента, чтобы отправить ему уведомление..."

}
 
void loop() {

  if (scale.is_ready()) {
//    value = scale.read();
    
    units = scale.get_units(), 10;
    if (units < 0)
    {
      units = 0.00;
    }
    grams = units * 0.035274;
    
    if (deviceConnected) {  
      char txString[13];
      dtostrf(grams, 10, 1, txString);
      pCharacteristic->setValue(txString);
      pCharacteristic->notify();
      //pCharacteristic->indicate();
      Serial.print("HX711 reading: ");
      Serial.printf("*** Grams: %d ***\n", units);
      Serial.printf("*** NOTIFY: %d ***\n", grams);
    } else {
      Serial.println("BLE device not found.");
    }
  } else {
    Serial.println("HX711 not found.");
  }

  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
      delay(500); // give the bluetooth stack the chance to get things ready
      pServer->startAdvertising(); // restart advertising
      Serial.println("start advertising");
      oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
      // do stuff here on connecting
      oldDeviceConnected = deviceConnected;
  }
  delay(1000);
}
