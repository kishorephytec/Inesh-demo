#ifndef JSON_H
#define JSON_H

#include <stdint.h>
#include <stddef.h>
#include <cjson/cJSON.h>

// Function to create a JSON string using Meter ID and data
char *create_json(const char *meter_id, const uint8_t *data, int data_length);
int hex_string_to_bytes(const char *hex_string, uint8_t *byte_array, size_t max_len);
cJSON *create_json_response(const char *data_type,int network);

#endif // JSON_H