#include "lib/token.h"
#include "lib/http.h"
#include "lib/utils.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

// configuration
#define ORG "initrode"
#define PEM "private_key.pem"
#define INSTALLATION_ID "10095605"
#define APP_ID "70561"

// constants and keys
#define GET "GET"
#define POST "POST"
#define API_URI "api.github.com"
#define INSTALLATION_AUTH_ENDPOINT "/app/installations/" INSTALLATION_ID "/access_tokens"
#define ORG_REPOS_ENDPOINT "/orgs/" ORG "/repos?type=private"
#define FULL_NAME_KEY "full_name"
#define INSTALLATION_AUTH_TYPE "token"

// messages
#define TOKEN_EXPIRED "- TOKEN STATUS: EXPIRED\n"
#define TOKEN_REFRESHED "- TOKEN STATUS: REFRESHED\n"
#define TOKEN_CURRENT "- TOKEN STATUS: CURRENT\n"
#define TOKEN_EXPIRY "- TOKEN EXPIRY: %s\n"
#define TOKEN_VALUE "- TOKEN: %s******************************\n"
#define TIMESTAMP "- TIMESTAMP: %s\n"
#define REPO_OBJECT "- REPO OBJECT: %s\n\n"
#define TITLE "EXAMPLE AUTOMATED TOKEN REFRESH USING GITHUB APP AND REST API\n"
#define NOTE "ctrl-c to exit; otherwise runs infinitely\n\n"


typedef struct GitHubAPIAuth {
    token_t * installation_token;
} gha_t;

gha_t * new_githubapi_auth(token_t * installation_token) {
    gha_t * gh = malloc(sizeof(gha_t));
    if (installation_token != NULL)
        gh->installation_token = installation_token;
    return gh;
}

void free_githubapi_auth(gha_t * gh) {
    if (gh == NULL)
        return;
    if (gh->installation_token != NULL)
        free_token(gh->installation_token);
    free(gh);
}

void github_app(gha_t * gh) {
    if (gh->installation_token != NULL)
        free(gh->installation_token);
    token_t * jwt_token = create_jwt(PEM, APP_ID);
    json_t * resp = https(POST, API_URI, INSTALLATION_AUTH_ENDPOINT, jwt_token);
    gh->installation_token = new_token_from_json(resp, INSTALLATION_AUTH_TYPE);
    const char *key;
    json_t *value;
    free_token(jwt_token);
}

void auth(gha_t * gh) {
    if (gh->installation_token == NULL || time(NULL) + 60 >= gh->installation_token->expires_at) {
        printf(TOKEN_EXPIRED);
        github_app(gh);
        printf(TOKEN_REFRESHED);
    }
    printf(TOKEN_CURRENT);
    char * expires_at = timestamp(gh->installation_token->expires_at, false);
    printf(TOKEN_EXPIRY, expires_at);
    free(expires_at);
    char token_shortened[10];
    memcpy(token_shortened, gh->installation_token->value, 10);
    printf(TOKEN_VALUE, token_shortened);
}

json_t * org_repos(gha_t * gh) {
    auth(gh);
    json_t * resp;
    resp = https(GET, API_URI, ORG_REPOS_ENDPOINT, gh->installation_token);
    return resp;
}

int main() {
    gha_t * gh = new_githubapi_auth(NULL);
    printf(TITLE);
    printf(NOTE);
    for(int i = 0; i < 12; i++) {
        char * now = timestamp(time(NULL), true);
        printf(TIMESTAMP, now);
        free(now);
        json_t * repos = org_repos(gh);
        size_t index;
        json_t *value;
        json_array_foreach(repos, index, value) {
            json_t * full_name_obj = json_object_get(value, "full_name");
            printf(REPO_OBJECT, json_string_value(full_name_obj));
            json_decref(full_name_obj);
            break;
        }
        json_decref(repos);
        sleep(900); // sleep 15 minutes
    }
    free_githubapi_auth(gh);
    return 0;
}