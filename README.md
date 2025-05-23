# CPU Cache Benchmark Tool

A comprehensive C-based benchmarking suite designed to analyze and measure CPU cache performance across different cache levels (L1, L2, L3) and memory access patterns. Originally optimized for AMD Ryzen processors but works on any x86-64 system.

## Features

### 🎯 Cache Hierarchy Analysis
- **L1 Data Cache Testing**: Measures performance within L1 cache boundaries
- **L2 Cache Performance**: Tests intermediate cache level behavior  
- **L3 Cache Investigation**: Detailed analysis of shared cache performance
- **Memory Latency Profiling**: Shows performance transitions between cache levels

### 🔬 Multiple Access Pattern Tests
- **Sequential Access**: Cache-friendly linear memory traversal
- **Random Access**: Worst-case cache performance scenarios
- **Stride Testing**: Cache line efficiency analysis (64-byte boundaries)
- **Read/Write Comparison**: Performance differences between operations

### 📊 Advanced Cache Analysis
- **Associativity Testing**: Demonstrates cache thrashing effects
- **Cache Line Optimization**: Shows impact of memory alignment
- **Bandwidth Measurements**: Memory throughput at different working set sizes
- **Fine-grained L3 Analysis**: Detailed investigation of cache boundaries

## System Requirements

### Hardware
- **CPU**: x86-64 processor with `rdtsc` support
- **Memory**: At least 256MB RAM (128MB for largest test)
- **Architecture**: Intel or AMD 64-bit processors

### Software
- **Compiler**: GCC 4.8+ or Clang 3.5+
- **OS**: Linux, macOS, or Windows (with MinGW/MSYS2)
- **C Standard**: C99 or later

## Compilation Instructions

### Linux/macOS (Recommended)

#### Basic Compilation
```bash
gcc -O2 -march=native cache_benchmark.c -o cache_benchmark
```

#### Optimized Build (Recommended)
```bash
gcc -O3 -march=native -mtune=native -ffast-math cache_benchmark.c -o cache_benchmark
```

#### Debug Build
```bash
gcc -g -O0 -DDEBUG cache_benchmark.c -o cache_benchmark_debug
```

### Windows (MinGW/MSYS2)
```bash
gcc -O2 -march=native cache_benchmark.c -o cache_benchmark.exe
```

### Clang Alternative
```bash
clang -O2 -march=native -mtune=native cache_benchmark.c -o cache_benchmark
```

### Compiler Flags Explained
- `-O2`/`-O3`: Enable optimizations (essential for accurate timing)
- `-march=native`: Optimize for your specific CPU architecture
- `-mtune=native`: Tune performance for your CPU microarchitecture  
- `-ffast-math`: Enable fast floating-point optimizations

## Usage

### Basic Execution
```bash
./cache_benchmark
```

### Running with Process Priority (Linux/macOS)
```bash
# Run with high priority for more consistent results
sudo nice -n -20 ./cache_benchmark

# Or use taskset to pin to specific CPU core
taskset -c 0 ./cache_benchmark
```

### Redirecting Output
```bash
# Save results to file
./cache_benchmark > benchmark_results.txt

# Save with timestamp
./cache_benchmark | tee "benchmark_$(date +%Y%m%d_%H%M%S).txt"
```

## Understanding the Output

### Memory Latency Test
```
Size            Sequential (ms) Random (ms)     Bandwidth (GB/s)
--------------------------------------------------------
32 KB           133.38          1105.79         25.42
512 KB          148.17          1267.10         25.54
8 MB            193.69          3872.66         19.68
64 MB           292.47          8912.76         21.37
```
- **Size**: Working set size being tested
- **Sequential**: Time for cache-friendly linear access
- **Random**: Time for cache-hostile random access
- **Bandwidth**: Memory throughput in GB/s

### Cache Line Stride Test
```
Stride          Time (ms)       Efficiency
----------------------------------
1               24199.06        100.1%
64              381.53          6347.2%
128             192.69          12567.6%
```
- **Stride**: Byte offset between accesses
- **Efficiency**: Performance improvement over byte-by-byte access
- **64-byte peak**: Confirms cache line size

### Cache Thrashing Test
```
Cache Level     Time (ms)       Thrashing Factor
------------------------------------------
L1 (32KB)       1.56            1.84x
L2 (512KB)      1.58            3.00x
L3 (32MB)       25.97           2.98x
```
- **Thrashing Factor**: Performance degradation when exceeding associativity limits
- Higher values indicate more severe cache conflicts

## Performance Optimization Tips

### For Application Developers
1. **Keep hot data under L3 effective size** (often 8-16MB on modern CPUs)
2. **Use 64-byte aligned data structures** for optimal cache line utilization
3. **Prefer sequential access patterns** (4-10x faster than random)
4. **Consider cache blocking** for large data processing

### For System Administrators
1. **Disable CPU frequency scaling** during benchmarking
2. **Close unnecessary applications** to reduce cache pollution
3. **Use CPU affinity** to pin benchmark to specific cores
4. **Consider NUMA topology** on multi-socket systems

## Interpreting Results

### Expected Performance Characteristics
- **L1 Cache Hits**: 1-4 cycles (~0.5-2ns)
- **L2 Cache Hits**: 10-25 cycles (~5-12ns)  
- **L3 Cache Hits**: 40-100 cycles (~20-50ns)
- **Memory Access**: 200-400 cycles (~100-200ns)

### Common Patterns
- **Sharp latency increases** when exceeding cache sizes
- **Sequential vs Random**: 4-10x performance difference
- **Cache line effects**: Major performance gains at 64-byte boundaries
- **Associativity limits**: 2-4x slowdown when thrashing occurs

## Architecture-Specific Notes

### AMD Ryzen Series
- **L3 Cache**: Shared across cores in CCX (Core Complex)
- **Effective L3**: May be less than total due to cross-CCX latency
- **Memory Controller**: Integrated, affects DRAM latency

### Intel Core Series  
- **L3 Cache**: Fully shared across all cores
- **Ring Bus**: Affects L3 access latency based on core distance
- **Memory Controller**: Integrated since Nehalem

## Troubleshooting

### Compilation Issues
```bash
# Missing math library (rare)
gcc -O2 -march=native cache_benchmark.c -lm -o cache_benchmark

# Older GCC versions
gcc -std=c99 -O2 cache_benchmark.c -o cache_benchmark
```

### Runtime Issues
- **Inconsistent results**: Close background applications, disable frequency scaling
- **Memory allocation failures**: Reduce MAX_SIZE in source code
- **Permission denied**: Don't run as root unless necessary

### Platform-Specific
- **Windows**: Ensure MinGW/MSYS2 properly installed
- **macOS**: May require Developer Tools (`xcode-select --install`)
- **Linux**: Usually works out of the box with GCC

## Contributing

Feel free to submit issues and pull requests for:
- Additional CPU architecture support
- New benchmark patterns
- Performance optimizations
- Documentation improvements

## License

This project is released under the MIT License. See LICENSE file for details.

## Acknowledgments

- AMD and Intel for comprehensive architecture documentation
- The systems programming community for cache analysis techniques
- Contributors who provided testing on various CPU architectures

---

**Note**: Results may vary significantly between different CPU architectures, frequencies, and system configurations. Always run multiple iterations and consider system-specific factors when interpreting results.
