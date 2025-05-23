#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>

#ifdef _WIN32
#include <windows.h>
#define CACHE_LINE_SIZE 64
#else
#include <sys/time.h>
#define CACHE_LINE_SIZE 64
#endif

// Typical cache sizes for AMD Ryzen 5600
#define L1_CACHE_SIZE (32 * 1024)      // 32KB L1 Data Cache per core
#define L2_CACHE_SIZE (512 * 1024)     // 512KB L2 Cache per core
#define L3_CACHE_SIZE (32 * 1024 * 1024) // 32MB L3 Cache shared

// Test array sizes
#define MIN_SIZE (4 * 1024)            // 4KB
#define MAX_SIZE (128 * 1024 * 1024)   // 128MB
#define NUM_ITERATIONS 1000000

// High-resolution timer functions
static inline uint64_t get_cycles() {
    uint32_t lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

double get_time_ms() {
#ifdef _WIN32
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart * 1000.0 / freq.QuadPart;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
#endif
}

// Sequential access benchmark
double benchmark_sequential_access(void* buffer, size_t size, int iterations) {
    volatile char* ptr = (volatile char*)buffer;
    volatile char dummy;
    
    double start_time = get_time_ms();
    
    for (int i = 0; i < iterations; i++) {
        for (size_t j = 0; j < size; j += CACHE_LINE_SIZE) {
            dummy = ptr[j];
        }
    }
    
    double end_time = get_time_ms();
    return end_time - start_time;
}

// Random access benchmark
double benchmark_random_access(void* buffer, size_t size, int iterations) {
    volatile char* ptr = (volatile char*)buffer;
    volatile char dummy;
    size_t num_elements = size / sizeof(size_t);
    
    // Create random access pattern
    size_t* indices = malloc(num_elements * sizeof(size_t));
    for (size_t i = 0; i < num_elements; i++) {
        indices[i] = (rand() % num_elements) * sizeof(size_t);
    }
    
    double start_time = get_time_ms();
    
    for (int i = 0; i < iterations; i++) {
        for (size_t j = 0; j < num_elements; j++) {
            dummy = ptr[indices[j]];
        }
    }
    
    double end_time = get_time_ms();
    free(indices);
    return end_time - start_time;
}

// Stride access benchmark to test cache line effects
double benchmark_stride_access(void* buffer, size_t size, int stride, int iterations) {
    volatile char* ptr = (volatile char*)buffer;
    volatile char dummy;
    
    double start_time = get_time_ms();
    
    for (int i = 0; i < iterations; i++) {
        for (size_t j = 0; j < size; j += stride) {
            dummy = ptr[j];
        }
    }
    
    double end_time = get_time_ms();
    return end_time - start_time;
}

// Memory write benchmark
double benchmark_write_access(void* buffer, size_t size, int iterations) {
    volatile char* ptr = (volatile char*)buffer;
    
    double start_time = get_time_ms();
    
    for (int i = 0; i < iterations; i++) {
        for (size_t j = 0; j < size; j += CACHE_LINE_SIZE) {
            ptr[j] = (char)(i + j);
        }
    }
    
    double end_time = get_time_ms();
    return end_time - start_time;
}

// Cache associativity test
double benchmark_associativity(size_t cache_size, int ways) {
    size_t stride = cache_size / ways;
    size_t total_size = cache_size * 2;
    char* buffer = aligned_alloc(4096, total_size);
    
    if (!buffer) {
        printf("Failed to allocate memory for associativity test\n");
        return -1;
    }
    
    memset(buffer, 0, total_size);
    
    volatile char* ptr = (volatile char*)buffer;
    volatile char dummy;
    
    double start_time = get_time_ms();
    
    for (int i = 0; i < NUM_ITERATIONS / 10; i++) {
        for (int w = 0; w < ways + 1; w++) {
            dummy = ptr[w * stride];
        }
    }
    
    double end_time = get_time_ms();
    free(buffer);
    return end_time - start_time;
}

void print_cache_info() {
    printf("=== AMD Ryzen 5600 Cache Hierarchy ===\n");
    printf("L1 Data Cache: 32KB per core (8-way associative)\n");
    printf("L2 Cache: 512KB per core (8-way associative)\n");
    printf("L3 Cache: 32MB shared (16-way associative)\n");
    printf("Cache Line Size: 64 bytes\n");
    printf("==========================================\n\n");
}

void run_latency_test() {
    printf("=== Memory Latency Test ===\n");
    printf("Size\t\tSequential (ms)\tRandom (ms)\tBandwidth (GB/s)\n");
    printf("--------------------------------------------------------\n");
    
    for (size_t size = MIN_SIZE; size <= MAX_SIZE; size *= 2) {
        char* buffer = aligned_alloc(4096, size);
        if (!buffer) {
            printf("Failed to allocate %zu bytes\n", size);
            continue;
        }
        
        memset(buffer, 0xAA, size);
        
        int iterations = NUM_ITERATIONS / (size / MIN_SIZE + 1);
        if (iterations < 100) iterations = 100;
        
        double seq_time = benchmark_sequential_access(buffer, size, iterations);
        double rand_time = benchmark_random_access(buffer, size, iterations);
        
        double bandwidth = (double)(size * iterations) / (seq_time / 1000.0) / (1024*1024*1024);
        
        if (size < 1024) {
            printf("%zu B\t\t", size);
        } else if (size < 1024 * 1024) {
            printf("%zu KB\t\t", size / 1024);
        } else {
            printf("%zu MB\t\t", size / (1024 * 1024));
        }
        
        printf("%.2f\t\t%.2f\t\t%.2f\n", seq_time, rand_time, bandwidth);
        
        free(buffer);
    }
    printf("\n");
}

void run_stride_test() {
    printf("=== Cache Line Stride Test ===\n");
    printf("Testing with 1MB buffer\n");
    printf("Stride\t\tTime (ms)\tEfficiency\n");
    printf("----------------------------------\n");
    
    size_t test_size = 1024 * 1024;
    char* buffer = aligned_alloc(4096, test_size);
    if (!buffer) {
        printf("Failed to allocate buffer\n");
        return;
    }
    
    memset(buffer, 0xAA, test_size);
    
    double baseline_time = benchmark_stride_access(buffer, test_size, 1, NUM_ITERATIONS / 100);
    
    int strides[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512};
    int num_strides = sizeof(strides) / sizeof(strides[0]);
    
    for (int i = 0; i < num_strides; i++) {
        double time = benchmark_stride_access(buffer, test_size, strides[i], NUM_ITERATIONS / 100);
        double efficiency = baseline_time / time * 100.0;
        
        printf("%d\t\t%.2f\t\t%.1f%%\n", strides[i], time, efficiency);
    }
    
    free(buffer);
    printf("\n");
}

void run_cache_thrashing_test() {
    printf("=== Cache Thrashing Test ===\n");
    printf("Testing cache associativity limits\n");
    printf("Cache Level\tTime (ms)\tThrashing Factor\n");
    printf("------------------------------------------\n");
    
    // Test L1 cache thrashing
    double l1_normal = benchmark_associativity(L1_CACHE_SIZE, 8);
    double l1_thrash = benchmark_associativity(L1_CACHE_SIZE, 16);
    printf("L1 (32KB)\t%.2f\t\t%.2fx\n", l1_normal, l1_thrash / l1_normal);
    
    // Test L2 cache thrashing
    double l2_normal = benchmark_associativity(L2_CACHE_SIZE, 8);
    double l2_thrash = benchmark_associativity(L2_CACHE_SIZE, 16);
    printf("L2 (512KB)\t%.2f\t\t%.2fx\n", l2_normal, l2_thrash / l2_normal);
    
    // Test L3 cache thrashing
    double l3_normal = benchmark_associativity(L3_CACHE_SIZE, 16);
    double l3_thrash = benchmark_associativity(L3_CACHE_SIZE, 32);
    printf("L3 (32MB)\t%.2f\t\t%.2fx\n", l3_normal, l3_thrash / l3_normal);
    
    printf("\n");
}

void run_read_write_comparison() {
    printf("=== Read vs Write Performance ===\n");
    printf("Size\t\tRead (ms)\tWrite (ms)\tWrite/Read Ratio\n");
    printf("--------------------------------------------------\n");
    
    size_t sizes[] = {L1_CACHE_SIZE, L2_CACHE_SIZE, L3_CACHE_SIZE, L3_CACHE_SIZE * 2};
    const char* labels[] = {"L1 (32KB)", "L2 (512KB)", "L3 (32MB)", "RAM (64MB)"};
    
    for (int i = 0; i < 4; i++) {
        char* buffer = aligned_alloc(4096, sizes[i]);
        if (!buffer) continue;
        
        memset(buffer, 0xAA, sizes[i]);
        
        int iterations = NUM_ITERATIONS / (sizes[i] / MIN_SIZE + 1);
        if (iterations < 100) iterations = 100;
        
        double read_time = benchmark_sequential_access(buffer, sizes[i], iterations);
        double write_time = benchmark_write_access(buffer, sizes[i], iterations);
        
        printf("%s\t\t%.2f\t\t%.2f\t\t%.2f\n", 
               labels[i], read_time, write_time, write_time / read_time);
        
        free(buffer);
    }
    printf("\n");
}

void print_analysis() {
    printf("=== Performance Analysis ===\n");
    printf("Expected behavior for AMD Ryzen 5600:\n");
    printf("• L1 Cache (32KB): Fastest access, ~1-2 cycles latency\n");
    printf("• L2 Cache (512KB): ~10-12 cycles latency\n");
    printf("• L3 Cache (32MB): ~40-50 cycles latency\n");
    printf("• Main Memory: ~200+ cycles latency\n\n");
    
    printf("Key observations:\n");
    printf("• Sharp latency increase when exceeding cache sizes\n");
    printf("• Random access much slower than sequential\n");
    printf("• Cache line size (64B) affects stride performance\n");
    printf("• Cache associativity limits cause thrashing\n");
    printf("• Write performance generally slower than reads\n");
    printf("================================\n");
}

int main() {
    printf("CPU Cache Benchmark Tool\n");
    printf("Optimized for AMD Ryzen 5600\n");
    printf("========================\n\n");
    
    srand(time(NULL));
    
    print_cache_info();
    
    printf("Running benchmarks... (this may take a few minutes)\n\n");
    
    run_latency_test();
    run_stride_test();
    run_cache_thrashing_test();
    run_read_write_comparison();
    print_analysis();
    
    return 0;
}
