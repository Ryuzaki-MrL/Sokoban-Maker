#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "network.h"
#include "message.h"
#include "input.h"
#include "list.h"

typedef struct sHeader {
    const char* name;
    const char* value;
} header_t;

static CURL* curl = NULL;
static curl_mime* form = NULL;
static curl_mimepart* field = NULL;
static struct curl_slist* headers = NULL;
static int responsecode = 0;
static string_t responsebody = { 0 };
static string_t rawheaders = { 0 };
static list_t responseheaders = { NULL, NULL, 0 };
static char cookie[100] = "";

static size_t writeCallback(void* data, size_t size, size_t nmemb, string_t* out) {
    size_t sz = size * nmemb;

    out->buffer = realloc(out->buffer, out->size + sz + 1);
    if (!out->buffer) {
        printf("Out of memory.\n");
        return 0;
    }

    memcpy(out->buffer + out->size, data, sz);
    out->size += sz;
    out->buffer[out->size] = 0;

    return sz;
}

static size_t writeCallbackFile(void* data, size_t size, size_t nmemb, FILE* out) {
    return fwrite(data, size, nmemb, out);
}

static void httpParseHeaders() {
    char* tok = strtok(rawheaders.buffer, ":");
    while(tok != NULL) {
        header_t* h = (header_t*)calloc(1, sizeof(header_t));
        if (!h) break;
        h->name = tok;
        tok = strtok(NULL, "\n");
        h->value = tok;
        tok = strtok(NULL, ":");
        insertAt(&responseheaders, -1, h);
    }
}

static int httpFindHeader(const void* header, const void* name) {
    return (strcmp(((const header_t*)header)->name, (const char*)name)==0);
}

void networkInit() {
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
}

void networkFini() {
    free(responsebody.buffer);
    free(rawheaders.buffer);
    clearList(&responseheaders);
    if (curl) curl_easy_cleanup(curl);
    curl_global_cleanup();
}

void httpStartConnection(const char* url) {
    free(responsebody.buffer);
    free(rawheaders.buffer);
    memset(&rawheaders, 0, sizeof(rawheaders));
    memset(&responsebody, 0, sizeof(responsebody));
    clearList(&responseheaders);
    curl_easy_reset(curl);
    form = curl_mime_init(curl);
    curl_easy_setopt(curl, CURLOPT_URL, url);
}

void httpAddRequestHeader(const char* name, const char* value) {
    char buf[256];
    sprintf(buf, "%s: %s", name, value);
    headers = curl_slist_append(headers, buf);
}

void httpAddPostFieldText(const char* name, const char* value) {
    field = curl_mime_addpart(form);
    curl_mime_name(field, name);
    curl_mime_data(field, value, CURL_ZERO_TERMINATED);
}

const char* httpPost() {
    if (!curl) return NULL;

    httpAddRequestHeader("Accept-Language", getLangString());

    curl_easy_setopt(curl, CURLOPT_USERAGENT, "SokobanMaker");
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1);
    curl_easy_setopt(curl, CURLOPT_COOKIE, cookie);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responsebody);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &rawheaders);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) return NULL;

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responsecode);

    httpParseHeaders();
    httpGetResponseHeader("Set-Cookie", cookie, sizeof(cookie));

    return responsebody.buffer;
}

char* httpGetResponseHeader(const char* name, char* value, size_t len) {
    header_t* found = (header_t*)search(&responseheaders, httpFindHeader, name);
    if (found && found->value) {
        strncpy(value, found->value+1, len-1);
        return value;
    }
    return NULL;
}

int httpGetResponseCode() {
    return responsecode;
}

void httpEndConnection() {
    if (form) {
        curl_mime_free(form);
        form = NULL;
    }
    if (headers) {
        curl_slist_free_all(headers);
        headers = NULL;
    }
}

const char* httpGet(const char* url) {
    httpStartConnection(url);
    const char* ret = httpPost();
    httpEndConnection();
    return ret;
}

int downloadFile(const char* url, const char* path) {
    httpStartConnection(url);
    if (!curl) {
        httpEndConnection();
        return 0;
    }

    FILE* out = fopen(path, "wb");
    if (!out) {
        httpEndConnection();
        return 0;
    }

    curl_easy_setopt(curl, CURLOPT_USERAGENT, "SokobanMaker");
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, out);

    curl_easy_perform(curl);
    fclose(out);

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responsecode);
    if (responsecode != 200) {
        remove(path);
    }
    httpEndConnection();
    return (responsecode == 200);
}

const char* uploadFile(const char* url, const char* path) {
    httpStartConnection(url);

    field = curl_mime_addpart(form);
    curl_mime_name(field, "upfile");
    curl_mime_filedata(field, path);

    const char* res = httpPost();
    httpEndConnection();
    return res;
}
