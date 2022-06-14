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

unsigned char *iv = (unsigned char *)"0123456789012345";
const inline int buffer_size = 4096;
void encrypt(int input_fd,int output_fd, const uint8_t* key) { 


    EVP_CIPHER_CTX *ctx;
    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();


    uint8_t* input_buffer = new uint8_t[buffer_size];
    uint8_t* cipher_buffer = new uint8_t[buffer_size * 2];

    int input_size;

    int buffer[2];
    pipe(buffer);

    while((input_size = read(input_fd,input_buffer,buffer_size)) > 0) { 
        int encrypted_size = encrypt(ctx,input_buffer,input_size,key,iv,cipher_buffer);
        write(output_fd,&encrypted_size,sizeof(encrypted_size));
        write(output_fd,cipher_buffer,encrypted_size);
        //For atomic writes :
        //splice(buffer[0],nullptr,output_fd,nullptr,sizeof(encrypted_size) + encrypted_size,SPLICE_F_MOVE);
    }
}

void decrypt(int input_fd,int output_fd,const uint8_t* key) { 
    EVP_CIPHER_CTX *ctx;
    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    uint8_t* input_buffer = new uint8_t[buffer_size * 2];
    uint8_t* cipher_buffer = new uint8_t[buffer_size * 2];

    int input_size;
    
    while(read(input_fd,&input_size,sizeof(input_size)) == sizeof(input_size)) { 
        int newInputSize = read(input_fd,input_buffer,input_size);
        if(newInputSize != input_size) { 
            cerr << newInputSize << " " << input_size  << endl;
            exit(1);
        }
        int decrypted_size = decrypt(ctx,input_buffer,input_size,key,iv,cipher_buffer);
        write(output_fd,cipher_buffer,decrypted_size);
    }
}

template <typename T>
int virtual_pipe(int input_fd,T&& function) { 

    if (input_fd < 0)
    {
        perror("invalid fd");
        exit(1);
    }

    int pipe_fd[2];
    pipe(pipe_fd);
    int f = fork();
    if(f == 0) { 
        close(pipe_fd[0]);
        function(input_fd,pipe_fd[1]);
    }

    close(input_fd);
    close(pipe_fd[1]);
    return pipe_fd[0];

}
