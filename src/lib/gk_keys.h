
#ifndef __GK_KEYS_H__
#define __GK_KEYS_H__

char *gk_keys_generated_private_key(void);
char *gk_keys_generated_public_key(void);
char *gk_keys_errors(void);
int gk_keys_has_errors(void);
int gk_keys_key_generation_in_progress(void);

int gk_keys_rsa_key_generate_background(int key_size_bits);
void printSslErrors(void);
int gk_keys_rsa_key_generate(int key_size_bits);
void gk_keys_rsa_key_free(void);





#endif // __GK_KEYS_H__
