# Laboratorio A: Pthreads en C

Solución de las tres partes: estadísticas, histograma y productores/consumidores.

## Compilación

Requisitos: GCC con Pthreads y Make. Python 3 se usa para pruebas y mediciones.

```bash
make
./estadisticas 10000000 4
./histograma 1000000 4 uniforme
./histograma 1000000 4 concentrada
./productores_consumidores 20000 2 2 256
```

- Estadísticas: `N hilos`. Cada hilo calcula resultados locales; el principal los combina después de esperar al trabajador correspondiente.
- Histograma: `N hilos uniforme|concentrada`. Compara referencia secuencial, un mutex global por incremento e histogramas privados.
- Buffer: `N productores consumidores capacidad`. N es el total conjunto. Utiliza un mutex, dos variables de condición y cero semáforos. Comprueba cada identificador exactamente una vez.

Los primeros `N % hilos` segmentos reciben un elemento adicional. `comun.h` debe estar junto a los tres archivos C. Los programas rechazan entradas inválidas y comprueban errores de memoria y Pthreads.

## Pruebas y mediciones

```bash
make pruebas
make mediciones
```

Las pruebas incluyen 203 casos: 20 de estadísticas, 40 de histograma, 135 del buffer y ocho entradas inválidas. Para estadísticas e histograma se emplean también referencias independientes en Python.

`make mediciones` genera localmente la carpeta `resultados/`: hace un calentamiento descartado y cinco repeticiones por configuración; guarda CSV crudos, resúmenes y características del entorno. Los tiempos se expresan en segundos y la eficiencia se guarda como fracción. La campaña comprende 43 configuraciones. La información generada no se publica automáticamente en este repositorio.

Los tiempos paralelos incluyen preparar las tareas, crear y esperar hilos y combinar resultados. Excluyen generación de datos y reservas de memoria. En el buffer incluyen la comprobación de identificadores bajo el mutex. No recompiles ni limpies los ejecutables mientras realizas mediciones.

El informe LaTeX y las capturas de la entrega se distribuyen por separado. Este repositorio contiene el código fuente y las herramientas para comprobarlo y repetir los experimentos.
