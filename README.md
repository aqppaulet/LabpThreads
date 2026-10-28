# Laboratorio A: Pthreads en C

Solución de estadísticas, histograma y productores/consumidores con buffer acotado.

## Compilación y ejecución

Requisitos: GCC con Pthreads y Make. Python 3 se usa para pruebas y mediciones.

```bash
make
./estadisticas 10000000 4
./histograma 1000000 4 uniforme
./histograma 1000000 4 concentrada
./productores_consumidores 20000 2 2 256
```

- Estadísticas: `N hilos`. Cada hilo calcula resultados locales; el principal los combina después de esperar al trabajador correspondiente.
- Histograma: `N hilos uniforme|concentrada`. Compara referencia secuencial, un mutex global por incremento e histogramas privados por hilo.
- Buffer: `N productores consumidores capacidad`. N es el total conjunto. Usa un mutex, dos variables de condición y cero semáforos. Cada identificador se verifica exactamente una vez.

Los primeros `N % hilos` segmentos reciben un elemento adicional. `comun.h` debe estar junto a los tres archivos C. Se rechazan entradas inválidas y se comprueban errores de memoria y Pthreads.

## Pruebas y resultados

```bash
make pruebas
make mediciones
```

Las pruebas abarcan 203 casos: 20 de estadísticas, 40 de histograma, 135 del buffer y ocho entradas inválidas. Las estadísticas y categorías se contrastan también con referencias independientes en Python.

La campaña guardada en `resultados/` corresponde al 27 de octubre de 2026. Incluye 43 configuraciones con un calentamiento descartado y cinco repeticiones medidas por configuración. Los CSV crudos conservan cada muestra; los resúmenes contienen medianas, mínimos, máximos, speedup y eficiencia. La eficiencia se guarda como fracción, no porcentaje.

Los tiempos del CSV están en segundos. Incluyen preparar, crear y esperar hilos y combinar resultados, pero excluyen generación de datos y reservas. En el buffer incluyen la comprobación individual bajo el mutex. Las condiciones del equipo y el orden fijo de las implementaciones influyen en los tiempos. No recompiles ni limpies los ejecutables durante las mediciones.

## Informe y capturas

El informe LaTeX se entrega por separado, organizado en siete secciones y con enlace a este repositorio. El paquete `overleaf_pthreads.zip` contiene `main.tex` y `imagenes/`: se puede importar directamente en Overleaf y compilar con pdfLaTeX.

En la carpeta local de entrega, `python3 generar_informe.py` actualiza solo las cuatro tablas experimentales desde los CSV y conserva la redacción existente. Si se repiten las mediciones, hay que revisar también el análisis escrito.

Las capturas adicionales del 28 de octubre muestran ejecuciones individuales, no las medianas de la campaña original. Sus paneles presentan la salida CSV real por campos para facilitar la lectura; no se modifican los tiempos. El script local `capturar_resultados.py` necesita Google Chrome para volver a generar esas capturas. No se requiere Chrome para los programas, las pruebas ni las mediciones.
