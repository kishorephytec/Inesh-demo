#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <systemd/sd-bus.h>
#include <time.h>

char* call_decode_message(const char *hex_message, int len) {
    sd_bus *bus = NULL;
    sd_bus_message *msg = NULL;
    sd_bus_message *reply = NULL;
    int ret;
    char *result = NULL;

    // Connect to the system bus
    ret = sd_bus_default_system(&bus);
    if (ret < 0) {
        fprintf(stderr, "[ERROR][DBUS] Failed to connect to system bus: %s\n", strerror(-ret));
        return NULL;
    }

    // Create a new method call message
    ret = sd_bus_message_new_method_call(bus, &msg,
                                         "com.example.DecodeService", // Target service name
                                         "/com/example/DecodeService", // Object path
                                         "com.example.DecodeServiceInterface", // Interface name
                                         "decode_message"); // Method name
    if (ret < 0) {
        fprintf(stderr, "[ERROR][DBUS] Failed to create message: %s\n", strerror(-ret));
        goto cleanup;
    }

    // Create a hex buffer
    int hex_buffer_size = (len * 2) + 1;
    char *hex_buffer = (char *)malloc(hex_buffer_size);
    if (!hex_buffer) {
        fprintf(stderr, "[ERROR][DBUS] Memory allocation failed\n");
        goto cleanup;
    }
    
    for (int i = 0; i < len; i++) {
        snprintf(hex_buffer + (i * 2), 3, "%02X ", (unsigned char)hex_message[i]);
    }
    hex_buffer[hex_buffer_size - 1] = '\0';
    
    // Append the argument to the message
    ret = sd_bus_message_append(msg, "s", hex_buffer);
    free(hex_buffer);
    if (ret < 0) {
        fprintf(stderr, "[ERROR][DBUS] Failed to append argument: %s\n", strerror(-ret));
        goto cleanup;
    }

    // Call the method and get the reply
    ret = sd_bus_call(bus, msg, 0, NULL, &reply);
    if (ret < 0) {
        fprintf(stderr, "[ERROR][DBUS] Failed to send message: %s\n", strerror(-ret));
        goto cleanup;
    }

    // Read the response
    ret = sd_bus_message_read(reply, "s", &result);
    if (ret < 0) {
        fprintf(stderr, "[ERROR][DBUS] Failed to read response: %s\n", strerror(-ret));
        goto cleanup;
    }
printf("result: %s\n",result);
return result;
cleanup:
    sd_bus_message_unref(msg);
    sd_bus_message_unref(reply);
    sd_bus_unref(bus);
}

