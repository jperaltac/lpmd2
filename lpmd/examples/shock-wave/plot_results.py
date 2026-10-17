#!/usr/bin/env python3
"""Plot the shock profiles and optional OpenMP timings using matplotlib."""

import argparse
import csv
import math
from pathlib import Path
import statistics


REFERENCE_DENSITY = 4 / 5.4**3  # FCC number density in atoms / angstrom^3.


def read_rows(path, required):
    with path.open(newline="") as source:
        reader = csv.DictReader(source)
        if not required.issubset(reader.fieldnames or []):
            raise ValueError(f"{path}: missing CSV columns: {', '.join(sorted(required))}")
        rows = list(reader)
    if not rows:
        raise ValueError(f"{path}: empty CSV")
    if any(None in row or any(value is None for value in row.values()) for row in rows):
        raise ValueError(f"{path}: incomplete CSV row or unexpected extra fields")
    return rows


def finite(value):
    number = float(value)
    if not math.isfinite(number):
        raise ValueError("CSV values must be finite")
    return number


def read_profiles(path):
    required = {"time_fs", "z_angstrom", "number_density_A^-3", "mean_vz_m_s"}
    profiles = {}
    for row in read_rows(path, required):
        time_fs = finite(row["time_fs"])
        z = finite(row["z_angstrom"])
        density = finite(row["number_density_A^-3"])
        velocity = finite(row["mean_vz_m_s"]) if row["mean_vz_m_s"].strip() else math.nan
        if time_fs < 0 or density < 0:
            raise ValueError("profile times and densities must be nonnegative")
        if density > 0 and math.isnan(velocity):
            raise ValueError("a nonempty spatial bin must have a mean velocity")
        profiles.setdefault(time_fs, []).append((z, density / REFERENCE_DENSITY, velocity))
    return {time_fs: sorted(values) for time_fs, values in profiles.items()}


def select_times(profiles):
    times = sorted(profiles)
    selected = {min(times, key=lambda time_fs: abs(time_fs - target))
                for target in (500, 1000, 2000) if times[0] <= target <= times[-1]}
    # Short smoke runs never reach 0.5 ps; show their first, middle and last frames.
    return sorted(selected or {times[0], times[len(times) // 2], times[-1]})


def read_timings(path):
    samples = {}
    steps = set()
    required = {"requested_threads", "repeat", "steps", "wall_seconds"}
    for row in read_rows(path, required):
        threads, repeat, run_steps = (int(row[key]) for key in ("requested_threads", "repeat", "steps"))
        seconds = finite(row["wall_seconds"])
        if min(threads, repeat, run_steps) <= 0 or seconds <= 0:
            raise ValueError("thread counts, repeats, steps and elapsed times must be positive")
        steps.add(run_steps)
        samples.setdefault(threads, []).append(seconds)
    if 1 not in samples or len(steps) != 1:
        raise ValueError("timings must include one thread and use the same number of steps")
    return {threads: statistics.median(values) for threads, values in samples.items()}


def plot_profiles(plt, profiles, target):
    figure, axes = plt.subplots(2, 1, figsize=(8, 6.5), sharex=True, constrained_layout=True)
    for time_fs in select_times(profiles):
        z, density, velocity = zip(*profiles[time_fs])
        label = f"{time_fs / 1000:g} ps"
        axes[0].plot(z, density, ".-", label=label)
        axes[1].plot(z, velocity, ".-", label=label)
    axes[0].axhline(1, color="0.5", linestyle="--", linewidth=1)
    axes[1].axhline(0, color="0.5", linestyle="--", linewidth=1)
    axes[0].set(ylabel=r"Densidad relativa $n/n_0$", ylim=(0, None),
                title=r"Perfiles longitudinales; $n_0=4/(5.4\ \mathrm{\AA})^3$")
    axes[1].set(xlabel=r"$z$ (Å)", ylabel=r"Velocidad media $\langle v_z\rangle$ (m/s)")
    axes[0].legend(title="Tiempo físico", ncol=3)
    for axis in axes:
        axis.grid(alpha=0.25)
    figure.savefig(target, dpi=200)
    plt.close(figure)


def plot_speedup(plt, medians, target):
    threads = sorted(medians)
    speedup = [medians[1] / medians[count] for count in threads]
    figure, axis = plt.subplots(figsize=(6.5, 4.5), constrained_layout=True)
    axis.plot(threads, speedup, "o-", label="Medido con medianas")
    axis.plot(threads, threads, "--", color="0.5", label=r"Ideal: $S_p=p$")
    axis.set(xlabel=r"Hilos OpenMP solicitados, $p$", ylabel=r"Aceleración $S_p=T_1/T_p$",
             title="Tiempo de reloj del proceso", xticks=threads, ylim=(0, None))
    axis.legend()
    axis.grid(alpha=0.25)
    figure.savefig(target, dpi=200)
    plt.close(figure)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("profiles", type=Path, help="CSV exported by profiles.py")
    parser.add_argument("--times", type=Path, help="times.csv produced by benchmark.py")
    parser.add_argument("--output-dir", type=Path, default=Path.cwd(),
                        help="directory for new PNG files; existing figures are never replaced")
    args = parser.parse_args()
    try:
        profiles = read_profiles(args.profiles)
        medians = read_timings(args.times) if args.times else None
        targets = [args.output_dir / "shock-profiles.png"]
        if medians:
            targets.append(args.output_dir / "shock-speedup.png")
        for target in targets:
            if target.exists():
                raise ValueError(f"{target} already exists; choose a new --output-dir")
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        args.output_dir.mkdir(parents=True, exist_ok=True)
        plot_profiles(plt, profiles, targets[0])
        if medians:
            plot_speedup(plt, medians, targets[1])
        for target in targets:
            print(f"Saved {target}")
    except ImportError:
        parser.exit(1, "Error: install matplotlib before running this script.\n")
    except (OSError, ValueError, csv.Error) as error:
        parser.exit(1, f"Error: {error}\n")


if __name__ == "__main__":
    main()
