#include "http.h"


// slurp the http response chunks into a single string
char * slurp_http_response(BIO * bio) {
    int size;
    size_t current_size = 1024;
    char buf[current_size];
    char * response = malloc(current_size);
    for(;;)
    {
        size = BIO_read(bio, buf, 1023);
        if(size <= 0)
        {
            break;
        }
        buf[size] = 0;
        if (strlen(response) + size >= current_size) {
            current_size *= 2;
            response = realloc(response, current_size);
        }
        strcat(response, buf);
    }
    return response;
}

// extact the json body from an http response string
json_t * extract_json_body(BIO * bio) {
    // slurp response
    char * response_str = slurp_http_response(bio);
    // check response body line-by-line
    char * json_str = strtok(response_str, "\r\n");
    while( json_str != NULL )
        if (starts_with_any(json_str=strtok(NULL, "\r\n"), "{["))
            break;
    // return parse json object
    json_error_t jerror;
    json_t *ret = json_loads(json_str, JSON_DISABLE_EOF_CHECK | JSON_ALLOW_NUL, &jerror);
    if (ret == NULL)
        printf("error: %d\n", jerror.position);
    // free the response
    free(response_str);
    return ret;
}

json_t * https(char * method, char * host, char * endpoint, token_t * jwt_token) {
    BIO* bio;
    SSL* ssl;
    SSL_CTX* ctx;
    SSL_library_init();
    if ((ctx = SSL_CTX_new(SSLv23_client_method())) == NULL)
    {
        printf("Ctx is null\n");
    }

    bio = BIO_new_ssl_connect(ctx);
    char hostname[255];
    sprintf(hostname, "%s:443", host);
    BIO_set_conn_hostname(bio, hostname);
    if(BIO_do_connect(bio) <= 0)
    {
        printf("Failed connection\n");
        return NULL;
    }

    char* data = "";
    char write_buf[1024];
    sprintf(write_buf, "%s %s HTTP/1.1\r\n"
                        "Host: %s\r\n"
                        "Authorization: %s %s\r\n"
                        "Accept: application/vnd.github.machine-man-preview+json\r\n"
                        "User-Agent: github-token-refresh\r\n"
                        "Connection: close\r\n"
                        "\r\n", method, endpoint, host, jwt_token->type, jwt_token->value);
    if(BIO_write(bio, write_buf, strlen(write_buf)) <= 0)
    {
        printf("Failed write\n");
    }

    json_t * ret = extract_json_body(bio);
    BIO_free_all(bio);
    SSL_CTX_free(ctx);
    return ret;
}

