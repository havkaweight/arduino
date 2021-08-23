#include <HX711.h>


#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
BLECharacteristic *macCharacteristic = NULL;
BLEAdvertising *pAdvertising = NULL;


float calibration_factor = -446.35; // Pasha's scale
bool deviceConnected = false;
bool oldDeviceConnected = false;
float grams;

// Сайт для генерирования UUID:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "f5ff08da-50b0-4a3e-b5e8-83509e584475"
#define CHARACTERISTIC_UUID "25dbb242-e59b-452c-9a04-c37bdb92e00a"

#define MAC_SERVICE_UUID        "11e12bc1-1c77-418e-a013-44dbecf33616"
#define MAC_CHARACTERISTIC_UUID "c455ab3b-f6e0-4002-ae3e-f109d03b1cd9"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer *pServer) {
      deviceConnected = false;
    }
};


// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 33;
const int LOADCELL_SCK_PIN = 32;
 
HX711 scale;

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

  BLEService *macService = pServer->createService(MAC_SERVICE_UUID); // Создаем BLE-сервис Device Info
  
  macCharacteristic = macService->createCharacteristic(
      MAC_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ
  ); // Создаем BLE-характеристику

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  macCharacteristic->addDescriptor(new BLE2902()); // создаем BLE-дескриптор

  static uint8_t mac_addr[6];
  esp_efuse_mac_get_default(mac_addr);

  // Note that when declaring an array of type char, one more element than your initialization is required, to hold the required null character.
  // https://www.arduino.cc/reference/en/language/variables/data-types/array/
  char mac_address[13] = "000000000000";

  for (int i=0; i<6; i++)
  {
    if (i==5) {
      mac_addr[i] = mac_addr[i]+2;
    }
    
    String str_buffer = String(mac_addr[i], HEX);
    
    mac_address[i*2] = str_buffer[1];
    mac_address[i*2+1] = str_buffer[0];
  }
  
  Serial.println(mac_address);

  macCharacteristic->setValue(mac_address);

  macService->start();

  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(BLEUUID(MAC_SERVICE_UUID));
  
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  std::string scale_data = "scale";
  oAdvertisementData.setServiceData(BLEUUID(MAC_SERVICE_UUID), scale_data);
  
  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->start(); // запускаем оповещения (advertising)
  Serial.println("Waiting a client connection to notify...");  //  "Ждем подключения клиента, чтобы отправить ему уведомление..."
}
 
void loop() {

  if (scale.is_ready()) {
//    value = scale.read();
    
    grams = scale.get_units(), 10;
    if (grams < 0)
    {
      grams = 0.00;
    }
    
    if (deviceConnected) {  
      char txString[13];
      dtostrf(grams, 10, 1, txString);
      pCharacteristic->setValue(txString);
      pCharacteristic->notify();
      //pCharacteristic->indicate();
      Serial.print("HX711 reading: ");
      Serial.printf("*** Grams: %d ***\n", grams);
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
