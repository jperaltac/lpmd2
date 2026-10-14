#!/usr/bin/env python3
"""Measure the same shock setup with different requested OpenMP thread counts."""

import argparse
import csv
import os
from pathlib import Path
import re
import statistics
import subprocess
import time


def positive_int(value):
    number = int(value)
    if number <= 0:
        raise argparse.ArgumentTypeError("must be a positive integer")
    return number


def benchmark(args):
    executable = args.executable.resolve(strict=True)
    control = args.control.read_text()
    control, replacements = re.subn(r"(?m)^steps\s+\d+\s*$", f"steps {args.steps}", control)
    if replacements != 1:
        raise ValueError("expected exactly one 'steps N' statement")
    if args.no_trajectory:
        control = re.sub(r"(?m)^output\s+[^\n]*\n?", "", control)
    # A fresh directory prevents accidental replacement of previous results.
    args.output.mkdir(parents=True, exist_ok=False)
    medians = {}
    with (args.output / "times.csv").open("x", newline="") as target:
        writer = csv.writer(target)
        writer.writerow(["requested_threads", "repeat", "steps", "wall_seconds"])
        for threads in args.threads:
            samples = []
            for repeat in range(1, args.repeats + 1):
                run_dir = args.output / f"threads-{threads}-run-{repeat}"
                run_dir.mkdir()
                run_control = run_dir / "shock.control"
                run_control.write_text(control)
                environment = os.environ.copy()
                environment.update(OMP_NUM_THREADS=str(threads), OMP_DYNAMIC="FALSE",
                                   OMP_PROC_BIND="close", OMP_PLACES="cores",
                                   OMP_DISPLAY_ENV="TRUE")
                if args.plugin_path:
                    environment["LPMD_PATH"] = str(args.plugin_path.resolve(strict=True))
                with (run_dir / "run.log").open("w") as log:
                    started = time.perf_counter()
                    result = subprocess.run([str(executable), str(run_control.resolve())],
                                            cwd=run_dir, env=environment, stdout=log,
                                            stderr=subprocess.STDOUT, check=False)
                    elapsed = time.perf_counter() - started
                log_text = (run_dir / "run.log").read_text()
                if result.returncode or "SIMULATION FINISHED" not in log_text:
                    raise ValueError(f"simulation failed; see {run_dir / 'run.log'}")
                if "_OPENMP" not in log_text or "OMP_NUM_THREADS" not in log_text:
                    raise ValueError("no OpenMP environment was reported; use an OpenMP build "
                                     f"and inspect {run_dir / 'run.log'}")
                samples.append(elapsed)
                writer.writerow([threads, repeat, args.steps, f"{elapsed:.6f}"])
                target.flush()
                print(f"{threads} thread(s), repeat {repeat}: {elapsed:.3f} s", flush=True)
            medians[threads] = statistics.median(samples)
    print("\nRequested threads | Median seconds | Speedup vs 1 thread")
    for threads in args.threads:
        print(f"{threads:17d} | {medians[threads]:14.3f} | "
              f"{medians[1] / medians[threads]:.2f}x")
    print(f"\nLogs and times.csv: {args.output.resolve()}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", type=Path, help="lpmd built with LPMD_ENABLE_OPENMP=ON")
    parser.add_argument("--control", type=Path,
                        default=Path(__file__).with_name("shock.control"))
    parser.add_argument("--plugin-path", type=Path, help="directory containing this build's plugins")
    parser.add_argument("--threads", type=positive_int, nargs="+", default=[1, 2, 4])
    parser.add_argument("--repeats", type=positive_int, default=3)
    parser.add_argument("--steps", type=positive_int, default=1000)
    parser.add_argument("--output", type=Path, default=Path("shock-benchmark"))
    parser.add_argument("--no-trajectory", action="store_true",
                        help="omit trajectory output in every run to measure compute scaling")
    args = parser.parse_args()
    if 1 not in args.threads or len(set(args.threads)) != len(args.threads):
        parser.error("--threads must include 1 and contain no duplicates")
    try:
        benchmark(args)
    except (OSError, ValueError) as error:
        parser.exit(1, f"Error: {error}\n")


if __name__ == "__main__":
    main()
