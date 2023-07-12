#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void utils_random_string(char *s, const int len);
void utils_get_sha1(const char *input, size_t input_len, const char *key, unsigned char *output);


typedef struct Buffer {

  uint8_t *data;
  int size;
  int head;
  int tail;

} Buffer;

int utils_get_ipv4addr(char *hostname, char *ipv4addr, size_t size);

int utils_is_valid_ip_address(char *ip_address);

Buffer* utils_buffer_new(int size);

int utils_buffer_push(Buffer *rb, const uint8_t *data, int size);
   
int utils_buffer_pop(Buffer *rb, uint8_t *data, int size);

void utils_buffer_clear(Buffer *rb);

#endif // UTILS_H_
