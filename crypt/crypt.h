#pragma once
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <fd/fd.h>

using namespace std; 

void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}
inline int decrypt(EVP_CIPHER_CTX* ctx,const unsigned char *ciphertext, int ciphertext_len, const unsigned char *key,
            const unsigned char *iv, unsigned char *plaintext)
{
    int len;
    int plaintext_len;
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleErrors();

    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
        handleErrors();
    plaintext_len = len;

    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
        handleErrors();
    plaintext_len += len;

    return plaintext_len;
}
inline int encrypt(EVP_CIPHER_CTX* ctx,const unsigned char *plaintext, int plaintext_len, const unsigned char *key,
            const unsigned char *iv, unsigned char *ciphertext)
{
    
    int len;
    int ciphertext_len;
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        handleErrors();

    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        handleErrors();
    ciphertext_len = len;

    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        handleErrors();
    ciphertext_len += len;

    return ciphertext_len;
}

/*
 * Aes crypt encrypts/decrypts using AES 128 in the following format
 *
 * For encryption a block of a maximum size of buffer_size (4096) is taken from input_fd, then it is encrypted to
 * cypher_buffer, then the number of encrypted bytes and the encrypted bytes are written to output.
 *
 * For decryption the number of encrypted bytes is read from input and then the encypted bytes are read.
 * Finally the decrypted data is written to the output fd
 *
 */

#include <iostream>
using namespace std;

template <bool mode>
int aes_crypt(int input_fd,const uint8_t* key)
{
    unsigned char *iv = (unsigned char *)"0123456789012345";
    if (input_fd < 0)
    {
        perror("invalid fd");
        exit(1);
    }

    int pipe_fd[2];
    pipe(pipe_fd);
    int f = fork();

    if (f == 0)
    {
        close(pipe_fd[0]);

        EVP_CIPHER_CTX *ctx;
        /* Create and initialise the context */
        if(!(ctx = EVP_CIPHER_CTX_new()))
            handleErrors();


        const int buffer_size  = 4096 * 2;
        uint8_t input_buffer[buffer_size * 2];
        uint8_t cipher_buffer[buffer_size * 2];

        int read_size = buffer_size;
        int n;
        while((mode || read(input_fd,&read_size,sizeof(int))) && ((n = read(input_fd,input_buffer,read_size)) > 0))
        {
            int nl;
            if constexpr (mode)
            {
                 nl = encrypt(ctx,input_buffer,n,key,iv,cipher_buffer);
                 write(pipe_fd[1],&nl,sizeof(int));
            }
            else{
                 nl = decrypt(ctx,input_buffer,n,key,iv,cipher_buffer);
            }
            write(pipe_fd[1],cipher_buffer,nl);
        }

        EVP_CIPHER_CTX_free(ctx);
    }

    close(input_fd);
    close(pipe_fd[1]);
    return pipe_fd[0];
}

const auto create_encrypt_device = aes_crypt<true>;
const auto create_decrypt_device = aes_crypt<false>;
