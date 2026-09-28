#include "client-crypteMove.h"
#include "client.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;

typedef size_t usize;
void remove_elt(char *array, usize elt) {
  usize len = strlen(array);
  if (elt >= len) {
    return;
  }

  for (usize i = 0; i < len; i++) {
    if (i >= elt) {
      array[i] = array[i + 1];
    }
  }
}
void add_front(char *array, char c) {
  usize len = strlen(array);

  for (usize i = len + 1; i > 0; i--) {
    array[i] = array[i - 1];
  }

  array[0] = c;
}
void shift_left(char *array) {
  usize len = strlen(array);

  if (len <= 1) {
    return;
  }
  unsigned char first = array[0];
  for (usize i = 0; i < len; i++) {
    array[i] = array[i + 1];
  }
  array[len - 1] = first;
}
void shift_right(char *array) {
  usize len = strlen(array);

  if (len <= 1)
    return;

  char last = array[len - 1];

  for (usize i = len - 1; i > 0; i--) {
    array[i] = array[i - 1];
  }

  array[0] = last;
}

void encrypt(char *text, char *new_text) {
  usize i = 0;
  while (strlen(text) != 0) {

    unsigned char character = text[0];
    new_text[i] = character;
    remove_elt(text, 0);
    usize x = character % 8;

    usize length = strlen(text);
    if (x > length) {
      x = length;
    }
    for (usize j = 0; j < x; j++) {
      shift_left(text);
    }
    i++;
  }
  new_text[i] = '\0';
}
void decrypt(char *encrypted, char *text) {
  usize len = strlen(encrypted);

  text[0] = '\0';

  for (usize i = len; i > 0; i--) {
    unsigned char character = encrypted[i - 1];

    usize x = character % 8;
    usize current_len = strlen(text);

    if (x > current_len)
      x = current_len;

    for (usize j = 0; j < x; j++) {
      shift_right(text);
    }

    add_front(text, character);
  }
}
