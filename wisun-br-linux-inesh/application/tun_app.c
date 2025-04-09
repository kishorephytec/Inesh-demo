#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <net/if.h>

#include "common/iobuf.h"
#include "tun_app.h"
#include "app_wsbrd/app/wsbrd.h"
#include "json.h"
#include "mqtt.h"
#include "dlms.h"
#include "db.h"

// Function to send a message via UDP
void tun_app_write(const char *meter_id, const char *message, int len) {
    int sock;
    struct sockaddr_in6 server_addr;
    char ip_str[INET6_ADDRSTRLEN];
    uint8_t ip[16]; // Binary representation of the IPv6 address

    // Validate input parameters
    if (!meter_id || !message || len <= 0) {
        fprintf(stderr, "Invalid input to tun_app_write.\n");
        return;
    }

    // Retrieve the IP address associated with the given Meter ID from the database
    if (db_get_ip_by_meter_id(meter_id, ip_str, sizeof(ip_str)) != 0) {
        fprintf(stderr, "No IP address found for Meter ID: %s\n", meter_id);
        return;
    }

    // Convert the IP address from string to binary format
    if (inet_pton(AF_INET6, ip_str, ip) != 1) {
        fprintf(stderr, "Invalid IP address format for Meter ID: %s\n", meter_id);
        return;
    }

    // Create a socket
    sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return;
    }

    // Configure the server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(UDP_PORT); // Use predefined UDP port
    memcpy(&server_addr.sin6_addr, ip, 16); // Directly assign the binary IP address

#ifdef DEBUG
    // Print the IP address in hexadecimal format for debugging
    printf("Sending to IP: ");
    for (int i = 0; i < 16; i++) {
        printf("%02x", ip[i]);
        if (i < 15) printf(":");
    }
    printf("\n");

    // Print the message in hexadecimal format for debugging
    printf("Message: ");
    for (int i = 0; i < len; i++) {
        printf("%02x ", message[i]);
    }
    printf("\n");
#endif

    // Send the message
    if (sendto(sock, message, len, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to send message");
    } else {
        printf("Message sent successfully to Meter ID: %s\n", meter_id);
    }

    // Close the socket
    close(sock);
}

void tun_app_read(struct wsbr_ctxt *ctxt) {
    struct iobuf_read req = { };
    uint8_t buf[2000];
    struct sockaddr_in6 addr;
    socklen_t addr_len = sizeof(addr);
    uint8_t ip[16]; // Binary representation of the IPv6 address
    char publish_topic[60];
    char ip_str[INET6_ADDRSTRLEN];
    char meter_id[15]; // Buffer for Meter ID
    char* decoded_meter_id; // Dynamically allocated Meter ID
    int rc;

    req.data = buf;
    req.data_size = recvfrom(ctxt->tun_client_fd, buf, sizeof(buf), 0, (struct sockaddr *)&addr, &addr_len);

    // Copy the binary IPv6 address directly
    memcpy(ip, &addr.sin6_addr, 16);

#ifdef DEBUG
    printf("Received data from IP: ");
    for (int i = 0; i < 16; i++) {
        printf("%02x", ip[i]);
        if (i < 15) printf(":");
    }
    printf("\n");
#endif

    // Validate packet structure
    if (req.data_size < 12) { // Minimum size: C1 (1) + MAC_ADDRESS (8) + DATA_LENGTH (2) + FF (1)
        fprintf(stderr, "Invalid packet: too short.\n");
        return;
    }

#ifdef DEBUG
    uint8_t mac_address[8];
    memcpy(mac_address, &req.data[1], 8);
    printf("MAC Address: ");
    for (int i = 0; i < 8; i++) {
        printf("%02x", mac_address[i]);
        if (i < 7) printf(":");
    }
    printf("\n");
#endif

    // Extract DATA_LENGTH
    uint16_t data_length = (req.data[9] << 8) | req.data[10]; // Big-endian conversion
    if (data_length + 12 > req.data_size) { // Ensure DATA_LENGTH matches the packet size
        fprintf(stderr, "Invalid packet: DATA_LENGTH mismatch.\n");
        return;
    }

    // Extract DATA
    uint8_t *data = &req.data[11];
    uint8_t data_first_byte = data[0];

    inet_ntop(AF_INET6, ip, ip_str, sizeof(ip_str));

    // Check the first byte of the DATA field
    if (data_first_byte == 0x4D) {
        // Handle case where the first byte is 0x4D
        printf("First byte of DATA is 0x4D. Processing...\n");

        // Decode the Meter ID from the DATA
        decoded_meter_id = call_decode_message(data + 1, data_length - 1);
            printf("Decoded Meter ID: %s\n", decoded_meter_id);

            // Store the Meter ID and IP address in the database
            if (db_store_or_update(decoded_meter_id, ip_str) != 0) {
                fprintf(stderr, "Failed to store Meter ID and IP address in the database.\n");
            }

            char sub_topic[60];
            // Publish the JSON to the MQTT topic
            snprintf(sub_topic, sizeof(sub_topic), "%s%s", DCU_SUB_TOPIC, decoded_meter_id);
            printf("Subscribing to topic: %s\n", sub_topic);
            mqtt_subscribe(sub_topic);

            // Free the allocated memory for meter_id
            
    } else {
        // Handle other cases
        printf("First byte of DATA is not 0x4D. Publishing to MQTT...\n");

        // Find the Meter ID associated with the IP address
        rc = db_get_meter_id_by_ip(ip_str, meter_id, sizeof(meter_id));
        if (rc != 0) {
            fprintf(stderr, "No Meter ID found for the given IP.\n");
            return;
        }

        // Use the Meter ID to construct the MQTT topic
        char *json_string = create_json(meter_id, data, data_length);
        if (!json_string) {
            fprintf(stderr, "Failed to create JSON string.\n");
            return;
        }

        printf("Generated JSON: %s\n", json_string);

        // Publish the JSON to the MQTT topic
        snprintf(publish_topic, sizeof(publish_topic), "%s%s", DCU_PUB_TOPIC, meter_id);
        mqtt_publish((const uint8_t *)json_string, strlen(json_string), publish_topic);

        free(json_string); // Free the JSON string after publishing
    }
}

int tun_initialise(const char *tun_dev){
    
    int test_fd;
    struct sockaddr_in6 addr;
    int buf_size = 2*1024*1024;

    if (!tun_dev || strlen(tun_dev) >= IF_NAMESIZE) {
        fprintf(stderr, "Invalid TUN device name.\n");
        return -1;
    }
    // Create an IPv6 UDP socket
    test_fd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (test_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the TUN interface
    if (setsockopt(test_fd, SOL_SOCKET, SO_BINDTODEVICE, tun_dev, IF_NAMESIZE) < 0) {
        perror("setsockopt SO_BINDTODEVICE failed");
        close(test_fd);
        exit(EXIT_FAILURE);
    }

    if (setsockopt(test_fd, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(buf_size))<0){
	perror("setsockopt SO_BINDTODEVICE failed");
        close(test_fd);
        exit(EXIT_FAILURE);
    }

printf("size: %d\n",sizeof(buf_size)); 

    // Configure the address structure
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_addr = in6addr_any;  // Bind to all available IPv6 addresses
    addr.sin6_port = htons(UDP_PORT); // Set the port to 1234

    // Bind the socket to the address and port
    if (bind(test_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        close(test_fd);
        exit(EXIT_FAILURE);
    }

    printf("Socket bound to interface %s on port %d\n", tun_dev, UDP_PORT);

    return test_fd;

}
