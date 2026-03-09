#!/bin/bash
set -e

mkdir -p dummy-cache-A/subdir/deeper dummy-cache-B/subdir/deeper

for i in {1..250}; do
    dd if=/dev/urandom of=dummy-cache-A/file_$i.tmp bs=1M count=1 status=none
    dd if=/dev/urandom of=dummy-cache-B/file_$i.tmp bs=1M count=1 status=none
done

dd if=/dev/urandom of=dummy-cache-A/subdir/deeper/test.tmp bs=5M count=1 status=none
dd if=/dev/urandom of=dummy-cache-B/subdir/deeper/test.tmp bs=5M count=1 status=none

echo "[INFO] DIRECTORY dummy-cache-A GENERATED AND POPULATED"
echo "[INFO] DIRECTORY dummy-cache-B GENERATED AND POPULATED"
echo "[DONE] EXITING..."
