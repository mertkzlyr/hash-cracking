// openmp_cracker.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <openssl/md5.h>

#define MAX_PASS_LEN 8  // Increased to 8 characters
#define ALPHABET "abcdefghijklmnopqrstuvwxyz0123456789"
#define ALPHABET_SIZE 36
#define CHUNK_SIZE 10000

void index_to_password(unsigned long long idx, int length, char *pass)
{
    for (int i = length - 1; i >= 0; i--)
    {
        pass[i] = ALPHABET[idx % ALPHABET_SIZE];
        idx /= ALPHABET_SIZE;
    }
    pass[length] = '\0';
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
    if (argc != 3)
    {
        printf("Usage: %s <num_threads> <hash>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);
    const char *target = argv[2];
    char found_pass[MAX_PASS_LEN + 1] = {0};
    int found = 0;

    omp_set_num_threads(num_threads);
    printf("Cracking: %s\n", target);
    printf("Using %d threads\n", num_threads);
    printf("Warning: Maximum password length is %d characters (%llu total combinations)\n", 
           MAX_PASS_LEN, total_passwords(MAX_PASS_LEN));
    double start = omp_get_wtime();

    for (int len = 1; len <= MAX_PASS_LEN && !found; len++)
    {
        unsigned long long max = total_passwords(len);
        printf("Trying length %d (%llu combinations)\n", len, max);

        #pragma omp parallel
        {
            char local_pass[MAX_PASS_LEN + 1];
            char local_hash[33];
            int local_found = 0;
            char local_found_pass[MAX_PASS_LEN + 1] = {0};

            #pragma omp for schedule(dynamic, CHUNK_SIZE) nowait
            for (unsigned long long i = 0; i < max; i++)
            {
                if (local_found) continue;

                index_to_password(i, len, local_pass);
                md5_hash(local_pass, local_hash);

                if (strcmp(local_hash, target) == 0)
                {
                    local_found = 1;
                    strcpy(local_found_pass, local_pass);
                }
            }

            if (local_found)
            {
                #pragma omp critical
                {
                    if (!found)
                    {
                        found = 1;
                        strcpy(found_pass, local_found_pass);
                    }
                }
            }
        }

        if (found) break;
    }

    double end = omp_get_wtime();
    printf("Found: %s -> %s\n", target, found_pass);
    printf("Time: %.3f s\n", end - start);
    return 0;
}
