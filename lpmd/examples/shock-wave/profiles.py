#!/usr/bin/env python3
"""Export longitudinal density and mean velocity from the example's XYZ file."""

import argparse
import csv
import math
from pathlib import Path


def positive_float(value):
    number = float(value)
    if not math.isfinite(number) or number <= 0:
        raise argparse.ArgumentTypeError("must be a finite positive number")
    return number


def positive_int(value):
    number = int(value)
    if number <= 0:
        raise argparse.ArgumentTypeError("must be a positive integer")
    return number


def export_profiles(args):
    if args.trajectory.resolve() == args.output.resolve():
        raise ValueError("the output must differ from the trajectory")
    width = args.cell[2] / args.bins
    volume = args.cell[0] * args.cell[1] * width
    frames = 0
    with args.trajectory.open() as source, args.output.open("x", newline="") as target:
        writer = csv.writer(target)
        writer.writerow(["frame", "step", "time_fs", "z_angstrom", "atoms",
                         "number_density_A^-3", "mean_vz_m_s"])
        while True:
            line = source.readline()
            if not line:
                break
            natoms = int(line.strip())
            if natoms <= 0 or not source.readline():
                raise ValueError("invalid XYZ frame header")
            counts = [0] * args.bins
            velocity_sums = [0.0] * args.bins
            for atom in range(natoms):
                fields = source.readline().split()
                if len(fields) != 7:
                    raise ValueError(f"frame {frames + 1}, atom {atom + 1}: expected "
                                     "SYM X Y Z VX VY VZ (XYZ level=1)")
                values = [float(value) for value in fields[1:]]
                if not all(math.isfinite(value) for value in values):
                    raise ValueError("non-finite coordinate or velocity")
                # z is periodic in this example; the same convention handles z=Lz.
                index = min(int((values[2] % args.cell[2]) / width), args.bins - 1)
                counts[index] += 1
                velocity_sums[index] += values[5]
            frames += 1
            # lpmd first saves at step=each; it does not save the initial frame.
            step = frames * args.each
            for index, count in enumerate(counts):
                mean_vz = velocity_sums[index] / count * 1e5 if count else ""
                writer.writerow([frames, step, step * args.dt, (index + 0.5) * width,
                                 count, count / volume, mean_vz])
        if not frames:
            raise ValueError("empty trajectory")
    print(f"Exported {frames} frames to {args.output}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("trajectory", type=Path)
    parser.add_argument("--output", type=Path, default=Path("profiles.csv"))
    parser.add_argument("--cell", type=positive_float, nargs=3,
                        default=[32.4, 32.4, 259.2], metavar=("LX", "LY", "LZ"))
    parser.add_argument("--bins", type=positive_int, default=24)
    parser.add_argument("--dt", type=positive_float, default=0.5, help="time step in fs")
    parser.add_argument("--each", type=positive_int, default=100,
                        help="trajectory output interval, starting at step=each")
    args = parser.parse_args()
    try:
        export_profiles(args)
    except (OSError, ValueError) as error:
        parser.exit(1, f"Error: {error}\n")


if __name__ == "__main__":
    main()
