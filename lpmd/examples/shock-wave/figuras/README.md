# Figuras de la guía de clase

Las figuras se crean con Matplotlib mediante `generar_figuras.py`. No usan
imágenes generadas por IA. Las ilustraciones conceptuales se distinguen de los
gráficos calculados a partir de simulaciones reales.

| Archivo | Procedencia y significado |
| --- | --- |
| `esquema-choque.png` | Esquema sin escala: dos mitades se acercan a ±400 m/s; después crece la zona central comprimida y aparecen descargas en la unión periódica. Conserva el número de símbolos entre paneles. Los símbolos ilustran el material; no son posiciones atómicas exportadas. |
| `hilos.png` | Reparto ideal de las mismas doce tareas entre uno o cuatro trabajadores. Ilustra concurrencia; no representa tiempos medidos ni promete una aceleración de cuatro veces. |
| `perfiles.png` | Datos de `perfiles-referencia.csv`, exportados con `../profiles.py` desde la trayectoria real de `../shock.control`. Muestra 0.5, 1 y 2 ps, con una ampliación de la zona central entre 70 y 190 Å. Normaliza la densidad por `n0 = 4 / 5.4³` átomos/Å³. |
| `tiempos.png` | Columna `simulation_seconds` de `tiempos-referencia.csv`, extraída de los registros reales de LPMD: geometría de 6912 átomos, 200 pasos y tres repeticiones con 1, 2 y 4 hilos, sin trayectoria XYZ. Las barras muestran medianas de `Elapsed real time`; las etiquetas añaden la aceleración respecto de un hilo. |

Los CSV de referencia deben proceder de ejecuciones completadas del ejemplo.
No se construyen curvas ni tiempos ficticios cuando faltan esos datos. El equipo,
la compilación y las condiciones de la medición deben acompañar a los archivos de
referencia al actualizar las figuras.

## Referencia incluida

Los datos incluidos se obtuvieron el **17 de octubre de 2026** en un
**AMD Ryzen 5 5600U**, con **GCC 14.2**, compilación `Release` y
`LPMD_ENABLE_OPENMP=ON`.

- `perfiles-referencia.csv`: corrida completa de `shock.control`, con 6912
  átomos de Ar, paso temporal de 0.5 fs y 4000 pasos (2 ps). El CSV conserva
  40 configuraciones y 24 franjas por configuración: 960 filas de datos. La
  suma de átomos de cada configuración es 6912.
- `tiempos-referencia.csv`: nueve mediciones reales del mismo sistema, con
  200 pasos y tres repeticiones por número de hilos. Se omitió la trayectoria
  XYZ de todos los casos; se conservaron el monitor y el estado final.
  El archivo original procede de `../benchmark.py`; sus columnas y valores
  `wall_seconds`, que miden el proceso completo desde el lanzamiento, se conservan.
  Se añadió `simulation_seconds`, extraída de la línea `Elapsed real time` de
  cada registro `threads-<hilos>-run-<repetición>/run.log`. Se verificaron nueve
  registros completos, todos con `SIMULATION FINISHED`.

La figura y la tabla siguientes usan **`simulation_seconds`**, el mismo tiempo
que los estudiantes leen al finalizar LPMD. Este intervalo incluye el bucle de
simulación, sus salidas y el guardado del estado final; comienza después de la
preparación y del primer cálculo de fuerzas. No incluye todo el arranque del
proceso. Los tiempos de CPU `Elapsed user time` y `Elapsed system time` no se
usan para esta comparación.

| Hilos | Mediana (s) | Aceleración respecto de un hilo |
| --- | --- | --- |
| 1 | 16.861600 | 1.00× |
| 2 | 9.258410 | 1.82× |
| 4 | 5.521720 | 3.05× |

Estos valores documentan las figuras de referencia; no son tiempos esperados
para otros equipos. Los CSV son las fuentes de las figuras numéricas y permiten
regenerarlas sin volver a ejecutar la simulación.

## Regenerar las imágenes

Desde esta carpeta, con Python 3 y Matplotlib disponibles:

```bash
python3 generar_figuras.py
```

El generador busca los dos CSV de referencia junto al script. Si faltan, crea
solo los esquemas e informa qué datos están pendientes. También acepta rutas
explícitas a los datos:

```bash
python3 generar_figuras.py --profiles /ruta/perfiles.csv --times /ruta/times.csv
```

El archivo indicado con `--times` debe incluir `simulation_seconds`, además de
los hilos y pasos. El CSV original de `benchmark.py`, que contiene únicamente
`wall_seconds`, requiere primero extraer el tiempo interno de sus registros.

La generación sustituye los PNG de esta carpeta para actualizar la guía. No
modifica los CSV, los controles ni los resultados originales de la simulación.
Las figuras se guardan a 300 ppp y con fondo blanco; la fuente editable de cada
figura está en el script.
