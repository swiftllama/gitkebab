#include "gk_keys.h"

#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/err.h>

void printSslErrors() {
    BIO *bio = BIO_new(BIO_s_mem());
    ERR_print_errors(bio);
    char *buf;
    BIO_get_mem_data(bio, &buf);
    if (buf != NULL) {
        printf("SSL error: %s\n", buf);
    }
    else {
        printf("Unknown SSL error\n");
    }
    BIO_free(bio);
}

char *gk_keys_rsa_key_generate(int *key_length) {
    if (key_length == NULL) {
        printf("DBG key length must not be null\n");
        return NULL;
    }
    
    EVP_PKEY *pkey = EVP_PKEY_new();
    if (!pkey) {
        printf("DBG error creating pkey\n");
        printSslErrors();
        return NULL;
    }

    BIGNUM *bne = BN_new();
    if (bne == NULL) {
        printf("DBG error genreating big number\n");
        printSslErrors();
        return NULL;
    }
    if (BN_set_word(bne, RSA_F4) != 1) {
        printf("DBG error setting big number\n");
        printSslErrors();
        return NULL;
    }

    RSA *rsa = RSA_new();
    if (rsa == NULL) {
        printf("DBG error creating RSA structure\n");
        printSslErrors();
        return NULL;
    }
    
    if (RSA_generate_key_ex(rsa, 2048, bne, NULL) != 1) {
        printf("DBG error generating key\n");
        printSslErrors();
        return NULL;
    }
    
    if (!EVP_PKEY_assign_RSA(pkey, rsa)) {
        printf("DBG error genreeating key\n");
        printSslErrors();
        return NULL;
    }


    BIO *bio = BIO_new(BIO_s_mem());
    PEM_write_bio_RSAPrivateKey(bio, rsa, NULL, NULL, 0, NULL, NULL);

    int pem_pkey_size = BIO_pending(bio);
    char *new_key = (char*) calloc((pem_pkey_size)+1, 1);
    BIO_read(bio, new_key, pem_pkey_size);

    return new_key;
}

void gk_keys_rsa_key_free(char *key) {
    if (key != NULL) free(key);
}
