#include "globals.h"
#include "wifiscan.h"

boolean setup_wifi() {
  delay(10);

  int failureAttempt = 0; //DIRTY FIX ESP32 waiting for https://github.com/espressif/arduino-esp32/issues/653
  // We start by connecting to a WiFi network
  ESP_LOGI(TAG, "Connecting to %s %s", wifi_ssid, wifi_password);
  IPAddress ip_adress(ip);
  IPAddress gateway_adress(gateway);
  IPAddress subnet_adress(subnet);
  IPAddress dns_adress(Dns);
  WiFi.setAutoReconnect(false);

  WiFi.disconnect(true); // FIX - https://github.com/espressif/arduino-esp32/issues/653
  WiFi.config(ip_adress,gateway_adress,subnet_adress,dns_adress); //Uncomment this line if you want to use advanced network config
  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    failureAttempt++; //DIRTY FIX ESP32
    if (failureAttempt > 5) return false; //DIRTY FIX ESP32
  }
  delay(1500);
  ESP_LOGI(F("WiFi ok with manual config credentials"));
  return true;
}
