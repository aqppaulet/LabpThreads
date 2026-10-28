#ifndef COMUN_H
#define COMUN_H
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static inline void exigir(int ok, const char *mensaje) {
    if (!ok) { fprintf(stderr, "%s\n", mensaje); exit(EXIT_FAILURE); }
}
static inline void pt(int error) {
    if (error) { fprintf(stderr, "Pthreads: %s\n", strerror(error)); exit(EXIT_FAILURE); }
}
static inline size_t numero(const char *s, size_t maximo) {
    char *fin;
    errno = 0;
    unsigned long long n = strtoull(s, &fin, 10);
    exigir(*s >= '0' && *s <= '9' && !errno && !*fin && n > 0 && n <= maximo,
           "Parametro invalido: se requiere un entero positivo dentro del limite.");
    return (size_t)n;
}
static inline void *reservar(size_t n, size_t tam) {
    exigir(n && tam && n <= SIZE_MAX / tam, "Tamano de reserva invalido.");
    void *p = calloc(n, tam);
    exigir(p != NULL, "Memoria insuficiente.");
    return p;
}
static inline double reloj(void) {
    struct timespec t;
    exigir(clock_gettime(CLOCK_MONOTONIC, &t) == 0, "Fallo del reloj.");
    return (double)t.tv_sec + (double)t.tv_nsec * 1e-9;
}
/* Los primeros n % p segmentos reciben un elemento adicional. */
static inline void segmento(size_t n, size_t p, size_t id, size_t *a, size_t *b) {
    size_t q = n / p, r = n % p;
    *a = id * q + (id < r ? id : r);
    *b = *a + q + (id < r);
}
static inline uint32_t aleatorio(uint32_t *estado) {
    *estado = *estado * UINT32_C(1664525) + UINT32_C(1013904223);
    return *estado;
}
#endif
