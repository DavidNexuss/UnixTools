#include "crypt.h"

unsigned char *key = (unsigned char *)"01234567890123456789012345678901";

void crypt()   {  encrypt(0,1,key); }
void decrypt() {  decrypt(0,1,key); }

void test() { 
    decrypt(virtual_pipe(0,[key](auto in,auto out) { encrypt(in,out,key); }), 1, key);
}
int main (int argc,char** argv)
{
    test();
    string input = argc > 1 ? string(argv[1]) : "encrypt";

    if (input == "encrypt")      crypt();
    else if (input == "decrypt") decrypt();

    return 0;
}
