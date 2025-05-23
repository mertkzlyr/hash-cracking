// openmp_cracker.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <openssl/md5.h>

#define MAX_PASS_LEN 5
#define ALPHABET "abcdefghijklmnopqrstuvwxyz0123456789"
#define ALPHABET_SIZE 36
#define CHUNK_SIZE 1000

int found = 0;
char found_pass[MAX_PASS_LEN + 1] = {0};

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
    printf("Program başladı\n");
    if (argc != 2)
    {
        printf("Usage: %s <hash>\n", argv[0]);
        return 1;
    }

    const char *target = argv[1];
    found = 0;
    memset(found_pass, 0, sizeof(found_pass));

    printf("Cracking: %s\n", target);
    printf("Using %d threads\n", omp_get_max_threads());
    double start = omp_get_wtime();

    for (int len = 1; len <= MAX_PASS_LEN && !found; len++)
    {
        unsigned long long max = total_passwords(len);
        printf("Trying length %d (%llu combinations)\n", len, max);

#pragma omp parallel for schedule(dynamic, CHUNK_SIZE) shared(found, found_pass, target)
        for (unsigned long long i = 0; i < max; i++)
        {
            if (found)
                continue;

            char pass[MAX_PASS_LEN + 1];
            char hash[33];
            index_to_password(i, len, pass);
            md5_hash(pass, hash);

            if (strcmp(hash, target) == 0)
            {
#pragma omp critical
                {
                    if (!found)
                    {
                        strcpy(found_pass, pass);
                        found = 1;
                        printf("Thread %d found the password!\n", omp_get_thread_num());
                    }
                }
            }
        }
    }

    double end = omp_get_wtime();

    if (found_pass[0])
        printf("Found: %s -> %s\n", target, found_pass);
    else
        printf("Not found: %s\n", target);

    printf("Time: %.3f s\n", end - start);
    return 0;
}
