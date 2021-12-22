
#ifndef __GK_KEYS_H__
#define __GK_KEYS_H__

char *gk_keys_generated_private_key();
char *gk_keys_generated_public_key();
char *gk_keys_errors();
int gk_keys_has_errors();
int gk_keys_key_generation_in_progress();

int gk_keys_rsa_key_generate_background(int key_size_bits);
void printSslErrors();
int gk_keys_rsa_key_generate(int key_size_bits);
void gk_keys_rsa_key_free();





#endif // __GK_KEYS_H__
