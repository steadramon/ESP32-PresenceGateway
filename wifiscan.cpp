// Basic Config
#include "globals.h"
#include "wifiscan.h"
#include "macsniff.h"
#include <string>
#include <sstream>
#include <string.h>
#include <stdio.h>
static const char TAG[] = "wifiscan";

static wifi_country_t wifi_country = {WIFI_MY_COUNTRY, WIFI_CHANNEL_MIN,
                                      WIFI_CHANNEL_MAX, 0,
                                      WIFI_COUNTRY_POLICY_MANUAL};

String formatMac1(uint8_t mac[ETH_MAC_LEN]) {
  String hi = "";
  for (int i = 0; i < ETH_MAC_LEN; i++) {
    if (mac[i] < 16) hi = hi + "0" + String(mac[i], HEX);
    else hi = hi + String(mac[i], HEX);
    if (i < 5) hi = hi + ":";
  }
  return hi;
}

int nothing_new = 0;

IRAM_ATTR void wifi_sniffer_packet_handler(void *buff,
                                           wifi_promiscuous_pkt_type_t type) {

  const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
  const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
  const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;
  
  uint8_t macadd[6];

  for (int j=0; j<6; j++)
    macadd[j] = hdr->addr2[j];

/*  

 uint8_t ssid_length;
  uint8_t _subtype;
  char ssid[33];

  uint16_t probe_payload_len = ppkt->rx_ctrl.sig_len; 
  if (type == WIFI_PKT_MGMT) {
  _subtype = ppkt->payload[0];
  //_subtype = hdr->frame_ctrl[0];
  if(_subtype == 0x40){
    if(probe_payload_len > 0){
      ssid_length = ipkt->payload[1];
      for (int i=0; i<ssid_length; i++)
        ssid[i] = (char)ipkt->payload[i+2];
    }
  }
  }

  snprintf(ssid, 1, "");
*/

  mac_add((uint8_t *)macadd, ppkt->rx_ctrl.rssi, ppkt->rx_ctrl.channel, MAC_SNIFF_WIFI);

}

void wifi_sniffer_init(void) {
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  cfg.nvs_enable = 0; // we don't need any wifi settings from NVRAM
  wifi_promiscuous_filter_t filter = {
      .filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT}; // we need only MGMT frames
  esp_wifi_init(&cfg);             // configure Wifi with cfg
  esp_wifi_set_country(&wifi_country); // set locales for RF and channels
  esp_wifi_set_storage(WIFI_STORAGE_RAM); // we don't need NVRAM
  esp_wifi_set_mode(WIFI_MODE_STA);
  //esp_wifi_set_promiscuous_filter(&filter); // set MAC frame filter
  esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler);
  esp_wifi_set_promiscuous(true); // now switch on monitor mode
}

void loop_WifiSniffer(void){

  int channel = WIFI_CHANNEL_MIN;
  boolean sendMQTT = false;

  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(true);
  while (true) {
    nothing_new++;                          // Array is not finite, check bounds and adjust if required
    if (nothing_new > WIFI_CHANNEL_SWITCH_INTERVAL) {                // monitor channel for 200 ms
      nothing_new = 0;
      channel++;
      if (channel == WIFI_CHANNEL_MAX+1) break;             
      
      esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    }
    delay(1);  // critical processing timeslice for NONOS SDK! No delay(0) yield()

  }
  esp_wifi_set_promiscuous(false);
  delay(100);
}
