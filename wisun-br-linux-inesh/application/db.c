#include "db.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "mqtt.h"


// Function to initialize the database and create the table if it doesn't exist
int db_initialize() {
    sqlite3 *db;
    char *err_msg = NULL;
    int rc;

    // Open the database
    rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    // SQL statement to create the table if it doesn't exist
    const char *sql = "CREATE TABLE IF NOT EXISTS meter_ip_mapping ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "meter_id TEXT NOT NULL UNIQUE, "
                      "ip_address TEXT NOT NULL UNIQUE);";

    // Execute the SQL statement
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return -1;
    }

    printf("Database initialized and table created (if not exists).\n");

    // Close the database
    sqlite3_close(db);
    return 0;
}

int db_store_or_update(const char *meter_id, const char *ip_address) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;

    // Open the database
    rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    // Step 1: Try updating existing meter_id
    const char *update_sql = "UPDATE meter_ip_mapping SET ip_address = ? WHERE meter_id = ?;";
    rc = sqlite3_prepare_v2(db, update_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare update statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    sqlite3_bind_text(stmt, 1, ip_address, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, meter_id, -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE && sqlite3_changes(db) > 0) {
        printf("Updated IP for Meter ID: %s\n", meter_id);
        sqlite3_close(db);
        return 0;
    }

    // Step 2: Try updating existing ip_address
    const char *update_meter_sql = "UPDATE meter_ip_mapping SET meter_id = ? WHERE ip_address = ?;";
    rc = sqlite3_prepare_v2(db, update_meter_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare update statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    sqlite3_bind_text(stmt, 1, meter_id, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, ip_address, -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE && sqlite3_changes(db) > 0) {
        printf("Updated Meter ID for IP: %s\n", ip_address);
        sqlite3_close(db);
        return 0;
    }

    // Step 3: If neither update worked, insert a new row
    const char *insert_sql = "INSERT INTO meter_ip_mapping (meter_id, ip_address) VALUES (?, ?);";
    rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare insert statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    sqlite3_bind_text(stmt, 1, meter_id, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, ip_address, -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to insert new record: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    printf("Inserted new Meter ID: %s with IP: %s\n", meter_id, ip_address);
    return 0;
}


// Function to retrieve the IP address for a given Meter ID
int db_get_ip_by_meter_id(const char *meter_id, char *ip_address, size_t ip_address_size) {
    sqlite3 *db;
    int rc;

    // Open the database
    rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    // SQL statement to retrieve the IP address for the given Meter ID
    const char *sql = "SELECT ip_address FROM meter_ip_mapping WHERE meter_id = ?;";

    // Prepare the SQL statement
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    // Bind the Meter ID parameter
    sqlite3_bind_text(stmt, 1, meter_id, -1, SQLITE_STATIC);

    // Execute the SQL statement and retrieve the result
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const char *retrieved_ip = (const char *)sqlite3_column_text(stmt, 0);
        strncpy(ip_address, retrieved_ip, ip_address_size - 1);
        ip_address[ip_address_size - 1] = '\0'; // Ensure null termination
        printf("Retrieved IP Address: %s for Meter ID: %s\n", ip_address, meter_id);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 0;
    } else if (rc == SQLITE_DONE) {
        fprintf(stderr, "No IP address found for Meter ID: %s\n", meter_id);
    } else {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }

    // Finalize the statement and close the database
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return -1;
}

// Function to retrieve the Meter ID for a given IP address
int db_get_meter_id_by_ip(const char *ip_address, char *meter_id, size_t meter_id_size) {
    sqlite3 *db;
    int rc;

    // Open the database
    rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    // SQL statement to retrieve the Meter ID for the given IP address
    const char *sql = "SELECT meter_id FROM meter_ip_mapping WHERE ip_address = ?;";

    // Prepare the SQL statement
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    // Bind the IP address parameter
    sqlite3_bind_text(stmt, 1, ip_address, -1, SQLITE_STATIC);

    // Execute the SQL statement and retrieve the result
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const char *retrieved_meter_id = (const char *)sqlite3_column_text(stmt, 0);
        strncpy(meter_id, retrieved_meter_id, meter_id_size - 1);
        meter_id[meter_id_size - 1] = '\0'; // Ensure null termination
        printf("Retrieved Meter ID: %s for IP Address: %s\n", meter_id, ip_address);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 0;
    } else if (rc == SQLITE_DONE) {
        fprintf(stderr, "No Meter ID found for IP Address: %s\n", ip_address);
    } else {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }

    // Finalize the statement and close the database
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return -1;
}

int db_get_all_meter_ids(char ***meter_id_list) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc, count = 0;
    char **list = NULL;

    rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    const char *sql = "SELECT meter_id FROM meter_ip_mapping";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *meter_id = (const char *)sqlite3_column_text(stmt, 0);
        list = realloc(list, sizeof(char *) * (count + 1));
        list[count] = strdup(meter_id);
        count++;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    *meter_id_list = list;
    return count;
}

void subscribe_to_existing_meters() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;

    // Open the database
    rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    // Query to fetch all meter IDs
    const char *sql = "SELECT meter_id FROM meter_ip_mapping;";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Loop through the results
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *meter_id = (const char *)sqlite3_column_text(stmt, 0);
        if (meter_id) {
            char sub_topic[60];
            snprintf(sub_topic, sizeof(sub_topic), "%s%s", DCU_SUB_TOPIC, meter_id);
            printf("Subscribing to topic: %s\n", sub_topic);
            mqtt_subscribe(sub_topic);
        }
    }

    // Cleanup
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}
