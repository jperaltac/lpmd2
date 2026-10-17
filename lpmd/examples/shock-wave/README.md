# Ondas de choque con OpenMP

Este ejemplo muestra la compresión que aparece cuando dos mitades de un cristal
FCC de argón se mueven una contra otra. La condición inicial es determinista,
sin velocidades térmicas aleatorias, para poder comparar ejecuciones con
distintos números de hilos. No requiere pantalla ni archivos de entrada externos.

## Guía para una clase

La [guía de estudiantes en PDF](guia-clase.pdf) resume la práctica en tres páginas:
compilación y verificación, corrida y comparación de tiempos, gráficos y preguntas
de análisis. Su [fuente LaTeX](guia-clase.tex) es editable y no requiere imágenes
externas. El nuevo graficador `plot_results.py` necesita Matplotlib; en
Debian/Ubuntu se instala con `sudo apt-get install python3-matplotlib`.

Para recompilar el PDF desde este directorio con LaTeX instalado:

```bash
mkdir -p /tmp/lpmd2-guia-clase
latexmk -pdf -interaction=nonstopmode -halt-on-error \
  -outdir=/tmp/lpmd2-guia-clase guia-clase.tex
cp /tmp/lpmd2-guia-clase/guia-clase.pdf .
```

## Compilar en Linux

Se necesita CMake >= 3.16, un compilador C++17 con OpenMP y los archivos de
desarrollo de zlib. Con GCC, el soporte OpenMP utiliza `libgomp`. Python 3 es
necesario para los scripts de perfiles, medición y gráficos, y para compilar las pruebas.
En Debian/Ubuntu, los requisitos básicos se instalan así:

```bash
sudo apt-get update
sudo apt-get install build-essential cmake zlib1g-dev python3
```

Desde la raíz del repositorio:

```bash
cmake -S . -B build-openmp \
  -DCMAKE_BUILD_TYPE=Release \
  -DLPMD_ENABLE_OPENMP=ON \
  -DLPMD_ENABLE_TESTS=ON
cmake --build build-openmp --parallel 4
ctest --test-dir build-openmp --output-on-failure
```

La configuración debe informar `Found OpenMP_CXX`. Si el compilador no soporta
OpenMP, la opción `ON` produce un error de configuración. No hace falta añadir
`-fopenmp` manualmente: CMake proporciona las opciones de compilación y enlace.
OpenGL/GLUT son opcionales; este ejemplo funciona con la compilación sin gráficos.

`--parallel 4` usa hasta cuatro procesos **para compilar**. Para que la
**simulación** use cuatro hilos se establece `OMP_NUM_THREADS=4` al ejecutarla.
Son controles independientes. OpenMP paraleliza los cálculos de fuerzas del
núcleo; la integración, los analizadores y la escritura siguen teniendo
trabajo secuencial. Las directivas antiguas exclusivas de plugins no se activan
con esta opción.

Para comparar con una compilación completamente secuencial:

```bash
cmake -S . -B build-serial -DCMAKE_BUILD_TYPE=Release \
  -DLPMD_ENABLE_OPENMP=OFF
cmake --build build-serial --parallel 4
```

OpenMP está desactivado por defecto. Con el ejecutable compilado sin OpenMP,
`OMP_NUM_THREADS` no paraleliza el cálculo.

## Ejecutar primero la verificación corta

Estos comandos se inician en la raíz del repositorio. Conserva `repo` como ruta
absoluta porque las salidas se escriben en el directorio actual:

```bash
repo="$PWD"
mkdir -p "$repo/runs/shock-smoke"
cd "$repo/runs/shock-smoke"
OMP_NUM_THREADS=4 OMP_DYNAMIC=FALSE OMP_PROC_BIND=close OMP_PLACES=cores \
  OMP_DISPLAY_ENV=TRUE LPMD_PATH="$repo/build-openmp/lib" \
  "$repo/build-openmp/bin/lpmd" \
  "$repo/lpmd/examples/shock-wave/smoke.control" > run.log 2>&1
```

`run.log` debe mostrar `SIMULATION FINISHED` sin errores. El entorno OpenMP también
aparece en ese archivo: comprueba `OMP_NUM_THREADS` y `OMP_DYNAMIC`. Esta
verificación integra 100 pasos, con 1152 átomos, y produce `smoke.xyz`,
`smoke-monitor.dat` y `smoke-final.dump`. Es una comprobación de funcionamiento;
su duración física de 50 fs es demasiado corta para estudiar el frente desarrollado.

Cada ejecución debe usar un directorio propio: lpmd reemplaza sus archivos de
salida si se vuelve a lanzar en el mismo directorio.

## Ejecutar el ejemplo de choque

En la misma terminal, donde `repo` sigue definido:

```bash
mkdir -p "$repo/runs/shock-4threads"
cd "$repo/runs/shock-4threads"
OMP_NUM_THREADS=4 OMP_DYNAMIC=FALSE OMP_PROC_BIND=close OMP_PLACES=cores \
  LPMD_PATH="$repo/build-openmp/lib" \
  "$repo/build-openmp/bin/lpmd" \
  "$repo/lpmd/examples/shock-wave/shock.control" > run.log 2>&1
```

Usa inicialmente 1, 2 o 4 hilos y evita exceder los núcleos asignados al proceso.
En un equipo compartido o en un trabajo con planificador, cuenta los núcleos
disponibles para ese trabajo. No ejecutes varias comparaciones a la vez.

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

Una hora de ejecución es tiempo de reloj del computador. El tiempo físico del
sistema es `steps × dt`: aquí 2 ps. Cambiar el número de hilos mantiene ese
tiempo físico; aumentar `dt` cambia la resolución y exige verificar estabilidad
y convergencia, por lo que no debe usarse como atajo para acelerar el caso.

## Examinar la compresión

`shock.xyz` contiene 40 configuraciones con posición y velocidad de cada átomo:
`Ar x y z vx vy vz`, en Å y Å/fs. La primera corresponde al paso 100, no al paso
0. Se pueden exportar perfiles longitudinales sin instalar paquetes Python:

```bash
python3 "$repo/lpmd/examples/shock-wave/profiles.py" shock.xyz \
  --output profiles.csv
```

El CSV contiene tiempo, centro de cada franja, número de átomos, densidad
numérica en Å⁻³ y velocidad media longitudinal en m/s. Agrupa 24 franjas de
10.8 Å. Una franja vacía tiene densidad cero y velocidad media en blanco.
El destino CSV debe ser nuevo; el script evita reemplazar resultados anteriores.
Para generar las figuras de densidad relativa y velocidad media:

```bash
python3 "$repo/lpmd/examples/shock-wave/plot_results.py" profiles.csv
```

Se escribe `shock-profiles.png`, con curvas a 0.5, 1 y 2 ps. El graficador usa
Matplotlib sin abrir ventanas. Para agregar la figura de aceleración, pasa
`--times /ruta/al/benchmark/times.csv`; se genera también `shock-speedup.png`.
Usa `--output-dir figuras-nuevas` si ya existen figuras y deseas conservarlas.
Para analizar la verificación corta:

```bash
python3 "$repo/lpmd/examples/shock-wave/profiles.py" smoke.xyz \
  --cell 32.4 32.4 43.2 --bins 4 --each 20 --output smoke-profiles.csv
```

Grafica densidad y velocidad media frente a `z_angstrom` para varios valores de
`time_fs`. La densidad inicial es `4 / 5.4³ ≈ 0.02540 Å⁻³`. Busca su aumento
cerca del centro y la disminución de la velocidad media hacia cero en la zona
comprimida; sigue cómo esa zona crece hacia ambos lados. El cristal frío produce
oscilaciones de planos atómicos, por lo que conviene comparar varias franjas y
tiempos en lugar de interpretar una sola franja como una densidad continua.

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

## Medir si OpenMP ahorra tiempo

El script ejecuta la misma geometría y condición inicial con 1, 2 y 4 hilos,
repite cada medición tres veces y compara las medianas. Reduce el recorrido a
1000 pasos para evitar esperar la simulación completa. Python mide el tiempo
de reloj de todo el proceso; conserva los registros y `times.csv`:

```bash
cd "$repo/runs"
python3 "$repo/lpmd/examples/shock-wave/benchmark.py" \
  "$repo/build-openmp/bin/lpmd" \
  --plugin-path "$repo/build-openmp/lib" \
  --threads 1 2 4 --steps 1000 --repeats 3 --output benchmark-with-output
```

Para medir el cálculo con menos interferencia de la escritura de trayectorias:

```bash
python3 "$repo/lpmd/examples/shock-wave/benchmark.py" \
  "$repo/build-openmp/bin/lpmd" \
  --plugin-path "$repo/build-openmp/lib" \
  --threads 1 2 4 --steps 1000 --repeats 3 --no-trajectory \
  --output benchmark-without-trajectory
```

`--no-trajectory` elimina la misma salida XYZ de todos los controles temporales;
conserva el monitor y el estado final. El control original queda intacto.
Los directorios de medición deben ser nuevos. `benchmark.py` y `profiles.py`
usan únicamente la biblioteca estándar de Python; `plot_results.py` requiere
Matplotlib. La medición exige que el registro contenga el
entorno OpenMP solicitado; así detecta si se pasó una compilación secuencial.

La aceleración informada es `mediana_1_hilo / mediana_N_hilos`. Un valor menor
que 1 significa que esa configuración fue más lenta. La prueba corta sirve
para verificar funcionamiento; usa el sistema de 6912 átomos para medir.
Los resultados dependen del procesador, carga del equipo, memoria y salidas.
OpenMP no garantiza una reducción proporcional al número de hilos. También
conviene reducir la frecuencia de salidas y evitar visualización en vivo en
corridas largas, manteniendo las muestras necesarias para el análisis.

El cálculo paralelo acumula fuerzas y esfuerzos por hilo y luego los combina.
Esto evita bloqueos por cada par de átomos y escrituras simultáneas en el tensor
de esfuerzos. La suma puede cambiar sus últimos dígitos entre números de hilos;
la regresión de fuerzas comprueba equivalencia numérica con tolerancias.
Los buffers de aceleración requieren aproximadamente `24 × átomos × hilos`
bytes de memoria adicional.

### Medición de referencia

En un AMD Ryzen 5 5600U, GCC 14.2 y compilación `Release`, una comparación
de 200 pasos con 6912 átomos, tres repeticiones y `--no-trajectory` dio:

| Hilos solicitados | Mediana de tiempo de reloj | Aceleración frente a 1 hilo |
| --- | --- | --- |
| 1 | 18.057 s | 1.00× |
| 2 | 10.191 s | 1.77× |
| 4 | 6.537 s | 2.76× |

Es una referencia local, no un tiempo esperado para otros equipos ni para la
corrida completa. El ejemplo completo de 4000 pasos, con trayectoria XYZ,
terminó en aproximadamente 136 s con cuatro hilos en ese equipo. Se verificaron
sus 40 configuraciones y la conservación del número de átomos. La prueba corta
coincidió numéricamente entre las compilaciones secuencial y OpenMP con 1 y 4 hilos.
