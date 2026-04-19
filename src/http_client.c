#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "debate.h"

typedef struct {
    char *data;
    size_t size;
} MemoryBuffer;

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t real_size = size * nmemb;
    MemoryBuffer *mem = (MemoryBuffer *)userp;

    char *ptr = realloc(mem->data, mem->size + real_size + 1);
    if (ptr == NULL) {
        return 0;
    }

    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, real_size);
    mem->size += real_size;
    mem->data[mem->size] = '\0';

    return real_size;
}

int ollama_generate(
    const char *model,
    const char *system_prompt,
    const char *user_prompt,
    char *out,
    int out_size
) {
    CURL *curl;
    CURLcode res;

    MemoryBuffer chunk;
    chunk.data = malloc(1);
    chunk.size = 0;

    if (!chunk.data) {
        snprintf(out, out_size, "memory allocation error");
        return -1;
    }

    curl = curl_easy_init();
    if (!curl) {
        free(chunk.data);
        snprintf(out, out_size, "curl init error");
        return -1;
    }

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    char json_body[8192];
    snprintf(
        json_body,
        sizeof(json_body),
        "{"
        "\"model\":\"%s\","
        "\"messages\":["
            "{\"role\":\"system\",\"content\":\"%s\"},"
            "{\"role\":\"user\",\"content\":\"%s\"}"
        "],"
        "\"stream\":false,"
        "\"think\":false"
        "}",
        model,
        system_prompt,
        user_prompt
    );

    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:11434/api/chat");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_body);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        snprintf(out, out_size, "curl error: %s", curl_easy_strerror(res));
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        free(chunk.data);
        return -1;
    }

    strncpy(out, chunk.data, out_size - 1);
    out[out_size - 1] = '\0';

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    free(chunk.data);

    return 0;
}