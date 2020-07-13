#include "token.h"

#define KEY_SIZE 2048

token_t * new_token(const char * value) {
	token_t * t = malloc(sizeof(token_t));
	if (value != NULL)
		t->value = value;
	return t;
}

void free_token(token_t * t) {
	if (t == NULL)
		return;
	if (t->value != NULL)
		free((char *)t->value);
	free(t);
}

token_t * new_token_from_json(json_t * j, char * type) {
	json_t * token_obj = json_object_get(j, "token");
	json_t * expires_at_obj = json_object_get(j, "expires_at");
    token_t * token = new_token(json_string_value(token_obj));
    token->expires_at = str2time(json_string_value(expires_at_obj));
	if (type != NULL)
		token->type = type;
	free(token_obj);
	free(expires_at_obj);
	return token;
}

pem_t * new_pem(char * path) {
	pem_t * pem = malloc(sizeof(pem_t));
	pem->key = malloc(sizeof(unsigned char) * KEY_SIZE);
    FILE *fp_priv_key;
	fp_priv_key = fopen(path, "r");
	pem->length = fread(pem->key, 1, KEY_SIZE, fp_priv_key);
	fclose(fp_priv_key);
	pem->key[pem->length] = '\0';
	return pem;
}

void free_pem(pem_t * pem) {
	if (pem == NULL)
		return;
	if (pem->key != NULL)
		free(pem->key);
	free(pem);
}

token_t * create_jwt(char * pem_path, char * app_id) {
	// token to return
	token_t * t = new_token(NULL);
	// setup
    jwt_alg_t alg = JWT_ALG_RS256;
    jwt_t * jwt;
    time_t iat = time(NULL);
    time_t exp = iat + 10; // jwt good for 10 seconds
	// create jwt
	if (jwt_new(&jwt) != 0 || jwt == NULL)
		return NULL;
	// read pem file
	pem_t * pem = new_pem(pem_path);
	// basic grants
	jwt_add_grant_int(jwt, "exp", exp);
	jwt_add_grant_int(jwt, "iat", iat);
	jwt_add_grant(jwt, "iss", app_id);
	// set algorithm and sign key
	jwt_set_alg(jwt, alg, pem->key, pem->length);
	// encode and set token
	char *out = jwt_encode_str(jwt);
	t->expires_at = exp;
	t->value = strdup(out);
	t->type = "Bearer";
	// free
    jwt_free(jwt);
	free_pem(pem);
	free(out);
    return t;
}
