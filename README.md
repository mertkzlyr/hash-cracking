# Password Cracking Performance Analysis

This project compares the performance of different parallel computing approaches (Serial, OpenMP, and MPI) for password cracking using MD5 hashing.

## 🚀 Performance Results

### Hash 1: `b2157e7b2ae716a747597717f1efb7a0` (password: "abc12")

| Implementation | Time (s) | Speedup |
|----------------|----------|---------|
| Serial         | 1.256    | 1.00x   |
| OpenMP (1t)    | 1.300    | 0.97x   |
| OpenMP (2t)    | 1.570    | 0.80x   |
| OpenMP (3t)    | 1.725    | 0.73x   |
| OpenMP (4t)    | 2.121    | 0.59x   |
| MPI (1p)       | 1.327    | 0.95x   |
| MPI (2p)       | 0.869    | 1.61x   |
| MPI (3p)       | 0.781    | 1.61x   |
| MPI (4p)       | 0.748    | 1.68x   |

### Hash 2: `e99a18c428cb38d5f260853678922e03` (password: "abc123")

| Implementation | Time (s) | Speedup |
|----------------|----------|---------|
| Serial         | 44.729   | 1.00x   |
| OpenMP (1t)    | 46.514   | 0.96x   |
| OpenMP (2t)    | 56.632   | 0.79x   |
| OpenMP (3t)    | 62.779   | 0.71x   |
| OpenMP (4t)    | 75.407   | 0.59x   |
| MPI (1p)       | 47.528   | 0.94x   |
| MPI (2p)       | 31.009   | 1.44x   |
| MPI (3p)       | 29.912   | 1.50x   |
| MPI (4p)       | 26.804   | 1.67x   |

## 📊 Key Observations

1. **MPI Performance**
   - Shows consistent improvement with more processes
   - Achieves up to 1.68x speedup with 4 processes
   - Slightly slower than serial with 1 process due to initialization overhead
   - More predictable and efficient scaling

2. **OpenMP Performance**
   - Performance degrades as thread count increases
   - 4 threads result in 0.59x speedup (slower than serial)
   - Overhead increases with more threads
   - Less efficient for this type of workload

3. **Workload Characteristics**
   - Performance difference becomes more pronounced with longer passwords
   - CPU-intensive with minimal communication
   - Well-suited for MPI's process-based approach

## 🔍 Why MPI Outperforms OpenMP

### MPI Advantages
- **Process Isolation**: Each process has its own memory space
- **No Memory Contention**: Eliminates shared memory issues
- **Better Cache Utilization**: Each process has dedicated cache
- **Minimal Synchronization**: Communication only when needed
- **Static Work Distribution**: Predictable workload allocation

### OpenMP Challenges
- **Shared Memory**: Threads compete for memory access
- **Cache Coherency**: Overhead in maintaining cache consistency
- **Thread Scheduling**: Additional overhead in thread management
- **Synchronization**: Critical sections create contention
- **Dynamic Scheduling**: Overhead in work distribution

## 🔄 Threads vs Processes: The Fundamental Difference

### OpenMP Threads
- **Memory Space**: Share the same memory space with the main program
- **Resource Sharing**: Share CPU cache, memory bus, and other resources
- **Communication**: Direct memory access between threads
- **Overhead**: Lower creation overhead but higher synchronization overhead
- **Use Case**: Best for:
  - Fine-grained parallelism
  - Tasks requiring frequent communication
  - Shared memory architectures
  - I/O-bound operations

### MPI Processes
- **Memory Space**: Each process has its own isolated memory space
- **Resource Sharing**: No direct resource sharing between processes
- **Communication**: Explicit message passing between processes
- **Overhead**: Higher creation overhead but lower synchronization overhead
- **Use Case**: Best for:
  - Coarse-grained parallelism
  - CPU-intensive tasks
  - Distributed memory systems
  - Tasks requiring minimal communication

### Why Processes Work Better for Password Cracking
1. **Memory Isolation**
   - Each process has its own memory space
   - No need to synchronize memory access
   - Better cache utilization per process

2. **Resource Contention**
   - Processes don't compete for the same memory bus
   - Each process can use CPU cache more efficiently
   - No cache coherency overhead

3. **Communication Pattern**
   - Password cracking requires minimal communication
   - Only need to check if password is found
   - MPI's message passing is sufficient

4. **Workload Characteristics**
   - CPU-intensive computation
   - Independent password generation
   - No need for frequent data sharing

## 🏗️ Implementation Details

### Core Algorithm
All implementations use the same basic approach:
1. Generate passwords of increasing length (1 to MAX_PASS_LEN)
2. Calculate MD5 hash for each password
3. Compare with target hash
4. Return when match is found

### Serial Implementation
```c
int brute_force_recursive(const char *target_hash, char *pass, int length, int pos, char *found) {
    if (pos == length) {
        pass[pos] = '\0';
        char hash[33];
        md5_hash(pass, hash);
        if (strcmp(hash, target_hash) == 0) {
            strcpy(found, pass);
            return 1;
        }
        return 0;
    }
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        pass[pos] = ALPHABET[i];
        if (brute_force_recursive(target_hash, pass, length, pos + 1, found))
            return 1;
    }
    return 0;
}
```

**Serial Implementation Explanation:**
- Uses a recursive approach to generate all possible passwords
- `pos` tracks the current position in the password being generated
- When `pos == length`, we have a complete password to test
- For each position, tries all characters in the alphabet
- Returns 1 if password is found, 0 if not
- Simple but efficient for single-threaded execution

### MPI Implementation
```c
// Static work distribution
unsigned long long chunk = total / size;
unsigned long long start = rank * chunk;
unsigned long long end = (rank == size - 1) ? total : start + chunk;

// Each process works on its chunk
for (unsigned long long i = start; i < end && !found; i++) {
    char pass[MAX_PASS_LEN + 1];
    char hash[33];
    index_to_password(i, len, pass);
    md5_hash(pass, hash);
    
    if (strcmp(hash, target) == 0) {
        strcpy(found_pass, pass);
        found = 1;
    }
    
    // Check if any process found the password
    int global_found;
    MPI_Allreduce(&found, &global_found, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    found = global_found;
}
```

**MPI Implementation Explanation:**
- Divides work statically among processes
- Each process gets a chunk of the password space to search
- `chunk = total / size` calculates the size of each chunk
- `start` and `end` define the range for each process
- Last process gets any remaining work (`rank == size - 1`)
- `MPI_Allreduce` synchronizes the found status across all processes
- No shared memory, each process works independently
- Minimal communication overhead

### OpenMP Implementation
```c
#pragma omp parallel
{
    char local_pass[MAX_PASS_LEN + 1];
    char local_hash[33];
    int local_found = 0;
    char local_found_pass[MAX_PASS_LEN + 1] = {0};

    #pragma omp for schedule(dynamic, CHUNK_SIZE) nowait
    for (unsigned long long i = 0; i < max; i++) {
        if (local_found) continue;
        
        index_to_password(i, len, local_pass);
        md5_hash(local_pass, local_hash);
        
        if (strcmp(local_hash, target) == 0) {
            local_found = 1;
            strcpy(local_found_pass, local_pass);
        }
    }

    // Critical section for updating global found state
    if (local_found) {
        #pragma omp critical
        {
            if (!found) {
                found = 1;
                strcpy(found_pass, local_found_pass);
            }
        }
    }
}
```

**OpenMP Implementation Explanation:**
- Uses shared memory parallelization with threads
- Each thread has local variables to avoid contention
- `schedule(dynamic, CHUNK_SIZE)` dynamically assigns work to threads
- `nowait` allows threads to continue without waiting for others
- Critical section ensures safe updates to shared variables
- Threads share memory space, leading to potential contention
- Dynamic scheduling adds overhead but provides load balancing

## 🔧 Requirements

- OpenMP
- MPI
- OpenSSL
- C Compiler (GCC recommended)

## 🛠️ Compilation

```bash
# Serial Version
gcc -o serial_cracker serial_cracker.c -lcrypto

# OpenMP Version
gcc -fopenmp -o openmp_cracker openmp_cracker.c -lcrypto

# MPI Version
mpicc -o mpi_cracker mpi_cracker.c -lcrypto
```

## 🚀 Usage

```bash
# Serial Version
./serial_cracker <hash>

# OpenMP Version
./openmp_cracker <num_threads> <hash>

# MPI Version
mpirun -np <num_processes> ./mpi_cracker <hash>
```

## 📝 Conclusion

This analysis demonstrates that MPI is the superior choice for this password cracking workload. The process-based approach of MPI provides better scalability and more efficient parallelization compared to OpenMP's thread-based approach. The isolation of processes in MPI eliminates many of the overheads that OpenMP threads face, resulting in better performance scaling with increased parallelism.
