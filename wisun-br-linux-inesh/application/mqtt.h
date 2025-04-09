#ifndef MQTT_H
#define MQTT_H

#include <stdint.h>

// Fixed MQTT configuration
#define BROKER_ADDRESS "mqtt.eclipseprojects.io" // Replace with your broker address
#define BROKER_PORT 1883           // Replace with your broker port

#define DCU_SUB_TOPIC "RB_DCU/Ineshdata_cloud/"
#define DCU_PUB_TOPIC "RB_DCU/Ineshdata_dcu/"
#define DCU_PUB_PUSH_TOPIC "RB_DCU/Ineshdata_dcu/rf_push"

#define DEBUG 0
// Function to initialize the MQTT client
int mqtt_init();

// Function to publish a message to an MQTT topic
int mqtt_publish(const uint8_t *message, int message_len, const char *topic);

// Function to disconnect and clean up the MQTT client
void mqtt_cleanup();

#endif // MQTT_H