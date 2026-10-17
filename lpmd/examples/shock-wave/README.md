# Ondas de choque con OpenMP

Dos mitades de un cristal FCC de argón se encuentran y producen compresión
central. La condición inicial es determinista, sin velocidades térmicas
aleatorias, para comparar ejecuciones con distintos números de hilos.

## Guía de clase

La [guía en PDF](guia-clase.pdf) tiene cinco páginas con esquemas, perfiles
comentados, instrucciones y espacios para responder. Su [fuente LaTeX](guia-clase.tex)
utiliza las imágenes de `figuras/`.

La práctica parte de una instalación de LPMD hecha con CMake y `sudo`. Cada
estudiante conserva sus rutas. Se comprueba OpenMP en la instalación activa;
si se descargaron cambios del código, se recompila y reinstala para incorporarlos,
aunque OpenMP ya estuviera activo. Las simulaciones y las mediciones se
hacen directamente con `lpmd`. Python se usa después para analizar y graficar.

Antes de buscar el ejemplo, abre una terminal en la raíz del repositorio que
clonaste y actualiza tu copia:

```bash
git pull --ff-only
```

Esto descarga los cambios de las fuentes y los ejemplos. No actualiza por sí
solo el ejecutable instalado: para eso se recompila y reinstala, como explica
el paso 2. Si Git informa cambios locales incompatibles o ramas divergentes,
revisa esos cambios con el docente antes de continuar.

Después de actualizar, copia este directorio a una carpeta de trabajo usando el administrador de
archivos. Dentro de la copia, crea `prueba`, `choque` y `tiempos`. Mantén
`profiles.py` y `plot_results.py` en la carpeta que contiene esas tres carpetas.

## 1. Comprobar el LPMD instalado

Copia `smoke.control` a `prueba` y abre una terminal allí:

```bash
command -v lpmd
OMP_NUM_THREADS=2 OMP_DYNAMIC=FALSE OMP_DISPLAY_ENV=TRUE \
  lpmd smoke.control > run.log 2>&1
```

El primer comando muestra la ruta del ejecutable activo. Abre `run.log` con un
editor y busca `SIMULATION FINISHED`, `OPENMP DISPLAY ENVIRONMENT`, `_OPENMP`
y `OMP_NUM_THREADS = '2'`. Los primeros indicadores confirman que terminó la
prueba y se cargó OpenMP; el último indica los hilos solicitados. La aceleración
se mide después. `lpmd --help` o las bibliotecas del ejecutable principal no
bastan para comprobar OpenMP: el código de fuerzas se carga mediante plugins.

Esta prueba tiene 1152 átomos y 100 pasos de 0.5 fs. Produce `smoke.xyz`,
`smoke-monitor.dat` y `smoke-final.dump`; no dura lo suficiente para analizar
un frente desarrollado. Si termina sin indicadores de OpenMP, sigue el paso 2.
Si hay errores de simulación, resuélvelos antes de interpretar el resultado.

## 2. Recompilar e instalar el código actualizado

Haz este paso después de descargar cambios del código y también si la prueba
no muestra soporte OpenMP. Descargar las fuentes no actualiza el ejecutable ni
los plugins ya instalados. Si ya reinstalaste ese código y la prueba confirma
OpenMP, puedes pasar al paso 3.

Localiza el directorio donde compilaste este código de lpmd2: debe contener
su `CMakeCache.txt`, y las fuentes originales deben seguir en su ubicación.
El nombre puede ser `build` u otro. En ese archivo comprueba que
`CMAKE_INSTALL_PREFIX` corresponde a la instalación que mostró `command -v lpmd`;
por ejemplo, `/usr/local` para `/usr/local/bin/lpmd`.

Abre una terminal en ese directorio de compilación:

```bash
cmake . -DLPMD_ENABLE_OPENMP=ON -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel 4
sudo cmake --install .
```

CMake reutiliza las fuentes, el compilador y el prefijo registrados en el caché.
Se necesita CMake >= 3.16, un compilador C++17 con OpenMP y los archivos de
desarrollo de zlib. Con GCC, el runtime es `libgomp`. Si falta soporte OpenMP,
la configuración con `ON` falla; no hace falta añadir `-fopenmp` manualmente.
OpenGL/GLUT es opcional para esta práctica.

`--parallel 4` reparte la compilación entre procesos. `OMP_NUM_THREADS=4`
controla separadamente los hilos de la simulación. OpenMP está desactivado por
defecto y esa variable no paraleliza una instalación secuencial.

Vuelve a `prueba` y repite el paso 1. La reinstalación debe actualizar la copia
que realmente ejecutas. Si hay varias, usa la ruta completa de la recién
instalada en lugar de `lpmd` durante toda la práctica. Las líneas
`Plugin file found at` del registro permiten comprobar el origen de los módulos;
un `LPMD_PATH` de una sesión anterior puede seleccionar plugins antiguos.
Si no conservas el caché o las rutas no coinciden, revisa con el docente la
[guía de instalación](../../../docs/installation-guide/guia-instalacion.pdf).

## 3. Ejecutar el choque

Copia `shock.control` a `choque` y abre una terminal allí:

```bash
OMP_NUM_THREADS=4 OMP_DYNAMIC=FALSE lpmd shock.control > run.log 2>&1
```

Usa 1, 2 o 4 hilos según los núcleos disponibles para tu trabajo. No ejecutes
otras simulaciones al mismo tiempo. Confirma `SIMULATION FINISHED` al terminar.
Las salidas se escriben en el directorio de trabajo; una nueva ejecución allí
reemplaza los archivos, así que usa una carpeta nueva para conservar otra corrida.

Parámetros de `shock.control`:


| Parámetro | Valor |
| --- | --- |
| Cristal | FCC Ar, 6 × 6 × 48 celdas, 6912 átomos |
| Celda | 32.4 × 32.4 × 259.2 Å, periódica en las tres direcciones |
| Potencial | Lennard-Jones: sigma = 3.41 Å, epsilon = 0.0103408 eV, cutoff = 8.5 Å |
| Velocidad inicial | Mitad inferior: +0.004 Å/fs; superior: −0.004 Å/fs (±400 m/s) |
| Integración | Velocity Verlet, dt = 0.5 fs, 4000 pasos = 2 ps |
| Vecinos | `linkedcell`, 3 × 3 × 24 subceldas de 10.8 Å |
| Salidas | Cada 100 pasos (50 fs) |


Una hora frente al computador es tiempo de reloj. El tiempo físico del modelo
es `steps × dt`: aquí 2 ps. Cambiar hilos conserva ese tiempo físico; aumentar
`dt` cambia la resolución y exige comprobar estabilidad y convergencia.

## 4. Graficar los perfiles

Desde `choque`, donde quedaron las salidas:

```bash
python3 ../profiles.py shock.xyz --output perfiles.csv
python3 ../plot_results.py perfiles.csv
```

Abre `shock-profiles.png` con el administrador de archivos. Muestra densidad
relativa y velocidad media a 0.5, 1 y 2 ps. El exportador usa Python estándar;
el graficador requiere Matplotlib, que en Debian/Ubuntu se instala con
`sudo apt-get install python3-matplotlib`. Ninguno compila ni ejecuta LPMD.

El XYZ contiene 40 configuraciones: `Ar x y z vx vy vz`, en Å y Å/fs,
comenzando en el paso 100. `profiles.py` agrupa 24 franjas de 10.8 Å y exporta
tiempo, centro de franja, número de átomos, densidad numérica en Å⁻³ y velocidad
media en m/s. La densidad de referencia es `4 / 5.4³ ≈ 0.02540 Å⁻³`.
Busca compresión y velocidad media más cercana a cero en el centro.
Si cambias celda, `dt` o `each`, indica sus valores a `profiles.py`: XYZ no
contiene esa información. El CSV y las figuras deben tener destinos nuevos;
para otra figura puedes usar `--output-dir otra-carpeta`.

## 5. Comparar tiempos manualmente

`tiempos.control` conserva la geometría, las fuerzas, el paso temporal y las
velocidades iniciales de `shock.control`, pero integra 200 pasos y omite XYZ.
Es una medición breve del mismo sistema, con monitor y estado final en todos
los casos. No se usa para estudiar la evolución completa a 2 ps.

Dentro de `tiempos`, crea `1-hilo`, `2-hilos` y `4-hilos`, y copia el mismo
`tiempos.control` en cada carpeta. Abre una terminal en cada una y ejecuta
la línea correspondiente:

```bash
OMP_NUM_THREADS=1 OMP_DYNAMIC=FALSE lpmd tiempos.control > run.log 2>&1
OMP_NUM_THREADS=2 OMP_DYNAMIC=FALSE lpmd tiempos.control > run.log 2>&1
OMP_NUM_THREADS=4 OMP_DYNAMIC=FALSE lpmd tiempos.control > run.log 2>&1
```

Repite tres veces por número de hilos. Tras cada corrida, confirma el final,
anota `Elapsed real time` y renombra `run.log` a `r1.log`, `r2.log` o `r3.log`
con el administrador de archivos antes de repetir. El monitor y el estado
final se reemplazan. Usa solo los números de hilos disponibles para tu trabajo.

`Elapsed real time` mide el tiempo de reloj de la simulación y sus salidas,
después de la preparación inicial. No incluye todo el arranque del proceso.
`user` y `system` son tiempos de CPU y pueden sumar el trabajo de varios hilos.
Ordena los tres tiempos y toma el del medio, la mediana. Calcula
`S_p = mediana_1_hilo / mediana_p_hilos`; `S=2` significa la mitad de espera.
OpenMP no garantiza una mejora proporcional al número de hilos.

Los datos y las [figuras de referencia](figuras/README.md) documentan una
medición real y el método para regenerarlas. Los resultados dependen del
procesador, carga, memoria y salidas del equipo.

## Interpretación física

`shock-monitor.dat` registra paso, energía potencial, cinética y total,
temperatura, presión y `szz`. Las energías son del sistema completo, en eV;
temperatura en K, presión y esfuerzo configuracional `szz` en MPa. `szz` es la
contribución de las interacciones, no toda la presión longitudinal. La temperatura
global incluye el movimiento colectivo de las dos mitades: para estudiar
calentamiento hay que restar el flujo medio **local** antes de calcularla.
`shock-final.dump` guarda el estado final en el formato de reinicio de lpmd.
El cristal frío con este parámetro de red parte ligeramente en tensión: la
presión global puede ser negativa al inicio aunque el centro se comprima.

Este es un modelo didáctico de compresión transitoria. El encuentro central
genera dos frentes; la unión periódica en el extremo genera una descarga.
Estudia el intervalo previo a su interacción. No se aplica termostato durante
el choque. Para un estudio cuantitativo se necesitan equilibración térmica,
convergencia de tamaño y paso temporal, y condiciones de contorno adecuadas al
experimento. El potencial de este ejemplo tiene un corte sin desplazamiento
energético, de modo que los cruces del cutoff pueden producir pequeños cambios
en la energía total.

Si cambias la celda, `dt` o `each`, pasa sus nuevos valores a `profiles.py`;
XYZ no contiene esa información. El script está preparado para salidas que
empiezan en el paso `each`, como estos controles.

## Material para el docente

`benchmark.py` sigue disponible para mediciones automatizadas fuera de la
actividad manual. Mide el proceso completo, por lo que sus `wall_seconds` no
son exactamente el mismo intervalo que `Elapsed real time`. La guía distingue
ambas métricas y usa la segunda para la tabla y la figura de referencia.

El cálculo paralelo acumula fuerzas y esfuerzos por hilo y luego los combina.
El orden de suma puede cambiar los últimos dígitos. La regresión de fuerzas
comprueba equivalencia numérica con tolerancias; los buffers de aceleración
requieren aproximadamente `24 × átomos × hilos` bytes adicionales.

Para recompilar la guía desde este directorio con LaTeX instalado:

```bash
mkdir -p /tmp/lpmd2-guia-clase
latexmk -pdf -interaction=nonstopmode -halt-on-error \
  -outdir=/tmp/lpmd2-guia-clase guia-clase.tex
cp /tmp/lpmd2-guia-clase/guia-clase.pdf .
```
