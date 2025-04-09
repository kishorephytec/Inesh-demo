#ifndef DB_H
#define DB_H

#include <stddef.h>
// Define the fixed database path
#define DB_PATH "/var/lib/meter-database/meter_ip.db"

// Function to initialize the database and create the table if it doesn't exist
int db_initialize();

// Function to store or update the Meter ID and IP address in the database
int db_store_or_update(const char *meter_id, const char *ip_address);

// Function to retrieve the IP address for a given Meter ID
int db_get_ip_by_meter_id(const char *meter_id, char *ip_address, size_t ip_address_size);

// Function to retrieve the Meter ID for a given IP address
int db_get_meter_id_by_ip(const char *ip_address, char *meter_id, size_t meter_id_size);

// Function to retrieve the METER IDS for all meters
int db_get_all_meter_ids(char ***meter_id_list);

// Function to handle the subscription to existing meters
void subscribe_to_existing_meters();

#endif // DB_H
