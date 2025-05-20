In hashes.txt file hashes are equals to:
b2157e7b2ae716a747597717f1efb7a0 # abc12

compile serial.c with gcc -o serial_cracker serial_cracker.c -lcrypto

compile mpi_cracker.c with mpicc -o mpi_cracker mpi_cracker.c -lcrypto
usage for ./mpi_cracker: Usage: mpirun -np <num_procs> ./mpi_cracker <hash>

usage for: export OMP_NUM_THREADS=4 && ./openmp_cracker 5f4dcc3b5aa765d61d8327deb882cf99
