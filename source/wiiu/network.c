#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wiiu.h"

#include "network.h"
#include "message.h"
#include "input.h"
#include "list.h"

typedef struct sHeader {
    const char* name;
    const char* value;
} header_t;

static CURL* curl = NULL;
static struct curl_httppost* form = NULL;
static struct curl_httppost* last = NULL;
static struct curl_slist* headers = NULL;
static int responsecode = 0;
static string_t responsebody = { 0 };
static string_t rawheaders = { 0 };
static list_t responseheaders = { NULL, NULL, 0 };
static char cookie[100] = "";

static int writeCallback(void* data, int size, int nmemb, void* userp) {
    string_t* out = (string_t*)userp;
    int sz = size * nmemb;

    if (!out->buffer) {
        out->buffer = (char*)malloc(sz + 1);
    } else {
        char* tmp = realloc(out->buffer, out->size + sz + 1);
        if (!tmp) {
            free(out->buffer);
            out->buffer = NULL;
        } else {
            out->buffer = tmp;
        }
    }

    if (!out->buffer) {
        out->size = 0;
        return -1;
    }

    memcpy(out->buffer + out->size, data, sz);
    out->size += sz;
    out->buffer[out->size] = 0;

    return sz;
}

static void httpParseHeaders() {
    char* tok = strtok(rawheaders.buffer, ":");
    while(tok != NULL) {
        header_t* h = (header_t*)calloc(1, sizeof(header_t));
        if (!h) break;
        h->name = tok;
        tok = strtok(NULL, "\n");
        if (!tok) break;
        h->value = tok;
        tok = strtok(NULL, ":");
        insertAt(&responseheaders, -1, h);
    }
}

static int httpFindHeader(const void* header, const void* name) {
    return (strcmp(((const header_t*)header)->name, (const char*)name)==0);
}

void networkInit() {
    //curl = n_curl_easy_init();
}

void networkFini() {
    free(responsebody.buffer);
    free(rawheaders.buffer);
    clearList(&responseheaders);
    if (curl) n_curl_easy_cleanup(curl);
}

void httpStartConnection(const char* url) {
    free(responsebody.buffer);
    free(rawheaders.buffer);
    memset(&rawheaders, 0, sizeof(rawheaders));
    memset(&responsebody, 0, sizeof(responsebody));
    clearList(&responseheaders);
    //n_curl_easy_reset(curl);
    curl = n_curl_easy_init();
    n_curl_easy_setopt(curl, CURLOPT_URL, url);
}

void httpAddRequestHeader(const char* name, const char* value) {
    char buf[256];
    sprintf(buf, "%s: %s", name, value);
    headers = n_curl_slist_append(headers, buf);
}

void httpAddPostFieldText(const char* name, const char* value) {
    n_curl_formadd(&form, &last, CURLFORM_COPYNAME, name, CURLFORM_COPYCONTENTS, value, CURLFORM_END);
}

const char* httpPost() {
    if (!curl) return NULL;

    httpAddRequestHeader("Accept-Language", getLangString());

    n_curl_easy_setopt(curl, CURLOPT_USERAGENT, "SokobanMaker");
    n_curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1);
    n_curl_easy_setopt(curl, CURLOPT_COOKIE, cookie);
    n_curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    n_curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responsebody);
    n_curl_easy_setopt(curl, CURLOPT_HEADERDATA, &rawheaders);
    n_curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    n_curl_easy_setopt(curl, CURLOPT_HTTPPOST, form);

    CURLcode res = n_curl_easy_perform(curl);
    if (res != CURLE_OK) return NULL;

    n_curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responsecode);

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
        n_curl_formfree(form);
        form = last = NULL;
    }
    if (headers) {
        n_curl_slist_free_all(headers);
        headers = NULL;
    }
    if (curl) {
        n_curl_easy_cleanup(curl);
        curl = NULL;
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

    n_curl_easy_setopt(curl, CURLOPT_USERAGENT, "SokobanMaker");
    n_curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1);
    n_curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
    n_curl_easy_setopt(curl, CURLOPT_WRITEDATA, out);

    n_curl_easy_perform(curl);
    fclose(out);

    n_curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responsecode);
    if (responsecode != 200) {
        remove(path);
    }
    httpEndConnection();
    return (responsecode == 200);
}

const char* uploadFile(const char* url, const char* path) {
    httpStartConnection(url);

    FILE* file = fopen(path, "rb");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    size_t fsize = ftell(file);
    rewind(file);

    u8* buf = (u8*)calloc(1, fsize);
    if (!buf) return NULL;

    fread(buf, 1, fsize, file);
    fclose(file);

    n_curl_formadd(&form, &last,
        CURLFORM_COPYNAME, "upfile",
        CURLFORM_FILENAME, "mylevel.lvl",
        CURLFORM_BUFFER, "mylevel.lvl",
        CURLFORM_BUFFERPTR, buf,
        CURLFORM_BUFFERLENGTH, fsize,
        CURLFORM_CONTENTTYPE, "multipart/form-data",
        CURLFORM_END
    );
    const char* res = httpPost();

    httpEndConnection();
    free(buf);
    return res;
}
