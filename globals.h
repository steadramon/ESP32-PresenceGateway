#ifndef _GLOBALS_H
#define _GLOBALS_H

#define PROGVERSION "0.0.1" // use max 10 chars here!
#define PROGNAME "ESP32-PresenceGateway"

#define MAX_MACS 128

#define WIFI_RSSI_LIMIT -90
#define BLE_RSSI_LIMIT -90

#define BLE_TTL 10
#define WIFI_TTL 10

#define ETH_MAC_LEN 18

#define BROADCAST_OFFLINE true

// WiFi scan parameters
#define WIFI_CHANNEL_MIN                1       // start channel number where scan begings
#define WIFI_CHANNEL_MAX                13      // total channel number to scan
#define WIFI_MY_COUNTRY                 "EU"    // select locale for Wifi RF settings
#define WIFI_CHANNEL_SWITCH_INTERVAL    500      // ms to scan each channel

// BLE scan parameters
#define BLESCANTIME                     10       // [seconds] scan duration, 0 means infinite [default], see note below
#define BLESCANWINDOW                   80      // [milliseconds] scan window, see below, 3 .. 10240, default 80ms
#define BLESCANINTERVAL                 80      // [illiseconds] scan interval, see below, 3 .. 10240, default 80ms = 100% duty cycle

// MQTT base bits
#define Gateway_Name "PresenceGateway"
#define Base_Topic "/presence/"
#define version_Topic  Base_Topic Gateway_Name "/version"
#define will_Topic  Base_Topic Gateway_Name "/LWT"
#define status_Topic Base_Topic Gateway_Name "/Status"
#define will_QoS 0
#define will_Retain true
#define will_Message "Offline"
#define Gateway_AnnouncementMsg "Online"
#define maxMQTTretry 4 //maximum MQTT connection attempts before going to wifi setup

#define subjectMQTTtoX  Base_Topic Gateway_Name "/commands/#"
#define subjectWifitoMQTT  Base_Topic Gateway_Name "/"

#define subjectMultiGTWKey "toMQTT"

#define payloadHome "home"
#define payloadAway "away"

#include <set>
#include <array>

#define subjectSYStoMQTT  Base_Topic Gateway_Name "/SYStoMQTT"
#define TimeBetweenReadingSYS 120000 // time between system readings (like memory)

#include <ArduinoJson.h>
#include <PubSubClient.h>
#undef min
#undef max

extern uint16_t macs_wifi, macs_ble; // int values

struct macinfo {
  char addr[18];
  long lastDiscoveredTime;
  long lastSentTime;
  unsigned channel:8;
  signed ttl:8;
  signed rssi:8;
  bool type;
  bool status;
};

extern macinfo *macs_known;
extern int macs_known_count;
extern int macs_known_count_old;

extern uint8_t channel; // wifi channel rotation counter

#define MAC_SNIFF_WIFI 0
#define MAC_SNIFF_BLE 1

#endif

