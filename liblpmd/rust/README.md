# Rust, `csta` y `lpmd_stats`

Este directorio contiene un experimento de integración progresiva entre LPMD y
Rust. En el historial del repositorio aparece como “csta”; si llegaste aquí
buscando “cesta”, probablemente se trata del mismo punto: la dependencia de
Rust se llama [`csta`](https://crates.io/crates/csta), no `cesta`.

## Resumen corto

- **Qué hace `csta`:** aporta primitivas estadísticas en Rust. En este
  repositorio se usa su tipo `Hist` para construir un histograma y calcular
  media y varianza.
- **Qué añade LPMD encima:** el crate local `lpmd_stats` expone una función C
  ABI llamada `lpmd_histogram_summary` para que, en el futuro, código C o C++
  pueda pedir ese resumen estadístico sin conocer Rust.
- **Estado actual:** el crate Rust compila y se prueba de forma independiente.
  Además, CMake puede enlazarlo de forma experimental con el plugin
  `minimumimage` cuando se configura `-DLPMD_ENABLE_RUST_HOTSPOTS=ON`; el flujo
  normal lo mantiene desactivado por defecto.

## ¿Qué problema intenta resolver?

La base de LPMD sigue estando en C++. El documento de rendimiento del
repositorio identifica bucles numéricos y cálculos puros como candidatos a una
migración parcial a Rust porque se pueden aislar tras una interfaz C y probar en
bloques pequeños.

`lpmd_stats` es el primer bloque de ese estilo: en vez de reescribir una parte
grande del simulador, encapsula una operación acotada —crear un histograma sobre
un arreglo de `double`/`f64` y devolver estadísticas— detrás de una única
función exportada.

## ¿Dónde aparece `csta`?

La dependencia está declarada en `Cargo.toml`:

```toml
[dependencies]
csta = "1.0.1"
```

El código Rust la importa así:

```rust
use csta::hist::Hist;
```

La función exportada crea el histograma con `Hist::with_buckets`, agrega cada
muestra con `add_clamped` y finalmente obtiene media y varianza con
`average_variange`:

```rust
let mut hist = Hist::with_buckets(range_min, range_max, buckets);
for value in samples {
    hist.add_clamped(value);
}

let (average, variance) = hist.average_variange();
```

> Nota: el nombre `average_variange` viene de la API de `csta` usada aquí; no es
> un nombre definido por LPMD.

## API que LPMD expone alrededor de `csta`

La superficie pública actual es esta función:

```rust
#[no_mangle]
pub unsafe extern "C" fn lpmd_histogram_summary(
    data: *const f64,
    len: usize,
    range_min: f64,
    range_max: f64,
    buckets: usize,
    out_average: *mut f64,
    out_variance: *mut f64,
) -> i32
```

### Parámetros

- `data`: puntero al primer elemento del arreglo de muestras (`double` en C/C++;
  `f64` en Rust).
- `len`: cantidad de elementos accesibles desde `data`.
- `range_min` / `range_max`: límites del histograma. Deben ser finitos y
  `range_min < range_max`.
- `buckets`: número de cubetas del histograma. Debe ser mayor que cero.
- `out_average`: puntero opcional donde escribir la media. Puede ser `NULL` si
  no se necesita.
- `out_variance`: puntero opcional donde escribir la varianza. Puede ser `NULL`
  si no se necesita.

### Códigos de retorno

- `0`: éxito. Se escriben la media y/o varianza cuando sus punteros no son
  nulos.
- `-1`: argumentos inválidos: `data == NULL`, `len == 0`, rango no finito,
  rango invertido o sin ancho, o `buckets == 0`.
- `-2`: alguna muestra no es finita (`NaN`, `+inf` o `-inf`).

### Comportamiento importante

- Las muestras fuera del rango solicitado se agregan con `add_clamped`, es decir,
  se sujetan a la cubeta de borde más cercana en vez de descartarse.
- La función es `unsafe` porque acepta punteros crudos desde C/C++; el llamador
  debe garantizar que `data` apunta a `len` valores válidos y que los punteros de
  salida, si no son nulos, se pueden escribir.
- La función no reserva ni libera memoria para el llamador; solo escribe valores
  escalares en los punteros de salida.

## Hotspot experimental: lista de vecinos ortogonal

El crate también expone `lpmd_build_neighbor_list_orthogonal`, una función C ABI
para construir, por lotes, la lista de vecinos de un átomo en una celda
ortogonal usando buffers planos:

- entrada: posiciones `x, y, z` contiguas, longitudes de celda, periodicidad,
  índice central, `cutoff` y modo `full`;
- salida: índices vecinos, vectores `rij`, distancias cuadradas `r2` y contador
  de resultados;
- ownership: C/C++ reserva y libera todos los buffers; Rust solo valida y
  escribe resultados.

Esta ruta se usa desde `plugins/minimumimage.cc` solo cuando el build activa
`LPMD_ENABLE_RUST_HOTSPOTS`. Si Rust rechaza los datos o la celda no es
ortogonal, el plugin cae al camino C++ existente.

Para compilar la PoC desde la raíz del repositorio:

```bash
cmake -S . -B build-rust-hotspots -DCMAKE_BUILD_TYPE=Release -DLPMD_ENABLE_RUST_HOTSPOTS=ON
cmake --build build-rust-hotspots --target plugin_minimumimage
```

## Cómo se compila y se prueba hoy

Desde este directorio:

```bash
cd liblpmd/rust
cargo test
```

El crate se configura como:

```toml
crate-type = ["cdylib", "rlib"]
```

Esto permite dos usos:

- `rlib`: pruebas unitarias Rust con `cargo test`.
- `cdylib`: generación de una biblioteca dinámica exportando la función C ABI
  para un futuro enlace desde C/C++.

Para construir solamente la biblioteca dinámica:

```bash
cd liblpmd/rust
cargo build --release
```

El artefacto esperado queda bajo `liblpmd/rust/target/release/`, por ejemplo
`liblpmd_stats.so` en Linux.

## Cómo se incorporaría desde C/C++

La función de histograma puede llamarse desde C o C++ mediante una declaración
compatible:

```cpp
#include <cstddef>

extern "C" int lpmd_histogram_summary(
    const double* data,
    std::size_t len,
    double range_min,
    double range_max,
    std::size_t buckets,
    double* out_average,
    double* out_variance);
```

Ejemplo de llamada:

```cpp
double samples[] = {0.5, 1.5, 2.5, 3.5, 4.5};
double average = 0.0;
double variance = 0.0;

int status = lpmd_histogram_summary(
    samples,
    5,
    0.0,
    5.0,
    5,
    &average,
    &variance);

if (status != 0) {
    // Manejar -1 o -2.
}
```

Para integrarlo realmente en el build C++ habría que añadir, como mínimo:

1. Un paso de CMake que invoque `cargo build` para `liblpmd/rust`.
2. Un target importado o una ruta explícita al `cdylib` generado por Cargo.
3. El enlace de ese target con la parte C++ que vaya a consumir la función.
4. Una cabecera C/C++ estable para declarar `lpmd_histogram_summary`.
5. Una llamada desde un punto concreto de LPMD: por ejemplo un analizador, una
   propiedad estadística o un plugin que actualmente construya histogramas.

## Cómo encaja con el proceso normal de LPMD

Actualmente, el proceso normal es:

1. CMake compila `liblpmd`, los ejecutables y los plugins C++.
2. Los ejecutables leen archivos `.control`.
3. El `PluginManager` localiza plugins en `LPMD_PATH` o en la ruta de plugins
   generada por CMake.
4. Las simulaciones, conversiones y análisis se ejecutan usando esos módulos
   C++.

`csta` **no participa todavía** en ese flujo. No hay ningún archivo `.control`
que active `csta` directamente y no existe un módulo de plugin llamado `csta` o
`cesta`. La integración actual es preparatoria: deja disponible un componente
Rust probado y con ABI C para incorporarlo después en una parte específica del
flujo.

## Qué significaría “usar `csta`” en este repositorio

Hoy hay dos significados posibles:

1. **Uso directo para desarrollo:** entrar a `liblpmd/rust` y ejecutar las
   pruebas o compilar `lpmd_stats` con Cargo. Esto valida que la dependencia
   `csta` funciona y que la función FFI devuelve los códigos esperados.
2. **Uso dentro de LPMD:** todavía pendiente. Requiere conectar el `cdylib` de
   Rust con CMake y reemplazar o complementar una rutina C++ concreta con una
   llamada a `lpmd_histogram_summary`.

Por eso, si estás ejecutando `cmake --build build` desde la raíz y luego corres
`build/bin/lpmd`, no estás usando `csta` aún.

## Pruebas existentes

Las pruebas unitarias cubren:

- cálculo básico de media y varianza;
- rechazo de punteros nulos y entradas vacías;
- rechazo de muestras no finitas;
- aceptación de punteros de salida nulos;
- rechazo de rangos inválidos y cantidad de cubetas cero.

Estas pruebas viven en `src/lib.rs` junto a la función porque, por ahora, el
crate Rust está aislado del resto del proyecto.

## Próximos pasos recomendados

Si se quiere que `csta` forme parte real del proceso de LPMD, el siguiente paso
no debería ser añadir más código Rust aislado, sino escoger un consumidor C++
concreto. Buenos candidatos son plugins o analizadores que ya calculen
histogramas o distribuciones, por ejemplo cálculos de perfiles, distribución de
velocidades o funciones de distribución radial.

Una vez elegido el consumidor, conviene añadir:

- una cabecera `liblpmd` para la ABI;
- integración CMake/Cargo reproducible;
- un test C++ que llame a la función Rust cuando el componente esté disponible;
- documentación del archivo `.control` o plugin que active la ruta nueva.
