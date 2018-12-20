/* code snippets taken from
https://github.com/nkolban/esp32-snippets/tree/master/BLE/scanner
*/

// Basic Config
#include "globals.h"
#include "blescan.h"
#include "macsniff.h"

// Bluetooth specific includes
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// local Tag for logging
static const char TAG[] = "blescan";

boolean firstTime = true;

IRAM_ATTR class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
      /*
      const char* bname;
      if (advertisedDevice.haveName()) {
        bname = advertisedDevice.getName().c_str();
      }
      */
      mac_add((uint8_t *)advertisedDevice.getAddress().getNative(), advertisedDevice.getRSSI(), 0, MAC_SNIFF_BLE);
    }

};

void start_BLEscan() {
    BLEDevice::init("");

  // put your main code here, to run repeatedly:
  BLEScan *pBLEScan = BLEDevice::getScan(); //create new scan
  MyAdvertisedDeviceCallbacks myCallbacks;
  pBLEScan->setAdvertisedDeviceCallbacks(&myCallbacks);
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(0x50);
  pBLEScan->setWindow(0x30);
  int scantime = BLESCANTIME;
  if (firstTime) {
    scantime = BLESCANTIME * 3;
    firstTime = false;
  }
  ESP_LOGI(TAG,"Start BLE scan for %d seconds...", scantime);
  BLEScanResults foundDevices = pBLEScan->start(scantime);
  delay(200);
}
