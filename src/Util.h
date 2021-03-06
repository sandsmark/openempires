#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <SDL2/SDL_net.h>

#define UTIL_TCP_SEND(socket, pointer) SDLNet_TCP_Send(socket, pointer, sizeof(*pointer))

#define UTIL_TCP_RECV(socket, pointer) SDLNet_TCP_Recv(socket, pointer, sizeof(*pointer))

#define UTIL_SORT(pointer, count, comparator) (qsort(pointer, count, sizeof(*(pointer)), comparator))

#define UTIL_ALLOC(type, count) (type*) calloc(count, sizeof(type))

#define UTIL_REALLOC(pointer, type, count) ((type*) realloc(pointer, sizeof(type) * count))

#define UTIL_WRAP(a, b) ((a) % (b))

#define UTIL_LEN(array) ((int32_t) (sizeof(array) / sizeof(*array)))

#define UTIL_FREAD(pointer, count, fp) (fread(pointer, sizeof(*pointer), count, fp))

#define UTIL_MIN(a, b) ((a) < (b) ? (a) : (b))

#define UTIL_MAX(a, b) ((a) > (b) ? (a) : (b))

void Util_Bomb(const char* const message, ...);

char* Util_StringJoin(const char* const a, const char* const b);

char* Util_StringDup(const char* s);

bool Util_StringEqual(const char* const a, const char* const b);

int32_t Util_Sqrt(const int64_t val);

int32_t Util_Pow(const int32_t value, const int32_t n);

void Util_Srand(const int32_t seed);

uint16_t Util_Rand(void);

bool Util_FlipCoin(void);
