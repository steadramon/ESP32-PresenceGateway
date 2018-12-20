// Basic Config
#include "globals.h"
#include "macsniff.h"

// Local logging tag
static const char TAG[] = "macsniff";

bool mac_add(uint8_t *paddr, int8_t rssi, int8_t chan, bool sniff_type) {

  char addr[ETH_MAC_LEN]; // temporary buffer for printf
  bool known = false;
  snprintf(addr, sizeof(addr), "%02x:%02x:%02x:%02x:%02x:%02x", paddr[0], paddr[1], paddr[2], paddr[3], paddr[4], paddr[5]);
  
  for (int u = 0; u < macs_known_count; u++) {
    if (! memcmp(macs_known[u].addr, addr, ETH_MAC_LEN)) {
      macs_known[u].lastDiscoveredTime = millis();
      macs_known[u].rssi = rssi;
      macs_known[u].status = 1;
      macs_known[u].ttl = (sniff_type)? BLE_TTL:WIFI_TTL;

      known = true;
      break;
    }
  }

  if (! known ) {  // AP is NEW, copy MAC to array and return it
    struct macinfo mi;
    mi.lastDiscoveredTime = millis();
    mi.rssi = rssi;
    memcpy(mi.addr, addr, ETH_MAC_LEN);
    mi.type = sniff_type;
    mi.channel = chan;
    mi.status = 1;
    mi.lastSentTime = 0;
    mi.ttl = (sniff_type)? BLE_TTL:WIFI_TTL;

    memcpy(&macs_known[macs_known_count], &mi, sizeof(mi));
    macs_known_count++;
    
    if (sniff_type != MAC_SNIFF_BLE) {
      macs_wifi++; // increment Wifi MACs counter
    }
    else if (sniff_type == MAC_SNIFF_BLE) {
      macs_ble++; // increment BLE Macs counter
    }

    // Log scan result
    ESP_LOGI(TAG,
             "%s %s RSSI %ddBi -> MAC %s -> WiFi:%d  BLTH:%d -> "
             "%d Bytes left",
             known ? "known  " : "NEW ",
             sniff_type == MAC_SNIFF_WIFI ? "WiFi" : "BLTH", rssi, addr,
             macs_wifi, macs_ble, ESP.getFreeHeap());

  }

  // True if MAC WiFi/BLE was new
  return known;
}

int get_ble_count() {
  return macs_ble;
}

int get_wifi_count() {
  return macs_wifi;
}

bool cleanup() {
  bool changed = false;
  for (int u = 0; u < macs_known_count; u++) {
    if (macs_known[u].ttl < 0) {
      macs_known[u].status = 0;
      changed = true;
    }
  }
  return changed;
}

void purge() {
  for (int u = 0; u < macs_known_count; u++) {
    if (macs_known[u].status == 0) {
      ESP_LOGI(TAG, "Offline!!!! %s", macs_known[u].addr );

      if (macs_known[u].type == MAC_SNIFF_WIFI) {
        macs_wifi--; // increment Wifi MACs counter
      } else if (macs_known[u].type == MAC_SNIFF_BLE) {
        macs_ble--; // increment BLE Macs counter
      }
      for (int i = u; i < macs_known_count; i++) memcpy(&macs_known[i], &macs_known[i + 1], sizeof(macs_known[i]));
      macs_known_count--;
      break;
    }
  }
}
