#include <3ds.h>
#include <stdio.h>
#include <string.h>

#include "network.h"
#include "message.h"
#include "input.h"

typedef struct sString {
    char* buffer;
    size_t size;
} string_t;

static httpcContext context;
static u32 responsecode = 0;
static string_t responsebody = { 0 };
static char cookie[100] = { 0 };

static int writeCallback(string_t* out) {
    u32 readsize;
    Result ret;
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
    httpcCloseContext(&context);
    httpcExit();
}

void httpStartConnection(const char* url) {
    httpcOpenContext(&context, HTTPC_METHOD_POST, url, 0);
}

void httpAddRequestHeader(const char* name, const char* value) {
    httpcAddRequestHeaderField(&context, name, value);
}

void httpAddPostFieldText(const char* name, const char* value) {
    httpcAddPostDataAscii(&context, name, value);
}

const char* httpPost() {
    httpcSetKeepAlive(&context, HTTPC_KEEPALIVE_ENABLED);

    httpcAddRequestHeaderField(&context, "Content-Type", "multipart/form-data");
    httpcAddRequestHeaderField(&context, "Accept-Language", getLangString());
    httpcAddRequestHeaderField(&context, "User-Agent", "SokobanMaker");
    httpcAddRequestHeaderField(&context, "Connection", "Keep-Alive");
    httpcAddRequestHeaderField(&context, "Cookie", cookie);

    Result res = httpcBeginRequest(&context);
    if (res != 0) return NULL;

    httpGetResponseHeader("Set-Cookie", cookie, sizeof(cookie));
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

    u8* buf = (u8*)calloc(1, fsize);
    fread(buf, 1, fsize, file);
    fclose(file);

    httpcAddPostDataBinary(&context, "upfile", buf, fsize);
    const char* res = httpPost();

    httpEndConnection();
    free(buf);
    return res;
}
