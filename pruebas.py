"""Pruebas de referencia, particion desigual, entradas invalidas y terminacion."""
import csv
import io
import subprocess


def ejecutar(programa, *args):
    salida = subprocess.run([f"./{programa}", *map(str, args)],
                            capture_output=True, text=True, check=True, timeout=20)
    fila = next(csv.DictReader(io.StringIO(salida.stdout)))
    assert fila["correcto"] == "1"
    return fila


def datos(n, modulo, desplazamiento=0):
    estado = 42
    for _ in range(n):
        estado = (estado * 1664525 + 1013904223) & 0xFFFFFFFF
        yield estado % modulo + desplazamiento


casos = 0
for n in (1, 3, 17, 1003, 100000):
    valores = list(datos(n, 2001, -1000))
    hist = [0] * 10
    for valor in datos(n, 100):
        hist[valor // 10] += 1
    for p in (1, 2, 4, 8):
        r = ejecutar("estadisticas", n, p)
        assert (int(r["suma"]), int(r["minimo"]), int(r["maximo"])) == (
            sum(valores), min(valores), max(valores))
        assert abs(float(r["promedio"]) - sum(valores) / n) < 1e-9
        casos += 1
        for distribucion in ("uniforme", "concentrada"):
            r = ejecutar("histograma", n, p, distribucion)
            esperado = hist if distribucion == "uniforme" else [n] + [0] * 9
            assert [int(r[f"cat{k}"]) for k in range(10)] == esperado
            assert int(r["total"]) == n
            casos += 1

for _ in range(3):
    for n in (1, 17, 1003):
        for p, c in ((1, 1), (2, 2), (4, 4), (2, 4), (4, 2)):
            for capacidad in (1, 7, 256):
                r = ejecutar("productores_consumidores", n, p, c, capacidad)
                assert int(r["producidos"]) == int(r["consumidos"]) == n
                casos += 1

invalidos = [
    ("estadisticas", "0", "2"), ("estadisticas", "-1", "2"),
    ("estadisticas", "17x", "2"), ("estadisticas", "17", "0"),
    ("estadisticas", "17", "257"), ("histograma", "17", "2", "otra"),
    ("productores_consumidores", "17", "2", "2", "0"),
    ("productores_consumidores", "17", "0", "2", "8"),
]
for programa, *args in invalidos:
    r = subprocess.run([f"./{programa}", *args], capture_output=True, timeout=5)
    assert r.returncode != 0
    casos += 1
print(f"OK: {casos} casos; referencias independientes y comprobacion exacta de IDs.")
