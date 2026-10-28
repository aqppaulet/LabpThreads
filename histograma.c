#include "comun.h"

enum { CATEGORIAS = 10 };
typedef struct {
    const int *v;
    size_t inicio, fin;
    int usar_mutex;
    uint64_t local[CATEGORIAS];
    uint64_t *compartido;
    pthread_mutex_t *mutex;
} Tarea;

static void secuencial(const int *v, size_t n, uint64_t *h) {
    for (size_t i = 0; i < n; ++i) ++h[v[i] / 10];
}
static void *trabajar(void *arg) {
    Tarea *t = arg;
    uint64_t local[CATEGORIAS] = {0};
    for (size_t i = t->inicio; i < t->fin; ++i) {
        size_t categoria = (size_t)t->v[i] / 10;
        if (t->usar_mutex) {
            pt(pthread_mutex_lock(t->mutex));
            ++t->compartido[categoria];
            pt(pthread_mutex_unlock(t->mutex));
        } else {
            ++local[categoria];
        }
    }
    memcpy(t->local, local, sizeof local);
    return NULL;
}
static void paralelo(const int *v, size_t n, size_t p, int usar_mutex,
                     uint64_t *h, Tarea *t, pthread_t *hilos,
                     pthread_mutex_t *mutex) {
    for (size_t i = 0; i < p; ++i) {
        t[i].v = v; t[i].usar_mutex = usar_mutex;
        t[i].compartido = h; t[i].mutex = mutex;
        segmento(n, p, i, &t[i].inicio, &t[i].fin);
        pt(pthread_create(&hilos[i], NULL, trabajar, &t[i]));
    }
    for (size_t i = 0; i < p; ++i) {
        pt(pthread_join(hilos[i], NULL));
        if (!usar_mutex)
            for (size_t k = 0; k < CATEGORIAS; ++k) h[k] += t[i].local[k];
    }
}
int main(int argc, char **argv) {
    exigir(argc == 4, "Uso: ./histograma N hilos uniforme|concentrada");
    size_t n = numero(argv[1], 100000000), p = numero(argv[2], 256);
    int concentrada = strcmp(argv[3], "concentrada") == 0;
    exigir(concentrada || strcmp(argv[3], "uniforme") == 0, "Distribucion invalida.");
    int *v = reservar(n, sizeof *v);
    Tarea *t = reservar(p, sizeof *t);
    pthread_t *hilos = reservar(p, sizeof *hilos);
    pthread_mutex_t mutex;
    pt(pthread_mutex_init(&mutex, NULL));
    uint32_t semilla = 42;
    for (size_t i = 0; i < n; ++i)
        v[i] = concentrada ? 5 : (int)(aleatorio(&semilla) % 100);
    uint64_t s[CATEGORIAS] = {0}, m[CATEGORIAS] = {0}, l[CATEGORIAS] = {0};
    double inicio = reloj();
    secuencial(v, n, s);
    double ts = reloj() - inicio;
    inicio = reloj();
    paralelo(v, n, p, 1, m, t, hilos, &mutex);
    double tm = reloj() - inicio;
    inicio = reloj();
    paralelo(v, n, p, 0, l, t, hilos, &mutex);
    double tl = reloj() - inicio;
    uint64_t total = 0;
    for (size_t k = 0; k < CATEGORIAS; ++k) {
        exigir(s[k] == m[k] && s[k] == l[k], "ERROR: histogramas diferentes.");
        total += l[k];
    }
    exigir(total == n, "ERROR: el histograma no suma N.");
    printf("N,hilos,distribucion,seq_s,mutex_s,privado_s,total,correcto");
    for (size_t k = 0; k < CATEGORIAS; ++k) printf(",cat%zu", k);
    printf("\n%zu,%zu,%s,%.9f,%.9f,%.9f,%" PRIu64 ",1",
           n, p, argv[3], ts, tm, tl, total);
    for (size_t k = 0; k < CATEGORIAS; ++k) printf(",%" PRIu64, l[k]);
    putchar('\n');
    pt(pthread_mutex_destroy(&mutex));
    free(hilos); free(t); free(v);
    return EXIT_SUCCESS;
}
