# Stack vs heap memory for cache coherence

## User

tell me about stack vs heap memory. when I do:
void blocking(double** output, double** input, int n, int b, double* result) {
  double time0 = get_seconds();

  for(int trials=0;trials<T;trials++) {
    for(int m=0;m<n;m+=b) {
      for(int k=0;k<n;k+=b) {
        for(int i=m;i<min(m+b, n);i++) {
          for(int j=k;j<min(k+b, n);j++) {
            output[j][i]=input[i][j];
          }
        }
      }
    }
  }

  *result = get_seconds() - time0;
}
stack: non-blocking takes more time then blocking 
heap: non-blocking takes less time then blocking.
It should be non-blocking takes more time then blocking because of blocking reduce cache miss

## Claude

## Stack vs Heap Memory

**Stack memory** is allocated in a contiguous, fixed region — typically small (a few MB). **Heap memory** is dynamically allocated and can be much larger, but is scattered across physical memory.

---

## Why Your Results Differ

### Heap Case (expected behavior ✓)
When `input`/`output` are `double**` (pointer-to-pointer, heap allocated), each row is a **separate heap allocation** scattered in memory:

```
input[0] → [somewhere in memory]
input[1] → [somewhere else entirely]
input[2] → [yet another location]
```

- **Non-blocking** transpose jumps between rows randomly → **tons of cache misses** → slow
- **Blocking** keeps access within a tile that fits in cache → **fewer cache misses** → fast
- ✅ Blocking wins, as expected

---

### Stack Case (surprising behavior ✗)
On the stack, a 2D array like `double input[N][N]` is **one contiguous block**:

```
[row0col0][row0col1]...[row0colN][row1col0][row1col1]...
```

The key insight is **prefetching**. Modern CPUs detect sequential access patterns and automatically prefetch cache lines ahead of time. With a contiguous stack array:

- **Non-blocking**: column access strides are predictable and regular — the CPU prefetcher can handle it
- **Blocking**: introduces loop overhead, more branch checks (`min(m+b, n)`), and **disturbs the prefetcher's simple stride pattern**

The blocking overhead **outweighs cache benefits** when the data is already contiguous and the prefetcher works well.

---

## Summary Table

| | Stack (contiguous) | Heap (scattered rows) |
|---|---|---|
| Memory layout | One flat block | Rows at random addresses |
| Prefetcher behavior | Works well on strides | Can't predict jumps |
| Non-blocking speed | Fast (prefetcher helps) | Slow (cache misses) |
| Blocking speed | Slower (overhead > benefit) | Fast (tiles fit in cache) |

---

## The Core Lesson

Blocking helps most when memory is **non-contiguous or large enough to thrash the cache**. When data is contiguous (stack), the hardware prefetcher often compensates — and blocking's loop overhead can actually **hurt** performance instead.

## User

this is stack double input[MAXN][MAXN];
double output[MAXN][MAXN];
double inputB[MAXN][MAXN];
double outputB[MAXN][MAXN];

this is heap: double** input   = alloc_matrix(MAXN);
  double** inputB  = alloc_matrix(MAXN);
double** output  = alloc_matrix(MAXN);
double** outputB = alloc_matrix(MAXN);

implementation of double** alloc_matrix(int n) {
  double** m = new double*[n];
  for(int i=0;i<n;i++) m[i] = new double[n];
  return m;
}

## Claude

## Now the Behavior Makes Complete Sense

### Stack Layout: `double input[MAXN][MAXN]`
Declared as **global/local static 2D arrays** — truly one flat contiguous block:

```
input[0][0], input[0][1], ..., input[0][MAXN-1], input[1][0], input[1][1], ...
└─────────────────── all contiguous in memory ───────────────────────────────┘
```

All 4 arrays (`input`, `output`, `inputB`, `outputB`) likely end up **adjacent in the data segment**, so the CPU prefetcher sees very regular, predictable stride patterns across the whole working set.

---

### Heap Layout: `alloc_matrix` with `new double*[n]` + loop
This does **n+1 separate allocations**:

```
m        → [ptr0, ptr1, ptr2, ...]   ← one allocation
m[0]     → [d, d, d, d, d, d]       ← separate allocation, somewhere in heap
m[1]     → [d, d, d, d, d, d]       ← another allocation, elsewhere
m[2]     → [d, d, d, d, d, d]       ← another allocation, elsewhere
```

With **4 matrices** each doing `n` separate `new double[n]` calls, you get `4n` scattered heap chunks. Row pointers are **not guaranteed to be adjacent**.

---

### Why This Explains Your Results

| Scenario | Memory | `input[i][j]` access | Cache behavior |
|---|---|---|---|
| Stack non-blocking | Flat 2D array | `*(base + i*MAXN + j)` — pure stride | Prefetcher handles it well |
| Stack blocking | Flat 2D array | Same, but with tile loop overhead | Overhead > benefit |
| **Heap non-blocking** | Scattered rows | Dereference `m[i]` ptr, then `[j]` | Row jumps = cache misses |
| **Heap blocking** | Scattered rows | Tile stays within a few rows | Misses reduced significantly |

---

### The Critical Difference: Pointer Indirection

```cpp
// Stack — direct address calculation, no indirection
output[j][i] = input[i][j];
// compiles to: *(output + j*MAXN + i) = *(input + i*MAXN + j)

// Heap — TWO memory lookups per access
output[j][i] = input[i][j];
// compiles to: *(*(output+j) + i) = *(*(input+i) + j)
//              ^first fetch ptr   ^then fetch data
```

On heap, accessing `output[j]` in the inner loop (j changes fast) means **fetching a different row pointer every iteration**, each pointing to a different memory region. This is what makes cache misses so severe and why blocking helps so much on heap but not on stack.

## User

how come heap allocation is scattered? What are the ways to allocate contiguous block?

## Claude

## Why Heap Allocations Are Scattered

Each `new` call asks the **heap allocator** (malloc under the hood) for a chunk of memory. The allocator manages a free list of available blocks and hands you whatever fits:

```
After alloc_matrix(4):

Heap memory (simplified):
[ADDRESS]  [CONTENT]
0x1000     m[0] → points to 0x3500    ← row 0 data
0x1200     (other program data)
0x1400     (freed chunk from earlier)
0x1600     m[2] → points to 0x4100    ← row 2 data
0x1800     (other allocation)
0x2000     m[1] → points to 0x2200    ← row 1 data
0x3500     [row 0: d,d,d,d,d,d,d,d]
0x4100     [row 2: d,d,d,d,d,d,d,d]
0x2200     [row 1: d,d,d,d,d,d,d,d]
```

Each `new double[n]` call is **independent** — the allocator doesn't know you want them together. Between calls, other allocations (from the OS, runtime, other threads) can grab memory in between.

---

## Ways to Allocate a Contiguous Block on the Heap

### 1. Single flat 1D array (manual indexing)
```cpp
double* matrix = new double[n * n];

// Access as matrix[i][j]:
matrix[i * n + j] = value;
```
✅ One allocation, perfectly contiguous
✅ Best cache performance
❌ Loses `[][]` syntax

---

### 2. Single allocation + pointer array (best of both worlds)
```cpp
double* data = new double[n * n];   // one contiguous block
double** m   = new double*[n];      // row pointer array

for (int i = 0; i < n; i++)
    m[i] = data + i * n;            // point into the block

// Now m[i][j] works AND memory is contiguous
m[2][3] = value;  // ✅ same as data[2*n + 3]
```
✅ Contiguous data
✅ Keeps `[][]` syntax
✅ Only 2 allocations total

```
data: [row0|row1|row2|row3]   ← one flat block
       ↑    ↑    ↑    ↑
m[0]──┘ m[1]┘ m[2]┘ m[3]┘   ← pointers into the block
```

---

### 3. `std::vector` of flat storage
```cpp
std::vector<double> matrix(n * n);

// Access:
matrix[i * n + j] = value;
```
✅ RAII, no manual delete
✅ Contiguous (guaranteed by standard)
❌ Still manual index math

---

### 4. Stack allocation (what you already have)
```cpp
double input[MAXN][MAXN];   // contiguous, but limited by stack size
```
✅ Fastest, no allocator overhead
❌ Stack is typically only ~1–8MB — large matrices will **stack overflow**
❌ Size must be compile-time constant

---

## Comparison

| Method | Contiguous | `[][]` syntax | Dynamic size | Safe |
|---|---|---|---|---|
| `new` in a loop | ❌ | ✅ | ✅ | manual delete |
| Flat `new double[n*n]` | ✅ | ❌ | ✅ | manual delete |
| Flat alloc + ptr array | ✅ | ✅ | ✅ | manual delete |
| `std::vector` | ✅ | ❌ | ✅ | ✅ automatic |
| Stack array | ✅ | ✅ | ❌ | ✅ automatic |

For performance-critical code like your cache blocking experiment, **option 2** (flat alloc + pointer array) is the most common fix — it gives you contiguous memory while keeping the same `m[i][j]` access pattern your existing code already uses.

## User

Can Single allocation + pointer array be applied for 3D matrix?

## Claude

Yes! Here's how it extends to 3D:

## 3D: Single Allocation + Pointer Array

For a 3D matrix `m[x][y][z]` of size `n×n×n`:

```cpp
double*   data = new double[n * n * n];   // one contiguous block
double**  rows = new double*[n * n];      // y*z pointers → into data
double*** m    = new double**[n];         // x pointers → into rows

for (int i = 0; i < n; i++) {
    m[i] = rows + i * n;                  // each m[i] points to n row-ptrs
    for (int j = 0; j < n; j++) {
        m[i][j] = data + i * n * n + j * n;  // each m[i][j] points to n doubles
    }
}

// Access naturally:
m[x][y][z] = value;  // ✅
```

### Memory Layout

```
data (flat):
[x=0,y=0 | x=0,y=1 | x=0,y=2 | x=1,y=0 | x=1,y=1 | ...]
 z=0,1,2    z=0,1,2    z=0,1,2

rows:
[ptr to x=0,y=0 | ptr to x=0,y=1 | ptr to x=1,y=0 | ...]
  ↑                 ↑
  into data          into data

m:
[ptr to rows[0] | ptr to rows[n] | ptr to rows[2n] | ...]
  ↑ (x=0 slice)    ↑ (x=1 slice)
  into rows          into rows
```

### Only 3 allocations total
```
new double[n³]      → actual data, contiguous
new double*[n²]     → row pointers (y layer)
new double**[n]     → slice pointers (x layer)
```

### Cleanup (must free in reverse order)
```cpp
delete[] data;
delete[] rows;
delete[] m;
```

---

## General Pattern for N dimensions

Each dimension adds **one pointer array** and **one level of indirection**:

| Dimension | Allocations | Access |
|---|---|---|
| 1D | `data` | `m[x]` |
| 2D | `data` + `double*[]` | `m[x][y]` |
| 3D | `data` + `double*[]` + `double**[]` | `m[x][y][z]` |
| ND | `data` + N-1 pointer arrays | `m[x][y][z]...` |

The **data block is always one flat allocation** — the pointer arrays are just a navigation structure on top of it. So cache performance stays good for the actual data accesses.

