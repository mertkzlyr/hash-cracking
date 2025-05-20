// serial_cracker.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include <time.h>

#define MAX_PASS_LEN 5
#define ALPHABET "abcdefghijklmnopqrstuvwxyz0123456789"
#define ALPHABET_SIZE 36

void md5_hash(const char *input, char *output)
{
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5((unsigned char *)input, strlen(input), digest);
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        sprintf(&output[i * 2], "%02x", digest[i]);
    output[32] = 0;
}

int brute_force_recursive(const char *target_hash, char *pass, int length, int pos, char *found)
{
    if (pos == length)
    {
        pass[pos] = '\0';
        char hash[33];
        md5_hash(pass, hash);
        if (strcmp(hash, target_hash) == 0)
        {
            strcpy(found, pass);
            return 1;
        }
        return 0;
    }
    for (int i = 0; i < ALPHABET_SIZE; i++)
    {
        pass[pos] = ALPHABET[i];
        if (brute_force_recursive(target_hash, pass, length, pos + 1, found))
            return 1;
    }
    return 0;
}

int brute_force_single(const char *target_hash, char *found_pass)
{
    char pass[MAX_PASS_LEN + 1];
    for (int len = 1; len <= MAX_PASS_LEN; len++)
    {
        if (brute_force_recursive(target_hash, pass, len, 0, found_pass))
            return 1;
    }
    return 0;
}

int main()
{
    FILE *f = fopen("hashes.txt", "r");
    if (!f)
    {
        perror("hashes.txt");
        return 1;
    }

    char hash[33];
    while (fgets(hash, sizeof(hash), f))
    {
        hash[strcspn(hash, "\r\n")] = 0;
        if (strlen(hash) != 32)
            continue;

        char found_pass[MAX_PASS_LEN + 1] = {0};
        printf("Cracking: %s\n", hash);

        clock_t start = clock();
        int res = brute_force_single(hash, found_pass);
        clock_t end = clock();

        if (res)
            printf("Found: %s -> %s\n", hash, found_pass);
        else
            printf("Not found: %s\n", hash);

        printf("Time: %.3f s\n\n", (double)(end - start) / CLOCKS_PER_SEC);
    }

    fclose(f);
    return 0;
}
