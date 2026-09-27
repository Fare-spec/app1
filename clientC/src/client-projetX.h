#ifndef CLIENT_PROJETX_H
#define CLIENT_PROJETX_H

#include <stdint.h>

uint8_t find_offset(char *text);
void create_offset(const char *text, uint8_t offset, char *new_text);

#endif
