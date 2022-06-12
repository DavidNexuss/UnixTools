#include "crypt.h"

unsigned char *key = (unsigned char *)"01234567890123456789012345678901";

void crypt()   {  cpy(create_encrypt_device(0,key),1); }
void decrypt() {  cpy(create_decrypt_device(0,key),1); }


int main (int argc,char** argv)
{
    string input = argc > 1 ? string(argv[1]) : "encrypt";

    if (input == "encrypt")      crypt();
    else if (input == "decrypt") decrypt();

    return 0;
}
