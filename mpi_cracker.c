// mpi_cracker.c
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>

#define MAX_PASS_LEN 8
#define ALPHABET "abcdefghijklmnopqrstuvwxyz0123456789"
#define ALPHABET_SIZE 36

void index_to_password(unsigned long long idx, int len, char *pass)
{
    for (int i = len - 1; i >= 0; i--)
    {
        pass[i] = ALPHABET[idx % ALPHABET_SIZE];
        idx /= ALPHABET_SIZE;
    }
    pass[len] = '\0';
}

void md5_hash(const char *input, char *output)
{
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5((unsigned char *)input, strlen(input), digest);
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        sprintf(&output[i * 2], "%02x", digest[i]);
    output[32] = 0;
}

unsigned long long total_passwords(int len)
{
    unsigned long long total = 1;
    for (int i = 0; i < len; i++)
        total *= ALPHABET_SIZE;
    return total;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2)
    {
        if (rank == 0)
            printf("Usage: mpirun -np <n> %s <hash>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    const char *target = argv[1];
    char found_pass[MAX_PASS_LEN + 1] = {0};
    int found = 0;

    if (rank == 0)
    {
        printf("Cracking: %s\n", target);
        printf("Warning: Maximum password length is %d characters (%llu total combinations)\n", 
               MAX_PASS_LEN, total_passwords(MAX_PASS_LEN));
    }

    double t_start = MPI_Wtime();

    for (int len = 1; len <= MAX_PASS_LEN && !found; len++)
    {
        unsigned long long total = total_passwords(len);
        if (rank == 0)
            printf("Trying length %d (%llu combinations)\n", len, total);

        unsigned long long chunk = total / size;
        unsigned long long start = rank * chunk;
        unsigned long long end = (rank == size - 1) ? total : start + chunk;

        for (unsigned long long i = start; i < end && !found; i++)
        {
            char pass[MAX_PASS_LEN + 1];
            char hash[33];
            index_to_password(i, len, pass);
            md5_hash(pass, hash);

            if (strcmp(hash, target) == 0)
            {
                strcpy(found_pass, pass);
                found = 1;
            }

            int global_found;
            MPI_Allreduce(&found, &global_found, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
            found = global_found;
        }

        int who_found = (found && strlen(found_pass) > 0) ? rank : -1;
        int final_rank;
        MPI_Allreduce(&who_found, &final_rank, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

        if (final_rank != -1)
        {
            MPI_Bcast(found_pass, MAX_PASS_LEN + 1, MPI_CHAR, final_rank, MPI_COMM_WORLD);
            break;
        }
    }

    double t_end = MPI_Wtime();
    if (rank == 0)
    {
        printf("Found: %s -> %s\n", target, found_pass);
        printf("Time: %.3f s\n", t_end - t_start);
    }

    MPI_Finalize();
    return 0;
}
