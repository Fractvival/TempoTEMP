
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>


OneWire oneWire(D4);
DallasTemperature dallas(&oneWire);
int deviceCount = 0;
const int slaveAddress = 10;

String sensor1 = "28385d56000000fa";
String sensor2 = "2837e25500000076";

String inTemp = "0";
String outTemp = "0";


// Funkce pro získání adresy jako textového řetězce nebo "0" pokud adresa není nalezena
String getAddressAsString(int index) 
{
  DeviceAddress deviceAddress;
  if (!dallas.getAddress(deviceAddress, index)) 
  {
    return "0"; // Pokud čidlo není nalezeno, vrátí "0"
  }
  String addressString = "";
  for (uint8_t i = 0; i < 8; i++) 
  {
    if (deviceAddress[i] < 16) addressString += "0"; // doplní nulu, pokud je hodnota menší než 16
    addressString += String(deviceAddress[i], HEX);
  }
  return addressString;
}


// Funkce pro převod textu adresy na DeviceAddress
bool getDeviceAddressFromString(const String& addressText, DeviceAddress& deviceAddress) 
{
  if (addressText.length() != 16) return false; // Ověří správnou délku
  for (uint8_t i = 0; i < 8; i++) 
  {
    String byteString = addressText.substring(i * 2, i * 2 + 2);
    deviceAddress[i] = (uint8_t) strtol(byteString.c_str(), NULL, 16); // převod hex na byte
  }
  return true;
}


// Funkce pro získání teploty podle textové adresy
float getTemperatureByAddress(String addressText) 
{
  DeviceAddress deviceAddress;
  if (!getDeviceAddressFromString(addressText, deviceAddress)) 
  {
    Serial.println("INFO: Address not valid");
    return DEVICE_DISCONNECTED_C; // Konstantní hodnota, když senzor není nalezen
  }
  return dallas.getTempC(deviceAddress); // vrátí teplotu ve stupních Celsia
}


// Funkce volaná při přijetí dat z masteru
void receiveEvent(int howMany) {
    while (Wire.available()) {
        char c = Wire.read(); // Čtení dat z bufferu
        Serial.print(c);
    }
    Serial.println();
}

// Funkce volaná při požadavku masteru na data
void requestEvent() 
{
  String toMaster = "0";
  toMaster = inTemp;
  toMaster += "/";
  toMaster += outTemp;
  Wire.write(toMaster.c_str());
  Serial.print("INFO: Sending data to master: ");
  Serial.println(toMaster);
}



void setup() 
{
  Wire.begin(slaveAddress);      // Zahájení I2C jako slave s adresou 8
  Wire.onReceive(receiveEvent);  // Volání při přijetí dat
  Wire.onRequest(requestEvent);  // Volání při požadavku na data  
  Serial.begin(9600);
  dallas.begin();
  Serial.println("INFO: Tempo starting...");
  Serial.println("INFO: Get info from dallas sensors");
  deviceCount = dallas.getDeviceCount();
  Serial.print("INFO: Sensors count: ");
  Serial.println(deviceCount);
  if (deviceCount <= 1)
  {
    Serial.print("INFO: Sensors not available, restarting board..");
    delay(5000);
    ESP.restart();
  }
  else
  {
    for (int i = 0; i < deviceCount; i++) {
      String addressText = getAddressAsString(i);
      Serial.print("Sensor #");
      Serial.print(i + 1);
      Serial.print(" - address: ");
      Serial.println(addressText);
    }
  }
}

void loop() 
{
  dallas.requestTemperatures();
  float temperatureC = getTemperatureByAddress(sensor1);
  if (temperatureC != DEVICE_DISCONNECTED_C) 
  {
    Serial.print("IN:");
    Serial.println(temperatureC, 1); // vypíše teplotu zaokrouhlenou na 1 desetinné místo
    inTemp = String(temperatureC, 1);
  } 
  else 
  {
    Serial.println("INFO: IN not found or disconnect");
  }
  delay(500);
  //dallas.requestTemperatures();
  temperatureC = 0.0f;
  temperatureC = getTemperatureByAddress(sensor2);
  if (temperatureC != DEVICE_DISCONNECTED_C) 
  {
    Serial.print("OUT:");
    Serial.println(temperatureC, 1); // vypíše teplotu zaokrouhlenou na 1 desetinné místo
    outTemp = String(temperatureC,1);
  } 
  else 
  {
    Serial.println("INFO: OUT not found or disconnect");
  }
  delay(500);
}
