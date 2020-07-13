#ifndef TOKEN_H
#define TOKEN_H

#include <jansson.h>
#include <jwt.h>
#include <string.h>

#include "utils.h"


typedef struct Token {
    time_t expires_at;
    const char * value;
    char * type;
} token_t;

typedef struct PEM {
    unsigned char * key;
    size_t length;
} pem_t;

token_t * new_token(const char *);
token_t * new_token_from_json(json_t *, char *);
token_t * create_jwt(char *, char *);
void free_token(token_t *);

#endif