#!/usr/bin/env bash
set -euo pipefail

if [[ "${EUID}" -ne 0 ]]; then
  SUDO="sudo"
else
  SUDO=""
fi

if command -v apt-get >/dev/null 2>&1; then
  ${SUDO} apt-get update
  ${SUDO} apt-get install -y build-essential cmake freeglut3-dev
elif command -v dnf >/dev/null 2>&1; then
  ${SUDO} dnf install -y gcc gcc-c++ make cmake freeglut-devel
elif command -v pacman >/dev/null 2>&1; then
  ${SUDO} pacman -Sy --noconfirm base-devel cmake freeglut
else
  echo "Unsupported package manager. Please install build tools, CMake, and GLUT development headers manually." >&2
  exit 1
fi
