#include "comun.h"

typedef struct { int64_t suma; int minimo, maximo; } Resultado;
typedef struct { const int *v; size_t inicio, fin; Resultado r; } Tarea;

static Resultado calcular(const int *v, size_t inicio, size_t fin) {
    Resultado r = {0, INT_MAX, INT_MIN};
    for (size_t i = inicio; i < fin; ++i) {
        r.suma += v[i];
        if (v[i] < r.minimo) r.minimo = v[i];
        if (v[i] > r.maximo) r.maximo = v[i];
    }
    return r;
}
static void *trabajar(void *arg) {
    Tarea *t = arg;
    t->r = calcular(t->v, t->inicio, t->fin);
    return NULL;
}
static Resultado paralelo(const int *v, size_t n, size_t p,
                          pthread_t *hilos, Tarea *t) {
    for (size_t i = 0; i < p; ++i) {
        t[i].v = v;
        segmento(n, p, i, &t[i].inicio, &t[i].fin);
        pt(pthread_create(&hilos[i], NULL, trabajar, &t[i]));
    }
    Resultado r = {0, INT_MAX, INT_MIN};
    for (size_t i = 0; i < p; ++i) {
        pt(pthread_join(hilos[i], NULL));
        r.suma += t[i].r.suma;
        if (t[i].r.minimo < r.minimo) r.minimo = t[i].r.minimo;
        if (t[i].r.maximo > r.maximo) r.maximo = t[i].r.maximo;
    }
    return r;
}
int main(int argc, char **argv) {
    exigir(argc == 3, "Uso: ./estadisticas N hilos (1..256)");
    size_t n = numero(argv[1], 100000000), p = numero(argv[2], 256);
    int *v = reservar(n, sizeof *v);
    pthread_t *hilos = reservar(p, sizeof *hilos);
    Tarea *t = reservar(p, sizeof *t);
    uint32_t semilla = 42;
    for (size_t i = 0; i < n; ++i) v[i] = (int)(aleatorio(&semilla) % 2001) - 1000;
    double inicio = reloj();
    Resultado s = calcular(v, 0, n);
    double ts = reloj() - inicio;
    inicio = reloj();
    Resultado r = paralelo(v, n, p, hilos, t);
    double tp = reloj() - inicio;
    exigir(s.suma == r.suma && s.minimo == r.minimo && s.maximo == r.maximo,
           "ERROR: resultados diferentes.");
    printf("N,hilos,seq_s,par_s,suma,minimo,maximo,promedio,correcto\n");
    printf("%zu,%zu,%.9f,%.9f,%" PRId64 ",%d,%d,%.12f,1\n",
           n, p, ts, tp, r.suma, r.minimo, r.maximo, (double)r.suma / (double)n);
    free(t); free(hilos); free(v);
    return EXIT_SUCCESS;
}
