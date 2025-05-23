// serial_cracker.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include <time.h>
#include <math.h>

#define MAX_PASS_LEN 8
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
        printf("Trying length %d (%llu combinations)\n", len, (unsigned long long)pow(ALPHABET_SIZE, len));
        if (brute_force_recursive(target_hash, pass, len, 0, found_pass))
            return 1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <hash>\n", argv[0]);
        return 1;
    }

    const char *target = argv[1];
    char found_pass[MAX_PASS_LEN + 1] = {0};

    printf("Cracking: %s\n", target);
    printf("Warning: Maximum password length is %d characters (%llu total combinations)\n", 
           MAX_PASS_LEN, (unsigned long long)pow(ALPHABET_SIZE, MAX_PASS_LEN));

    clock_t start = clock();
    int res = brute_force_single(target, found_pass);
    clock_t end = clock();

    printf("Found: %s -> %s\n", target, found_pass);
    printf("Time: %.3f s\n", (double)(end - start) / CLOCKS_PER_SEC);
    return 0;
}
