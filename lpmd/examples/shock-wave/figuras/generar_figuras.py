#!/usr/bin/env python3
"""Create the class guide's conceptual diagrams and figures from real CSV data.

With no CSV files present, only the two conceptual diagrams are generated.
Default data files live beside this script: perfiles-referencia.csv and
tiempos-referencia.csv. They must be exported from actual example runs.
Timing bars use simulation_seconds, taken from LPMD's "Elapsed real time".
"""

import argparse
import csv
import math
import os
from pathlib import Path
import statistics
import tempfile

os.environ.setdefault("MPLCONFIGDIR", str(Path(tempfile.gettempdir()) / "lpmd2-matplotlib"))
import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import FancyArrowPatch, Rectangle


BLUE = "#174A73"
TEAL = "#28776B"
CORAL = "#C86F45"
PALE = "#EAF0F4"
REFERENCE_DENSITY = 4 / 5.4**3
HERE = Path(__file__).resolve().parent

plt.rcParams.update({
    "font.family": "DejaVu Sans",
    "font.size": 12,
    "axes.titlesize": 13,
    "axes.labelsize": 12,
    "xtick.labelsize": 11,
    "ytick.labelsize": 11,
    "figure.facecolor": "white",
    "axes.spines.top": False,
    "axes.spines.right": False,
    "savefig.facecolor": "white",
})


def save(figure, name):
    target = HERE / name
    figure.savefig(target, dpi=300, bbox_inches="tight", pad_inches=0.08)
    plt.close(figure)
    print(f"Figura: {target}")


def arrow(axis, start, end, color, width=2.6):
    axis.add_patch(FancyArrowPatch(start, end, arrowstyle="-|>",
                                  mutation_scale=17, linewidth=width, color=color))


def atoms(axis, positions, color):
    for y in (0.12, 0.30, 0.48):
        axis.scatter(positions, [y] * len(positions), s=23, color=color, zorder=3)


def draw_shock_scheme():
    figure, axes = plt.subplots(2, 1, figsize=(8, 3.7))
    figure.subplots_adjust(left=0.03, right=0.97, top=0.85, bottom=0.17, hspace=0.48)
    figure.suptitle("Cómo se produce la compresión", x=0.03, ha="left", y=0.99,
                   fontsize=15, fontweight="bold", color=BLUE)
    for axis in axes:
        axis.set(xlim=(-3, 103), ylim=(-0.34, 1.27))
        axis.axis("off")
        axis.plot([0, 0], [0.03, 0.77], "--", color="0.6", linewidth=1)
        axis.plot([100, 100], [0.03, 0.77], "--", color="0.6", linewidth=1)
    axes[0].text(0, 1.13, "1. Inicio: los átomos se acercan", color=BLUE,
                 fontsize=12, fontweight="bold")
    atoms(axes[0], [5 + 6 * i for i in range(8)], BLUE)
    atoms(axes[0], [53 + 6 * i for i in range(8)], TEAL)
    arrow(axes[0], (16, 0.94), (43, 0.94), BLUE)
    arrow(axes[0], (84, 0.94), (57, 0.94), TEAL)
    axes[0].text(27, 0.68, "+400 m/s", ha="center", fontsize=11,
                 color=BLUE, bbox={"facecolor": "white", "edgecolor": "none", "pad": 1})
    axes[0].text(73, 0.68, "−400 m/s", ha="center", fontsize=11,
                 color=TEAL, bbox={"facecolor": "white", "edgecolor": "none", "pad": 1})
    axes[0].text(50, -0.22, "Centro de la celda", ha="center", fontsize=11)

    axes[1].text(0, 1.13, "2. Después: la zona comprimida crece", color=BLUE,
                 fontsize=12, fontweight="bold")
    axes[1].add_patch(Rectangle((33, 0.02), 34, 0.70, facecolor=CORAL, alpha=0.12))
    atoms(axes[1], [8, 18, 28], BLUE)
    atoms(axes[1], [36 + 3.1 * i for i in range(10)], CORAL)
    atoms(axes[1], [72, 82, 92], TEAL)
    arrow(axes[1], (34, 0.87), (17, 0.87), CORAL)
    arrow(axes[1], (66, 0.87), (83, 0.87), CORAL)
    axes[1].text(50, 0.86, "Frentes", ha="center", color=CORAL, fontsize=11)
    axes[1].text(16, -0.24, "Descarga", ha="center", color=BLUE, fontsize=11)
    axes[1].text(50, -0.24, "Átomos más juntos", ha="center", color=CORAL, fontsize=11)
    axes[1].text(84, -0.24, "Descarga", ha="center", color=TEAL, fontsize=11)

    figure.text(0.03, 0.105, "0", fontsize=11)
    figure.text(0.955, 0.105, "L", fontsize=11)
    figure.add_artist(FancyArrowPatch((0.07, 0.112), (0.93, 0.112),
                                      transform=figure.transFigure,
                                      arrowstyle="<->", linestyle="--", color="0.55",
                                      linewidth=1.3, mutation_scale=12))
    figure.text(0.5, 0.05, "Extremos conectados por periodicidad: no hay paredes",
                ha="center", fontsize=11)
    figure.text(0.97, 0.99, "Esquema, no a escala", ha="right", va="top",
                fontsize=11, color="0.4")
    save(figure, "esquema-choque.png")


def draw_threads():
    figure, axis = plt.subplots(figsize=(8, 2.6))
    figure.subplots_adjust(left=0.02, right=0.99, top=0.83, bottom=0.20)
    axis.set(xlim=(0, 10.5), ylim=(-0.1, 5.3))
    axis.axis("off")
    figure.text(0.03, 0.96, "El mismo trabajo: 12 tareas iguales", color=BLUE,
                fontsize=15, fontweight="bold", va="top")
    colors = (BLUE, TEAL, CORAL, "#6F6791")

    def task(number, x, y):
        axis.add_patch(Rectangle((x, y), 0.49, 0.68,
                                  facecolor=colors[(number - 1) // 3],
                                  edgecolor="white", linewidth=0.8))
        axis.text(x + 0.245, y + 0.34, str(number), color="white", fontsize=11,
                  ha="center", va="center")

    axis.text(0, 4.55, "1 hilo", fontsize=13, fontweight="bold", va="center")
    axis.text(1.6, 4.55, "Hilo 1", fontsize=11, va="center")
    for number in range(1, 13):
        task(number, 3 + (number - 1) * 0.57, 4.2)
    axis.text(0, 1.65, "4 hilos", fontsize=13, fontweight="bold", va="center")
    for worker in range(4):
        y = 2.9 - worker * 0.88
        axis.text(1.6, y + 0.34, f"Hilo {worker + 1}", fontsize=11, va="center")
        for index in range(3):
            task(worker * 3 + index + 1, 3 + index * 0.57, y)
    axis.text(5.3, 1.9, "Trabajan al mismo tiempo\nen grupos más pequeños", fontsize=12,
              color=TEAL, va="center")
    figure.text(0.03, 0.08, "Reparto ideal. En el programa real también hay coordinación y escritura.",
                fontsize=11, color="0.3")
    save(figure, "hilos.png")


def read_csv(path, required):
    with path.open(newline="") as source:
        reader = csv.DictReader(source)
        if not required.issubset(reader.fieldnames or []):
            raise ValueError(f"{path}: faltan columnas necesarias")
        rows = list(reader)
    if not rows or any(None in row or any(value is None for value in row.values()) for row in rows):
        raise ValueError(f"{path}: CSV vacío o incompleto")
    return rows


def finite(value):
    number = float(value)
    if not math.isfinite(number):
        raise ValueError("Los datos de referencia deben ser finitos")
    return number


def draw_profiles(path):
    profiles = {}
    for row in read_csv(path, {"time_fs", "z_angstrom", "number_density_A^-3", "mean_vz_m_s"}):
        time_fs = finite(row["time_fs"])
        z = finite(row["z_angstrom"])
        density = finite(row["number_density_A^-3"]) / REFERENCE_DENSITY
        velocity = finite(row["mean_vz_m_s"]) if row["mean_vz_m_s"].strip() else math.nan
        profiles.setdefault(time_fs, []).append((z, density, velocity))
    expected = (500.0, 1000.0, 2000.0)
    if not all(time_fs in profiles for time_fs in expected):
        raise ValueError("Los perfiles deben incluir 0.5, 1 y 2 ps de una corrida completa")
    figure, axes = plt.subplots(2, 1, figsize=(8, 4.7), sharex=True)
    figure.subplots_adjust(left=0.13, right=0.98, top=0.88, bottom=0.13, hspace=0.15)
    figure.suptitle("Datos reales: acercamiento a la zona central", x=0.13, ha="left",
                   fontsize=14, fontweight="bold", color=BLUE)
    for time_fs, color in zip(expected, (BLUE, TEAL, CORAL)):
        values = sorted(profiles[time_fs])
        z, density, velocity = zip(*values)
        axes[0].plot(z, density, "o-", color=color, markersize=4,
                     label=f"{time_fs / 1000:g} ps", linewidth=1.6)
        axes[1].plot(z, velocity, "o-", color=color, markersize=4, linewidth=1.6)
    axes[0].axhline(1, color="0.6", linestyle="--", linewidth=1)
    axes[1].axhline(0, color="0.6", linestyle="--", linewidth=1)
    axes[0].set(ylabel=r"Densidad relativa $n/n_0$", ylim=(0.5, 1.8))
    axes[1].set(xlabel="Posición z (Å), vista central ampliada",
                ylabel="Velocidad media (m/s)", xlim=(70, 190), ylim=(-550, 550))
    axes[0].legend(loc="upper left", ncol=3, frameon=False, fontsize=11,
                    columnspacing=1.3, handlelength=1.5)
    last = sorted(profiles[2000.0])
    central = [entry for entry in last if 120 <= entry[0] <= 145]
    if not central:
        raise ValueError("No hay franjas centrales para anotar la figura")
    plateau = statistics.median(entry[1] for entry in central)
    density_target = min(central, key=lambda entry: abs(entry[1] - plateau))
    axes[0].annotate(f"Más átomos por volumen\n$n/n_0 \\approx {plateau:.2f}$",
                      xy=density_target[:2], xytext=(165, 1.53), ha="center", va="center",
                      fontsize=11, color=CORAL,
                      arrowprops={"arrowstyle": "->", "color": CORAL, "lw": 1.3},
                      bbox={"facecolor": "white", "edgecolor": "none", "pad": 2})
    flow_target = min((entry for entry in last if 115 <= entry[0] <= 150),
                      key=lambda entry: abs(entry[2]))
    axes[1].annotate("El flujo se frena\njunto al centro", xy=(flow_target[0], flow_target[2]),
                      xytext=(151, 350), ha="center", va="center", fontsize=11, color=CORAL,
                      arrowprops={"arrowstyle": "->", "color": CORAL, "lw": 1.3},
                      bbox={"facecolor": "white", "edgecolor": "none", "pad": 2})
    for axis in axes:
        axis.grid(alpha=0.17)
        axis.set_axisbelow(True)
    save(figure, "perfiles.png")


def draw_timings(path):
    samples = {}
    steps = set()
    for row in read_csv(path, {"requested_threads", "simulation_seconds", "steps"}):
        threads = int(row["requested_threads"])
        elapsed = finite(row["simulation_seconds"])
        if threads <= 0 or elapsed <= 0:
            raise ValueError("Hilos y tiempos deben ser positivos")
        samples.setdefault(threads, []).append(elapsed)
        steps.add(int(row["steps"]))
    if set(samples) != {1, 2, 4} or len(steps) != 1:
        raise ValueError("Se requieren tiempos con 1, 2 y 4 hilos y los mismos pasos")
    if any(len(values) != 3 for values in samples.values()):
        raise ValueError("Se requieren tres repeticiones para cada número de hilos")
    medians = {threads: statistics.median(values) for threads, values in samples.items()}
    figure, axis = plt.subplots(figsize=(7, 2.8))
    figure.subplots_adjust(left=0.12, right=0.98, top=0.78, bottom=0.25)
    order = (1, 2, 4)
    axis.barh(range(3), [medians[count] for count in order], height=0.55,
               color=(BLUE, TEAL, CORAL))
    axis.set(yticks=range(3), yticklabels=("1 hilo", "2 hilos", "4 hilos"),
              xlabel="Tiempo de reloj de la simulación (s)", xlim=(0, max(medians.values()) * 1.47))
    axis.invert_yaxis()
    axis.set_title(f"Datos reales: {next(iter(steps))} pasos, tres repeticiones", loc="left",
                     fontweight="bold", color=BLUE, pad=12)
    for index, count in enumerate(order):
        axis.text(medians[count] + medians[1] * 0.025, index,
                   f"{medians[count]:.2f} s  ·  {medians[1] / medians[count]:.2f}×",
                   fontsize=11, va="center", color="0.2")
    axis.grid(axis="x", alpha=0.16)
    axis.set_axisbelow(True)
    axis.spines["left"].set_visible(False)
    axis.tick_params(axis="y", length=0)
    figure.text(0.12, 0.045, '"Elapsed real time": fuerzas, integración y salidas; sin XYZ.',
                fontsize=10.5, color="0.35")
    save(figure, "tiempos.png")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--profiles", type=Path)
    parser.add_argument("--times", type=Path)
    args = parser.parse_args()
    try:
        draw_shock_scheme()
        draw_threads()
        for provided, default, function in (
            (args.profiles, "perfiles-referencia.csv", draw_profiles),
            (args.times, "tiempos-referencia.csv", draw_timings),
        ):
            path = provided or HERE / default
            if path.exists():
                function(path)
            elif provided:
                raise ValueError(f"No existe el archivo de datos: {path}")
            else:
                print(f"Pendiente de datos reales: {path.name}")
    except (OSError, ValueError, csv.Error) as error:
        parser.exit(1, f"Error: {error}\n")


if __name__ == "__main__":
    main()
