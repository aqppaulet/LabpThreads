#include "comun.h"

typedef struct {
    size_t *datos, capacidad, entrada, salida, cantidad;
    size_t activos, producidos, consumidos, n;
    unsigned char *vistos;
    int error;
    pthread_mutex_t mutex;
    pthread_cond_t no_lleno, no_vacio;
} Buffer;
typedef struct { Buffer *b; size_t inicio, fin; } Productor;

/* Todas las llamadas a estas funciones ocurren bajo el mutex,
   salvo en la referencia secuencial, donde no existe concurrencia. */
static void insertar(Buffer *b, size_t valor) {
    exigir(b->cantidad < b->capacidad, "ERROR: buffer lleno.");
    b->datos[b->entrada] = valor;
    b->entrada = (b->entrada + 1) % b->capacidad;
    ++b->cantidad; ++b->producidos;
}
static void extraer(Buffer *b) {
    exigir(b->cantidad > 0, "ERROR: buffer vacio.");
    size_t valor = b->datos[b->salida];
    b->salida = (b->salida + 1) % b->capacidad;
    --b->cantidad; ++b->consumidos;
    if (valor >= b->n || b->vistos[valor]) b->error = 1;
    else b->vistos[valor] = 1;
}
static void *producir(void *arg) {
    Productor *p = arg;
    Buffer *b = p->b;
    for (size_t i = p->inicio; i < p->fin; ++i) {
        pt(pthread_mutex_lock(&b->mutex));
        while (b->cantidad == b->capacidad)
            pt(pthread_cond_wait(&b->no_lleno, &b->mutex));
        insertar(b, i);
        pt(pthread_cond_signal(&b->no_vacio));
        pt(pthread_mutex_unlock(&b->mutex));
    }
    pt(pthread_mutex_lock(&b->mutex));
    if (--b->activos == 0) pt(pthread_cond_broadcast(&b->no_vacio));
    pt(pthread_mutex_unlock(&b->mutex));
    return NULL;
}
static void *consumir(void *arg) {
    Buffer *b = arg;
    for (;;) {
        pt(pthread_mutex_lock(&b->mutex));
        while (b->cantidad == 0 && b->activos > 0)
            pt(pthread_cond_wait(&b->no_vacio, &b->mutex));
        if (b->cantidad == 0 && b->activos == 0) {
            pt(pthread_mutex_unlock(&b->mutex));
            break;
        }
        extraer(b);
        pt(pthread_cond_signal(&b->no_lleno));
        pt(pthread_mutex_unlock(&b->mutex));
    }
    return NULL;
}
static void verificar(const Buffer *b) {
    exigir(!b->error && b->producidos == b->n && b->consumidos == b->n &&
           b->cantidad == 0, "ERROR: perdida, duplicacion o conteos inconsistentes.");
    for (size_t i = 0; i < b->n; ++i)
        exigir(b->vistos[i] == 1, "ERROR: identificador no consumido.");
}
int main(int argc, char **argv) {
    exigir(argc == 5, "Uso: ./productores_consumidores N productores consumidores capacidad");
    size_t n = numero(argv[1], 100000000), p = numero(argv[2], 128);
    size_t c = numero(argv[3], 128), capacidad = numero(argv[4], 100000000);
    Buffer b = {0};
    b.capacidad = capacidad; b.n = n;
    b.datos = reservar(capacidad, sizeof *b.datos);
    b.vistos = reservar(n, sizeof *b.vistos);
    pthread_t *hp = reservar(p, sizeof *hp), *hc = reservar(c, sizeof *hc);
    Productor *t = reservar(p, sizeof *t);
    double inicio = reloj();
    for (size_t i = 0; i < n; ++i) { insertar(&b, i); extraer(&b); }
    double ts = reloj() - inicio;
    verificar(&b);
    b.entrada = b.salida = b.cantidad = b.producidos = b.consumidos = 0;
    b.error = 0; b.activos = p;
    memset(b.vistos, 0, n * sizeof *b.vistos);
    pt(pthread_mutex_init(&b.mutex, NULL));
    pt(pthread_cond_init(&b.no_lleno, NULL));
    pt(pthread_cond_init(&b.no_vacio, NULL));
    inicio = reloj();
    for (size_t i = 0; i < c; ++i) pt(pthread_create(&hc[i], NULL, consumir, &b));
    for (size_t i = 0; i < p; ++i) {
        t[i].b = &b;
        segmento(n, p, i, &t[i].inicio, &t[i].fin);
        pt(pthread_create(&hp[i], NULL, producir, &t[i]));
    }
    for (size_t i = 0; i < p; ++i) pt(pthread_join(hp[i], NULL));
    for (size_t i = 0; i < c; ++i) pt(pthread_join(hc[i], NULL));
    double tp = reloj() - inicio;
    verificar(&b);
    printf("N,productores,consumidores,capacidad,seq_s,par_s,producidos,consumidos,correcto\n");
    printf("%zu,%zu,%zu,%zu,%.9f,%.9f,%zu,%zu,1\n",
           n, p, c, capacidad, ts, tp, b.producidos, b.consumidos);
    pt(pthread_cond_destroy(&b.no_lleno));
    pt(pthread_cond_destroy(&b.no_vacio));
    pt(pthread_mutex_destroy(&b.mutex));
    free(t); free(hc); free(hp); free(b.vistos); free(b.datos);
    return EXIT_SUCCESS;
}
