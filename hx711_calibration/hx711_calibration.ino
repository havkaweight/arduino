/*
 Setup your scale and start the sketch WITHOUT a weight on the scale
 Once readings are displayed place the weight on the scale
 Press +/- or a/z to adjust the calibration_factor until the output readings match the known weight
 Arduino pin 6 -> HX711 CLK
 Arduino pin 5 -> HX711 DOUT
 Arduino pin 5V -> HX711 VCC
 Arduino pin GND -> HX711 GND 
*/

#include "HX711.h"

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 33;
const int LOADCELL_SCK_PIN = 32;

HX711 scale;

//float calibration_factor = 15.65; // this calibration factor is adjusted according to my load cell
float calibration_factor = -446.35; // Pasha's scale
float units;
float ounces;

void setup() {
  Serial.begin(115200);
  Serial.println("HX711 calibration sketch");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place known weight on scale");
  Serial.println("Press + or a to increase calibration factor");
  Serial.println("Press - or z to decrease calibration factor");

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale();
  scale.tare();  //Reset the scale to 0

  long zero_factor = scale.read_average(); //Get a baseline reading
  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);

  static uint8_t mac_addr[6];
  esp_efuse_mac_get_default(mac_addr);

  char mac_address[13];

  for (int i=0; i<6; i++)
  {
    if (i==5) {
      mac_addr[i] = mac_addr[i]+2;
    }
    
    String str_buffer = String(mac_addr[i], HEX);
    Serial.println(mac_addr[i], HEX);
    Serial.println(":");
    
    mac_address[i*2] = str_buffer[1];
    mac_address[i*2+1] = str_buffer[0];
  }
  Serial.println(mac_address);
  
}

void loop() {
//
//  scale.set_scale(calibration_factor); //Adjust to this calibration factor
//
//  Serial.print("Reading: ");
//  units = scale.get_units(), 10; 
//  Serial.print(units);
//  Serial.print(" grams ");
//  if (units < 0)
//  {
//    units = 0.00;
//  }
//  ounces = units;// * 0.035274;
//  Serial.print(ounces);
//  Serial.print(" ounce"); 
//  Serial.print(" calibration_factor: ");
//  Serial.print(calibration_factor);
//  Serial.println();
//
//  if(Serial.available())
//  {
//    char temp = Serial.read();
//    if(temp == '+' || temp == 'a')
//      calibration_factor += 0.1;
//    else if(temp == '-' || temp == 'z')
//      calibration_factor -= 0.1;
//  }
}
