"""Cinco repeticiones medidas por configuracion, tras una de calentamiento."""
import csv
import io
import platform
import statistics
import subprocess
from datetime import datetime
from pathlib import Path

SALIDA = Path("resultados")
SALIDA.mkdir(exist_ok=True)
REPETICIONES = 5


def ejecutar(programa, args):
    r = subprocess.run([f"./{programa}", *map(str, args)], check=True,
                       capture_output=True, text=True, timeout=60)
    fila = next(csv.DictReader(io.StringIO(r.stdout)))
    assert fila["correcto"] == "1"
    return fila


def guardar(nombre, filas):
    with (SALIDA / nombre).open("w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(filas[0]))
        w.writeheader()
        w.writerows(filas)


def medir(programa, configuraciones, tiempos):
    crudo, resumen = [], []
    for args in configuraciones:
        ejecutar(programa, args)  # Calentamiento, excluido de las medianas.
        muestras = []
        for repeticion in range(1, REPETICIONES + 1):
            fila = ejecutar(programa, args)
            crudo.append({"repeticion": repeticion, **fila})
            muestras.append(fila)
        fila = dict(muestras[0])
        for campo in tiempos:
            valores = [float(m[campo]) for m in muestras]
            fila[campo] = statistics.median(valores)
            fila[campo + "_min"] = min(valores)
            fila[campo + "_max"] = max(valores)
        p = int(fila.get("hilos", 1))
        for campo in tiempos[1:]:
            s = fila["seq_s"] / fila[campo]
            fila[campo + "_speedup"] = s
            if "hilos" in fila:
                fila[campo + "_eficiencia"] = s / p
        resumen.append(fila)
        print(programa, *args, "OK", flush=True)
    guardar(programa + "_crudo.csv", crudo)
    guardar(programa + "_resumen.csv", resumen)


entorno = [f"Fecha: {datetime.now().astimezone().isoformat()}",
           f"Sistema: {platform.platform()}",
           "Compilacion: gcc -O2 -std=c11 -Wall -Wextra -Wpedantic -Werror -pthread",
           "Calentamientos: 1; repeticiones: 5; estadistico: mediana",
           "Semilla: 42; ejecucion sin afinidad ni exclusividad del equipo"]
for comando in (["gcc", "--version"], ["lscpu"], ["free", "-b"]):
    entorno.append(subprocess.check_output(comando, text=True))
(SALIDA / "entorno.txt").write_text("\n".join(entorno))

medir("estadisticas", [(n, p) for n in (100000, 1000000, 10000000)
                       for p in (1, 2, 4, 8)], ["seq_s", "par_s"])
medir("histograma", [(n, p, "uniforme") for n in (10000, 100000, 1000000)
                     for p in (1, 2, 4, 8)] +
                    [(1000000, p, "concentrada") for p in (1, 2, 4, 8)],
      ["seq_s", "mutex_s", "privado_s"])
medir("productores_consumidores",
      [(20000, p, c, b) for p, c in ((1, 1), (2, 2), (4, 4), (2, 4), (4, 2))
       for b in (1, 16, 256)], ["seq_s", "par_s"])
