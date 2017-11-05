#include <3ds.h>
#include <stdio.h>
#include <string.h>

#include "network.h"
#include "message.h"
#include "input.h"
#include "util.h"
#include "sha1.h"

typedef struct sString {
    char* buffer;
    size_t size;
} string_t;

static httpcContext context;
static u32 responsecode = 0;
static string_t requestbody = { 0 };
static string_t responsebody = { 0 };
static char cookie[100] = "";
static char boundary[50] = "----";

static int writeCallback(string_t* out) {
    u32 readsize;
    Result ret;
    if (!out->buffer) {
        out->buffer = malloc(0x1000);
    }
    do {
        ret = httpcDownloadData(&context, (u8*)out->buffer + out->size, 0x1000, &readsize);
        out->size += readsize;
        if (ret == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING) {
            char* lastbuf = out->buffer;
            out->buffer = realloc(out->buffer, out->size + 0x1000);
            if (!out->buffer) {
                free(lastbuf);
                return 0;
            }
        }
    } while (ret == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING);
    return 1;
}

static int writeCallbackFile(FILE* out) {
    //u32 len;
    //httpcGetDownloadSizeState(&context, NULL, &len);
    u32 readsize;
    u8 buf[0x1000];
    Result ret;
    do {
        ret = httpcDownloadData(&context, buf, 0x1000, &readsize);
        fwrite(buf, 1, readsize, out);
    } while (ret == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING);
    return 1;
}

void networkInit() {
    httpcInit(0x400000);
}

void networkFini() {
    free(requestbody.buffer);
    free(responsebody.buffer);
    httpcCloseContext(&context);
    httpcExit();
}

void httpStartConnection(const char* url) {
    free(requestbody.buffer);
    free(responsebody.buffer);
    memset(&requestbody, 0, sizeof(requestbody));
    memset(&responsebody, 0, sizeof(responsebody));
    requestbody.buffer = calloc(1, 0x400);
    u32 curtime = osGetTime();
    SHA1(boundary+4, (const char*)&curtime, sizeof(u32));
    strhex(boundary+4, boundary+4, 20);
    httpcOpenContext(&context, HTTPC_METHOD_POST, url, 0);
}

void httpAddRequestHeader(const char* name, const char* value) {
    httpcAddRequestHeaderField(&context, name, value);
}

void httpAddPostFieldText(const char* name, const char* value) {
    snprintf(
        requestbody.buffer + requestbody.size, 0x3FF - requestbody.size,
        "%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",
        boundary, name, value
    );
    requestbody.size += strlen(requestbody.buffer + requestbody.size);
}

const char* httpPost() {
    httpcSetKeepAlive(&context, HTTPC_KEEPALIVE_ENABLED);

    char tmp[81];
    snprintf(tmp, sizeof(tmp)-1, "multipart/form-data, boundary=%s", boundary+2);

    httpcAddRequestHeaderField(&context, "Content-Type", tmp);
    httpcAddRequestHeaderField(&context, "Accept-Language", getLangString());
    httpcAddRequestHeaderField(&context, "User-Agent", "SokobanMaker");
    httpcAddRequestHeaderField(&context, "Connection", "Keep-Alive");
    httpcAddRequestHeaderField(&context, "Cookie", cookie);

    char* buf = requestbody.buffer + requestbody.size;
    sprintf(buf, "%s--\r\n", boundary);
    httpcAddPostDataRaw(&context, (const u32*)requestbody.buffer, strlen(buf) + requestbody.size);

    Result res = httpcBeginRequest(&context);
    if (res != 0) return NULL;

    httpcGetResponseHeader(&context, "Set-Cookie", tmp, sizeof(tmp));
    if (tmp[0]) strcpy(cookie, tmp);
    httpcGetResponseStatusCode(&context, &responsecode);

    if (writeCallback(&responsebody))
        return responsebody.buffer;
    return NULL;
}

char* httpGetResponseHeader(const char* name, char* value, size_t len) {
    if (httpcGetResponseHeader(&context, name, value, len) != 0)
        return NULL;
    return value;
}

int httpGetResponseCode() {
    return responsecode;
}

void httpEndConnection() {
    httpcCloseContext(&context);
}

const char* httpGet(const char* url) {
    httpStartConnection(url);
    const char* ret = httpPost();
    httpEndConnection();
    return ret;
}

int downloadFile(const char* url, const char* path) {
    httpStartConnection(url);

    FILE* out = fopen(path, "wb");
    if (!out) {
        httpEndConnection();
        return 0;
    }

    httpcSetKeepAlive(&context, HTTPC_KEEPALIVE_ENABLED);
    httpcAddRequestHeaderField(&context, "User-Agent", "SokobanMaker");
    httpcAddRequestHeaderField(&context, "Connection", "Keep-Alive");

    Result res = httpcBeginRequest(&context);
    if (res != 0) {
        httpEndConnection();
        return 0;
    }

    httpcGetResponseStatusCode(&context, &responsecode);
    if (responsecode != 200) {
        httpEndConnection();
        return 0;
    }

    writeCallbackFile(out);
    fclose(out);
    httpEndConnection();
    return 1;
}

const char* uploadFile(const char* url, const char* path) {
    httpStartConnection(url);

    FILE* file = fopen(path, "rb");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    size_t fsize = ftell(file);
    rewind(file);

    u8* buf = (u8*)calloc(1, 0x400 + fsize);
    if (!buf) return NULL;

    snprintf((char*)buf, 0x3FF, "%s\r\nContent-Disposition: form-data; name=\"upfile\"; filename=\"upfile\"\r\n\r\n", boundary);
    size_t postsize = strlen((const char*)buf);
    postsize += fread(buf + postsize, 1, fsize, file);
    fclose(file);

    buf[postsize++] = '\r';
    buf[postsize++] = '\n';
    sprintf((char*)buf + postsize, "%s--\r\n", boundary);
    postsize += strlen((const char*)buf + postsize);
    httpcAddPostDataRaw(&context, (const u32*)buf, postsize);
    const char* res = httpPost();

    httpEndConnection();
    free(buf);
    return res;
}
