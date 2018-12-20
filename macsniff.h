#ifndef _MACSNIFF_H
#define _MACSNIFF_H

bool mac_add(uint8_t *paddr, int8_t rssi, int8_t channel, bool sniff_type);
int get_ble_count();
int get_wifi_count();
bool cleanup();
bool send_devices();
void purge();

#endif
