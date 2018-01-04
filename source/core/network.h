#ifndef NETWORK_H
#define NETWORK_H

#include <stdlib.h>

#ifdef DEBUG
    #define URL_ROOT    "http://192.168.0.17/sokomaker/"
#else
    #define URL_ROOT    "http://sokomaker.000webhostapp.com/"
#endif

void networkInit();
void networkFini();

void httpStartConnection(const char* url);
void httpAddRequestHeader(const char* name, const char* value);
void httpAddPostFieldText(const char* name, const char* value);
char* httpGetResponseHeader(const char* name, char* value, size_t len);
int httpGetResponseCode();
const char* httpPost();
void httpEndConnection();

const char* httpGet(const char* url);

int downloadFile(const char* url, const char* path);
const char* uploadFile(const char* url, const char* path);

#endif // NETWORK_H