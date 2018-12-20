// Basics
#include "globals.h"
#include "User_config.h"
#include <math.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "wifi.h"

WiFiClient eClient;

// client link to pubsub mqtt
PubSubClient client(eClient);

macinfo macs_known_[MAX_MACS], *macs_known = macs_known_;

int macs_known_count = 0;                              // Number of known APs
int macs_known_count_old = 0;                              // Number of known APs
unsigned long lastReconnectAttempt = 0;

#include "mqtt.h"
#include "macsniff.h"
#include "wifiscan.h"
#include "blescan.h"

uint16_t macs_wifi = 0, macs_ble = 0; // globals for counting

unsigned long timer_sys_measures = 0;

void setup() {

  // Setup logging
#ifdef VERBOSE
  Serial.begin(115200);
  esp_log_level_set("*", ESP_LOG_VERBOSE);
#else
  // mute logs completely by redirecting them to silence function
  esp_log_level_set("*", ESP_LOG_NONE);
  esp_log_set_vprintf(redirect_log);
#endif

  ESP_LOGI(TAG, "Starting %s v%s", PROGNAME, PROGVERSION);

  //esp_event_loop_init(NULL, NULL);
       
  long port;
  port = strtol(mqtt_port,NULL,10);
  ESP_LOGI(TAG, "Connecting to MQTT by IP adress");
  client.setServer(mqtt_server, port);
  
  //client.setCallback(mqtt_callback);
  lastReconnectAttempt = 0;
  setup_wifi();
  reconnect_mqtt();
} // setup()

void loop() {

  if (client.connected()) { // we're connected
    client.publish(will_Topic, "Scanning", will_Retain);
    client.loop();
    client.disconnect ();
    delay(500);
  }
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.enableSTA(false);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    delay(500);
  }
  wifi_sniffer_init();
  loop_WifiSniffer();
  start_BLEscan();
  WiFi.enableSTA(true);
  
  cleanup();
  
  setup_wifi();
  reconnect_mqtt();
  if ( send_devices() ) {
    purge();
  }

  stateMeasures();
  delay(1000);
}

double calculateDistance(signed rssi) {
  
  signed txPower = -59;
  
  if (rssi == 0) {
    return -1.0; 
  }

  double ratio = rssi*1.0/txPower;
  if (ratio < 1.0) {
    return pow(ratio,10);
  }
  else {
    return (0.89976)*pow(ratio,7.7095) + 0.111;    
  }
} 

void stateMeasures(){
    unsigned long now = millis();
    if (now > (timer_sys_measures + TimeBetweenReadingSYS)) {//retriving value of memory ram every TimeBetweenReadingSYS

      if (reconnect_mqtt()) {
        lastReconnectAttempt = 0;
      } else {
        return;
      }
      
      timer_sys_measures = millis();
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& SYSdata = jsonBuffer.createObject();
      unsigned long uptime = millis()/1000;
      SYSdata["uptime"] = uptime;
      uint32_t freeMem;
      freeMem = ESP.getFreeHeap();
      SYSdata["freeMem"] = freeMem;
      long rssi = WiFi.RSSI();
      SYSdata["rssi"] = rssi;
      String SSID = WiFi.SSID();
      SYSdata["SSID"] = SSID;
      SYSdata["BLECount"] = get_ble_count();
      SYSdata["WiFiCount"] = get_wifi_count();

      ESP_LOGI(TAG, "Uptime: %ds Memory free: %d RSSI: %04d SSID: %s", uptime, freeMem, rssi, SSID);
      char JSONmessageBuffer[128];
      SYSdata.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
      client.publish(subjectSYStoMQTT,JSONmessageBuffer);
    }
}

bool send_devices() {
  if (!client.connected()) return false;

  for (int u = 0; u < macs_known_count; u++) {
    macs_known[u].ttl--;
    if ((!BROADCAST_OFFLINE) && (macs_known[u].status)) {
      continue;
    }
    DynamicJsonBuffer jsonBuffer;

    jsonBuffer.clear();
    JsonObject& data = jsonBuffer.createObject();

    data["mac"] = macs_known[u].addr;
    data["distance"] = calculateDistance(macs_known[u].rssi);
    data["channel"] = macs_known[u].channel;
    data["rssi"] = macs_known[u].rssi;
    data["status"] = macs_known[u].status ? "ONLINE" : "OFFLINE";
    data["source"] = macs_known[u].type ? "BLE" : "WiFi";
    data["payload"] = macs_known[u].status ? payloadHome : payloadAway;

    String topic = String(subjectWifitoMQTT)+ macs_known[u].addr;
    char JSONmessageBuffer[256];
    data.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    ESP_LOGI(TAG, "LOG: %s", JSONmessageBuffer);

    int rssilimit = -90;
    if (macs_known[u].type == 0) {
      rssilimit = WIFI_RSSI_LIMIT;
    } else {
      rssilimit = BLE_RSSI_LIMIT;
    }
    if (macs_known[u].rssi >= rssilimit) {
      if ((millis() - macs_known[u].lastSentTime) > 10000) {
        ESP_LOGI(TAG, "LOG: %s", JSONmessageBuffer);

        if (! client.publish((char*) topic.c_str(), JSONmessageBuffer) ) {
          return false;
        }
        if (! client.loop() ) {
          return false;
        }
        delay(100);
        macs_known[u].lastSentTime = millis();

      }
    
    }
      
  }
  return true;
}

