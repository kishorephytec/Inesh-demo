#include "mqtt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h> // Ensure you have the Mosquitto library installed
#include <cjson/cJSON.h>
#include <unistd.h>
#include <pthread.h>

#include "json.h"
#include "tun_app.h"

static struct mosquitto *mosq = NULL; // Global MQTT client instance

#include <pthread.h>
#include <unistd.h> // For sleep

volatile int interval = 300;  // Default sleep time

// Function to parse JSON and update sleep interval
void update_rtlist_interval(const char *json_payload) {
    cJSON *json = cJSON_Parse(json_payload);
    if (json == NULL) {
        printf("[ERROR] Failed to parse JSON from MQTT message.\n");
        return;
    }
 
    cJSON *rt_config = cJSON_GetObjectItem(json, "rt_configuration");
    if (rt_config) {
        cJSON *time_period = cJSON_GetObjectItem(rt_config, "rt_interval");
        if (cJSON_IsString(time_period) && time_period->valuestring) {
            interval = atoi(time_period->valuestring);  // Directly assign value without any range check
            printf("[INFO] Updated sleep interval to %d seconds\n", interval);
        }
    }
 
    cJSON_Delete(json);
}

void *generalized_publisher(void *arg) {

    while (1) {
        // Generate the JSON response
        char *data_type="rt list";
        int network=1;
        printf("%s\n", data_type);
        cJSON *response = create_json_response(data_type, network);
        if (response) {
            char *response_str = cJSON_PrintUnformatted(response);
            if (response_str) {
                char topic[256];
                snprintf(topic, sizeof(topic), "%s%s", DCU_PUB_TOPIC, "rf_command");
                mqtt_publish((const uint8_t *)response_str, strlen(response_str), topic);
                printf("Published %s: %s\n", data_type, response_str);
                free(response_str);
            }
            cJSON_Delete(response);
        } else {
            fprintf(stderr, "Failed to create JSON response for %s.\n", data_type);
        }

        // Wait for the specified interval
        sleep(interval);
    }

    return NULL;
}

// Function to handle "rf_command" JSON format
void handle_rf_command(cJSON *rf_command) {
    cJSON *network = cJSON_GetObjectItem(rf_command, "network");
    cJSON *data = cJSON_GetObjectItem(rf_command, "data");

    if (network && data) {
#ifdef DEBUG
        printf("RF Command:\n");

	const char *network_str = cJSON_GetStringValue(network);
    	int network_int = network_str ? atoi(network_str) : 0;

        printf("  Network: %d\n", network_int);
        printf("  Data: %s\n", data->valuestring);
        // Add further processing logic for "rf_command" here
#endif
        // Create the JSON response
        cJSON *response = create_json_response(data->valuestring,network_int);
        if (response) {
            // Publish the response
            char *response_str = cJSON_PrintUnformatted(response);
            char topic[50]; // Adjust size as needed

            snprintf(topic, sizeof(topic), "%s%s", DCU_PUB_TOPIC, "rf_command");
            mqtt_publish((const uint8_t *)response_str, strlen(response_str), topic);
            printf("Published %s: %s\n", data->valuestring, response_str);

            free(response_str);
            cJSON_Delete(response);
        } else {
            fprintf(stderr, "Failed to create JSON response for %s\n", data->valuestring);
        }  
    } else {
        fprintf(stderr, "Invalid rf_command format.\n");
    }
}

// Function to handle "rf_dlms" JSON format
void handle_rf_dlms(cJSON *rf_dlms) {
    cJSON *macid = cJSON_GetObjectItem(rf_dlms, "Macid");
    cJSON *data = cJSON_GetObjectItem(rf_dlms, "data");

    if (macid && data) {
        uint8_t byte_array[1024]; // Adjust size as needed
        int byte_len = hex_string_to_bytes(data->valuestring, byte_array, sizeof(byte_array));
        if (byte_len < 0) {
            fprintf(stderr, "Failed to convert hex string to bytes.\n");
            return;
        }
	tun_app_write(macid->valuestring, byte_array, byte_len);

#ifdef DEBUG
        printf("RF DLMS:\n");
        printf("  Mac ID: %s\n", macid->valuestring);
        printf("  Data: %s\n", data->valuestring);
        printf("  Data (binary): ");
        for (int i = 0; i < byte_len; i++) {
            printf("%02x ", byte_array[i]);
        }
        printf("\n");
#endif
        // Add further processing logic for "rf_dlms" here
        
    } else {
        fprintf(stderr, "Invalid rf_dlms format.\n");
    }
}

// Callback function to handle incoming messages
static void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) {
    if (message->payloadlen) {
        printf("Received message on topic %s: %s\n", message->topic, (char *)message->payload);
 
        // Parse the received message as JSON
        cJSON *json_message = cJSON_Parse((char *)message->payload);
        if (json_message == NULL) {
            fprintf(stderr, "Failed to parse JSON message.\n");
            return;
        }
 
        // Check for "rf_dlms" key
        cJSON *rf_dlms = cJSON_GetObjectItem(json_message, "rf_dlms");
        if (rf_dlms) {
            handle_rf_dlms(rf_dlms);  // Pass the inner "rf_dlms" object to the handler
        }
 
        // Check for "rf_command" key
        cJSON *rf_command = cJSON_GetObjectItem(json_message, "rf_command");
        if (rf_command) {
            handle_rf_command(rf_command);  // Pass the inner "rf_command" object to the handler
        }
 
        // If neither key is found, print an error
        if (strncmp(message->topic, "RB_DCU/RTList_Interval/", 23) == 0) {
            update_rtlist_interval((char *)message->payload);  // Call function to update RT interval
        }
 
        // Free the parsed JSON object
        cJSON_Delete(json_message);
    } else {
        printf("Received empty message on topic %s\n", message->topic);
    }
}

// Function to initialize the MQTT client
int mqtt_init() {
    // Initialize the Mosquitto library
    mosquitto_lib_init();

    // Create a new Mosquitto client instance
    mosq = mosquitto_new(NULL, true, NULL); // NULL client ID allows Mosquitto to generate a unique ID
    if (!mosq) {
        fprintf(stderr, "Failed to create Mosquitto client instance.\n");
        return -1;
    }

    mosquitto_message_callback_set(mosq, on_message);

    // Connect to the MQTT broker
    if (mosquitto_connect(mosq, BROKER_ADDRESS, BROKER_PORT, 60) != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Failed to connect to MQTT broker at %s:%d\n", BROKER_ADDRESS, BROKER_PORT);
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return -1;
    }
    printf("Connected to MQTT broker at %s:%d\n", BROKER_ADDRESS, BROKER_PORT);

    // Subscribe to the topics
    if (mqtt_subscribe("RB_DCU/RTList_Interval/#") != 0) {
        fprintf(stderr, "Failed to subscribe to RB_DCU/RTList_Interval topic.\n");
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return -1;
    }

    mqtt_subscribe("RB_DCU/Ineshdata_cloud/rf_command");


    // Start the periodic publisher threads
    pthread_t rt_list_thread;

   // Create the threads
    printf("Creating rt_list and show_routes publisher threads...\n");
    if (pthread_create(&rt_list_thread, NULL, generalized_publisher, NULL) != 0) {
        fprintf(stderr, "Failed to create rt_list publisher thread.\n");
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return -1;
    }
    pthread_detach(rt_list_thread);

    mosquitto_loop_start(mosq);
    return 0;
}

// Function to publish a message to an MQTT topic
int mqtt_publish(const uint8_t *message, int message_len, const char *topic) {
    if (!mosq) {
        fprintf(stderr, "MQTT client is not initialized.\n");
        return -1;
    }

    // Publish the message
    int ret = mosquitto_publish(mosq, NULL, topic, message_len, message, 0, false);
    if (ret != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Failed to publish message to topic %s: %s\n", topic, mosquitto_strerror(ret));
        return -1;
    }

    printf("Message published to topic %s\n", topic);
    return 0;
}

int mqtt_subscribe(const char *topic) {
    if (!mosq) {
        fprintf(stderr, "MQTT client is not initialized.\n");
        return -1;
    }

    // Subscribe to the topic
    int ret = mosquitto_subscribe(mosq, NULL, topic, 0);
    if (ret != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Failed to subscribe to topic %s: %s\n", topic, mosquitto_strerror(ret));
        return -1;
    }

    printf("Subscribed to topic %s\n", topic);
    return 0;
}

// Function to disconnect and clean up the MQTT client
void mqtt_cleanup() {
    if (mosq) {
        mosquitto_disconnect(mosq);
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        printf("MQTT client disconnected and cleaned up.\n");
    }
}
