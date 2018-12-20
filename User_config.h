#define VERBOSE 1 // comment out to silence the device, for mute use build option

#define wifi_ssid "WIFISSID"
#define wifi_password "password"
  
//set a constant IP address so your DHCP server isn't hammered

const byte ip[] = { 192, 168, 1, 81 }; //ip adress
const byte gateway[] = { 192, 168, 1, 1 }; //ip adress
const byte Dns[] = { 192, 168, 1, 1 }; //ip adress
const byte subnet[] = { 255, 255, 255, 0 }; //ip adress

char mqtt_user[20] = "user"; // not compulsory only if your broker needs authentication
char mqtt_pass[20] = "password"; // not compulsory only if your broker needs authentication
char mqtt_server[40] = "mqtt.local";
char mqtt_port[6] = "1883";


