#include "globals.h"

boolean reconnect_mqtt() {

  int failure_number = 0;
  // Loop until we're reconnected
  while (!client.connected()) {
      char integer_string[16];
      int integer =  millis()/1000;
      sprintf(integer_string, "%d", integer);
      char buf[32];
      snprintf(buf, sizeof buf, "%s%s", Gateway_Name, integer_string);
      if (client.connect(buf, mqtt_user, mqtt_pass, will_Topic, will_QoS, will_Retain, will_Message)) {
      failure_number = 0;
      // Once connected, publish an announcement...
      client.publish(will_Topic,Gateway_AnnouncementMsg,will_Retain);
      // publish version
      client.publish(version_Topic,PROGVERSION,will_Retain);
      //Subscribing to topic
      if (client.subscribe(subjectMQTTtoX)) {
        #ifdef ZgatewayRF
          client.subscribe(subjectMultiGTWRF);
        #endif
      }
      } else {
      failure_number ++; // we count the failure

      // Wait 5 seconds before retrying
      delay(5000);

      if (failure_number > maxMQTTretry){
        return false;
      }
    }
  }
  return client.connected();
}

