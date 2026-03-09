#!/bin/bash
set -e

AGE=365
SIZE=0

echo "[INFO] BENCHMARKING DMAN..."
/usr/bin/time -l ./dman dummy-cache-A $AGE $SIZE > /dev/null 2> dman-mem.txt

echo "[INFO] BENCHMARKING FIND..."
/usr/bin/time -l find dummy-cache-B -type f -delete > /dev/null 2> find-mem.txt

echo "[RESULTS]"
echo "===  DMAN  ==="
cat dman-mem.txt

echo "===  FIND  ==="
cat find-mem.txt

