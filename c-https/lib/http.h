#ifndef HTTP_H
#define HTTP_H

#include <jansson.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>

#include "utils.h"
#include "token.h"

json_t * https(char *, char *, char *, token_t *);

#endif