#ifndef DLMS_H
#define DLMS_H

#include <stddef.h> // For size_t

//Main function to decode a DLMS message and extract the specific value
char* call_decode_message(const char *hex_message, int len);

#endif // DLMS_H
