#!/bin/bash
set -e

mkdir -p dummy-cache-A/subdir/deeper dummy-cache-B/subdir/deeper

for i in {1..100}; do
    dd if=/dev/urandom of=dummy-cache-A/file_$i.tmp bs=1M count=1 status=none
    dd if=/dev/urandom of=dummy-cache-B/file_$i.tmp bs=1M count=1 status=none
done

dd if=/dev/urandom of=dummy-cache-A/subdir/deeper/test.tmp bs=5M count=1 status=none
dd if=/dev/urandom of=dummy-cache-B/subdir/deeper/test.tmp bs=5M count=1 status=none

echo "[EXIT] DUMMY CACHE DIRECTORIES GENERATED. EXITING..."
