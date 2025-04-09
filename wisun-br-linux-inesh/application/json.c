#include "json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h> // Ensure you have the cJSON library installed
#include <ctype.h>
#include <stddef.h>
#include <sys/utsname.h>
#include "app_wsbrd/app/wsbrd.h"

char *create_json(const char *meter_id, const uint8_t *data, int data_length) {
    // Create the root JSON object
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        fprintf(stderr, "Failed to create JSON object.\n");
        return NULL;
    }

    // Create the "rf_dlms" object
    cJSON *rf_dlms = cJSON_CreateObject();
    if (!rf_dlms) {
        fprintf(stderr, "Failed to create rf_dlms JSON object.\n");
        cJSON_Delete(root);
        return NULL;
    }
    cJSON_AddItemToObject(root, "rf_dlms", rf_dlms);

    // Add "Macid" to the "rf_dlms" object
    cJSON_AddStringToObject(rf_dlms, "Macid", meter_id);

    // Convert the data to a hexadecimal string
    char *data_hex = malloc(data_length * 2 + 1);
    if (!data_hex) {
        fprintf(stderr, "Failed to allocate memory for data_hex.\n");
        cJSON_Delete(root);
        return NULL;
    }
    for (int i = 0; i < data_length; i++) {
        sprintf(&data_hex[i * 2], "%02x", data[i]);
    }
    cJSON_AddStringToObject(rf_dlms, "data", data_hex);

    // Free the temporary data_hex buffer
    free(data_hex);

    // Convert the JSON object to a string
    char *json_string = cJSON_PrintUnformatted(root);
    if (!json_string) {
        fprintf(stderr, "Failed to print JSON string.\n");
        cJSON_Delete(root);
        return NULL;
    }

    // Clean up the JSON object
    cJSON_Delete(root);

    return json_string;
}

int hex_string_to_bytes(const char *hex_string, uint8_t *byte_array, size_t max_len) {
    size_t hex_len = strlen(hex_string);
    if (hex_len % 2 != 0) {
        fprintf(stderr, "Invalid hex string length.\n");
        return -1;
    }

    size_t byte_len = hex_len / 2;
    if (byte_len > max_len) {
        fprintf(stderr, "Byte array too small for hex string.\n");
        return -1;
    }

    for (size_t i = 0; i < byte_len; i++) {
        if (!isxdigit(hex_string[2 * i]) || !isxdigit(hex_string[2 * i + 1])) {
            fprintf(stderr, "Invalid hex character in string.\n");
            return -1;
        }
        sscanf(&hex_string[2 * i], "%2hhx", &byte_array[i]);
    }

    return byte_len; // Return the number of bytes written
}

cJSON *create_json_response(const char *data_type,int network) {

    struct utsname sys_info;

    cJSON *root = cJSON_CreateObject();
    if (!root) {
        fprintf(stderr, "Failed to create root JSON object.\n");
        return NULL;
    }

    // Create the "rf_command" object
    cJSON *rf_command = cJSON_CreateObject();
    if (!rf_command) {
        fprintf(stderr, "Failed to create rf_command JSON object.\n");
        cJSON_Delete(root);
        return NULL;
    }
    cJSON_AddItemToObject(root, "rf_command", rf_command);

    // Add the "network" field
    cJSON_AddNumberToObject(rf_command, "network", network);

    // Add the "data" field based on the data_type
    if (strcmp(data_type, "rt list") == 0) {
        // Fetch meter IDs from the database
        cJSON *meter_ids = cJSON_CreateArray();
        char **meter_id_list = NULL;
        int meter_count = db_get_all_meter_ids(&meter_id_list); // Fetch all meter IDs
        if (meter_count > 0) {
            for (int i = 0; i < meter_count; i++) {
                cJSON_AddItemToArray(meter_ids, cJSON_CreateString(meter_id_list[i]));
                free(meter_id_list[i]); // Free each meter ID after adding
            }
            free(meter_id_list); // Free the list itself
        }
        cJSON_AddItemToObject(rf_command, "data", meter_ids);
	char buff[50];
        snprintf(buff, sizeof(buff), "%d", meter_count);
        cJSON_AddStringToObject(rf_command, "count", buff);

    } else if (strcmp(data_type, "show routes") == 0) {
        // Fetch and add network routes (example data, replace with actual logic)
        cJSON *routes = cJSON_CreateArray();

        // Example: Add routes (replace with actual logic to fetch routes)
        cJSON *route1 = cJSON_CreateObject();
        cJSON_AddStringToObject(route1, "source", "meter_1");
        cJSON_AddStringToObject(route1, "destination", "meter_2");
        cJSON_AddItemToArray(routes, route1);

        cJSON *route2 = cJSON_CreateObject();
        cJSON_AddStringToObject(route2, "source", "meter_2");
        cJSON_AddStringToObject(route2, "destination", "meter_3");
        cJSON_AddItemToArray(routes, route2);

        cJSON_AddItemToObject(rf_command, "data", routes);

    } else if (strcmp(data_type, "env") == 0) {
        char ipv6_prefix_str[INET6_ADDRSTRLEN] = {0};
        inet_ntop(AF_INET6, &g_ctxt.config.ipv6_prefix, ipv6_prefix_str, sizeof(ipv6_prefix_str));
        
        char mac_address_str[18] = {0};
        snprintf(mac_address_str, sizeof(mac_address_str), "%02x:%02x:%02x:%02x:%02x:%02x",
         g_ctxt.config.ws_mac_address[0], g_ctxt.config.ws_mac_address[1],
         g_ctxt.config.ws_mac_address[2], g_ctxt.config.ws_mac_address[3],
         g_ctxt.config.ws_mac_address[4], g_ctxt.config.ws_mac_address[5]);

        cJSON *env_data = cJSON_CreateObject();
        if (uname(&sys_info) == 0) {
            cJSON_AddStringToObject(env_data, "build_name", sys_info.sysname);
            cJSON_AddStringToObject(env_data, "version", sys_info.version);
            cJSON_AddStringToObject(env_data, "product_id", sys_info.release);
        } else {
            cJSON_AddStringToObject(env_data, "build_name", "Unknown");
            cJSON_AddStringToObject(env_data, "version", "Unknown");
            cJSON_AddStringToObject(env_data, "product_id", "Unknown");
        }

        // Add hardcoded or fetched values for bootloader and radio
        cJSON_AddStringToObject(env_data, "bootloader", "b109");
        cJSON_AddStringToObject(env_data, "radio", "[V306_03355] 1.1");

        // Add other configuration details
        cJSON_AddStringToObject(env_data, "mac", mac_address_str);
        cJSON_AddStringToObject(env_data, "netname", g_ctxt.config.ws_name);
        cJSON_AddStringToObject(env_data, "global_prefix", ipv6_prefix_str);
        cJSON_AddNumberToObject(env_data, "pan_id", g_ctxt.config.ws_pan_id);
        cJSON_AddNumberToObject(env_data, "mtu", g_ctxt.config.lowpan_mtu);
        cJSON_AddNumberToObject(env_data, "transmit_level", g_ctxt.config.tx_power);
        cJSON_AddNumberToObject(env_data, "max_neighbours", g_ctxt.config.pan_size);

        // Use dwell intervals for unicast and broadcast
        cJSON_AddNumberToObject(env_data, "unicast", g_ctxt.config.uc_dwell_interval);
        cJSON_AddNumberToObject(env_data, "broadcast", g_ctxt.config.bc_dwell_interval);

        cJSON_AddItemToObject(rf_command, "data", env_data);

    } else {
        fprintf(stderr, "Unknown data type: %s\n", data_type);
        cJSON_Delete(root);
        return NULL;
    }

    return root;
}
