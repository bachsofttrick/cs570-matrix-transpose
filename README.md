# CS570 Matrix Transpose

Matrix transpose benchmarks comparing naive (non-blocking) vs cache-blocked implementations using pthreads.

## Files

| Source | Binary | Description |
|--------|--------|-------------|
| `transpose_csv.cc` | `transpose_csv` | Variable-size static arrays, CSV output |
| `transpose_malloc.cc` | `transpose_malloc` | Variable-size heap-allocated arrays |

## Build

```bash
make          # build all
make clean    # remove binaries
```

Or build individually:

```bash
g++ -lpthread -o transpose_csv transpose_csv.cc lib.cc
g++ -lpthread -o transpose_malloc transpose_malloc.cc lib.cc
```

## Run

```bash
./transpose_csv
./transpose_malloc
```
