#include "gk_keys.h"

#include <pthread.h>
#include <errno.h>
#include <stdint.h>

#include "openssl/pem.h"
#include "openssl/x509.h"
#include "openssl/err.h"

#include "gk_types.h"

int g_key_generation_in_progress = 0;
BIO *g_private_key_bio = NULL;
BIO *g_public_key_bio = NULL;
char *g_private_key = NULL;
char *g_public_key = NULL;
BIO *g_errors_bio = NULL;
char *g_errors = NULL;

static void gk_keys_rsa_key_free_ex(BIO *private_key_bio, BIO *public_key_bio, BIO *errors_bio);
static int gk_keys_rsa_key_generate_ex(int key_size_bits, BIO **private_key_bio, char **private_key, BIO **public_key_bio, char **public_key, BIO **errors_bio, char **errors);
static void freeSslErrors(BIO *errors_bio);
static void loadSslErrors(BIO **errors_bio, char **errors);


char *gk_keys_generated_private_key() {
    return g_private_key;
}

char *gk_keys_generated_public_key() {
    return g_public_key;
}

char *gk_keys_errors() {
    return g_errors;
}

int gk_keys_has_errors() {
    return g_errors != NULL;
}

int gk_keys_key_generation_in_progress() {
    return g_key_generation_in_progress;
}

static void *gk_keys_rsa_key_generate_background_worker(void *payload) {
    int key_size_bits = (intptr_t)payload;
    gk_keys_rsa_key_generate(key_size_bits);
    g_key_generation_in_progress = 0;
    return NULL;
}

int gk_keys_rsa_key_generate_background(int key_size_bits) {
    if (g_key_generation_in_progress == 1) {
        g_errors = "An ssh key generation is already in progress";
        return GK_FAILURE;
    }
    g_key_generation_in_progress = 1;

    pthread_t thread_id;
    int rc = pthread_create(&thread_id, NULL, gk_keys_rsa_key_generate_background_worker, (void *)(intptr_t) key_size_bits);
    pthread_detach(thread_id);

    if (rc != 0) {
        if (rc == EAGAIN) {
            g_errors = "Encountered EAGAIN while starting key generation background thread";
        }
        else {
            g_errors = "Error starting key genreation background thread";
        }
        return GK_FAILURE;
    }

    return GK_SUCCESS;
}

static void loadSslErrors(BIO **errors_bio, char **errors) {
    *errors_bio = BIO_new(BIO_s_mem());
    ERR_print_errors(*errors_bio);
    BIO_get_mem_data(*errors_bio, errors);
}

static void freeSslErrors(BIO *errors_bio) {
    if (errors_bio != NULL) BIO_free(errors_bio);
}

void printSslErrors() {
    BIO *bio = NULL;
    char *buf = NULL;
    loadSslErrors(&bio, &buf);
    if (buf != NULL) {
        printf("SSL error: %s\n", buf);
    }
    else {
        printf("Unknown SSL error\n");
    }
    freeSslErrors(bio);
}

int gk_keys_rsa_key_generate(int key_size_bits) {
    return gk_keys_rsa_key_generate_ex(key_size_bits, &g_private_key_bio, &g_private_key, &g_public_key_bio, &g_public_key, &g_errors_bio, &g_errors);
}

void gk_keys_rsa_key_free() {
    gk_keys_rsa_key_free_ex(g_private_key_bio, g_public_key_bio, g_errors_bio);
    g_private_key_bio = NULL;
    g_public_key_bio = NULL;
    g_errors_bio = NULL;
    g_private_key = NULL;
    g_public_key = NULL;
    g_errors = NULL;
}

static int gk_keys_rsa_key_generate_ex(int key_size_bits, BIO **private_key_bio, char **private_key, BIO **public_key_bio, char **public_key, BIO **errors_bio, char **errors) {
    if (errors == NULL) {
        printf("ERROR: gk_keys_rsa_key_generate_ex received NULL errors buffer, no key will be generated\n");
        return GK_FAILURE;
    }
    if (private_key_bio == NULL) {
        *errors = "private key bio pointer must not be null";
        return GK_FAILURE;
    }
    if (public_key_bio == NULL) {
        *errors = "public key bio pointer must not be null";
        return GK_FAILURE;
    }
    if (public_key == NULL) {
        *errors = "public key must not be null";
        return GK_FAILURE;
    }
    if (private_key == NULL) {
        *errors = "private key pointer must not be null";
        return GK_FAILURE;
    }
    if (errors_bio == NULL) {
        *errors = "bio errors pointer must not be null";
        return GK_FAILURE;
    }

    EVP_PKEY *pkey = EVP_PKEY_new();
    if (!pkey) {
        loadSslErrors(errors_bio, errors);
        return GK_FAILURE;
    }

    BIGNUM *bne = BN_new();
    if (bne == NULL) {
        loadSslErrors(errors_bio, errors);
        return GK_FAILURE;
    }
    if (BN_set_word(bne, RSA_F4) != 1) {
        loadSslErrors(errors_bio, errors);
        return GK_FAILURE;
    }

    RSA *rsa = RSA_new();
    if (rsa == NULL) {
        loadSslErrors(errors_bio, errors);
        return GK_FAILURE;
    }
    
    if (RSA_generate_key_ex(rsa, key_size_bits, bne, NULL) != 1) {
        loadSslErrors(errors_bio, errors);
        return GK_FAILURE;
    }
    
    if (!EVP_PKEY_assign_RSA(pkey, rsa)) {
        loadSslErrors(errors_bio, errors);
        return GK_FAILURE;
    }

    *private_key_bio = BIO_new(BIO_s_mem());
    PEM_write_bio_RSAPrivateKey(*private_key_bio, rsa, NULL, NULL, 0, NULL, NULL);
    int pem_private_key_size = BIO_pending(*private_key_bio);
    *private_key = (char*) calloc((pem_private_key_size)+1, 1);
    BIO_read(*private_key_bio, *private_key, pem_private_key_size);
    
    *public_key_bio = BIO_new(BIO_s_mem());
    PEM_write_bio_RSAPublicKey(*public_key_bio, rsa);
    int pem_public_key_size = BIO_pending(*public_key_bio);
    *public_key = (char*) calloc((pem_public_key_size)+1, 1);
    BIO_read(*public_key_bio, *public_key, pem_public_key_size);

    return GK_SUCCESS;
}

static void gk_keys_rsa_key_free_ex(BIO *private_key_bio, BIO *public_key_bio, BIO *errors_bio) {
    if (private_key_bio != NULL) BIO_free(private_key_bio);
    if (public_key_bio != NULL) BIO_free(public_key_bio);
    if (errors_bio != NULL) BIO_free(errors_bio);
}
