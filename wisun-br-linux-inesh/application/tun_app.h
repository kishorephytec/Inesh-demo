#ifndef TUN_APP_H
#define TUN_APP_H

#define UDP_PORT 1234

#include "app_wsbrd/app/wsbrd.h"

// Function to send a UDP message
void tun_app_write(const char *meter_id, const char *message, int len);

// Function to read data from the TUN interface
void tun_app_read(struct wsbr_ctxt *ctxt);

// Function to parse data received from the TUN interface
void tun_parse(char *buf);

int tun_initialise(const char *tun_dev);

#endif // TUN_APP_H