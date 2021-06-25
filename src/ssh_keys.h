
#ifndef __GITKEBAB_SSH_KEYS_H__
#define __GITKEBAB_SSH_KEYS_H__

typedef struct {
    char *private_key;
    char *public_key;
} gk_ssh_memkey_t;

//gk_ssh_memkey_t *gk_ssh_memkey(char *private_key, char *public_key);
//gk_ssh_memkey_t *gk_ssh_memkey_free(char *private_key, char *public_key);

#endif // __GITKEBAB_SSH_KEYS_H__
