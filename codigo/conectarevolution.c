#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"
#include <qrencode.h> // Biblioteca para gerar o QR no terminal

char *api_url = getenv("EVOLUTION_URL") ? getenv("EVOLUTION_URL") : "http://evolution-emanuel:8080";
char *api_key = getenv("EVOLUTION_API_KEY");
char *instance = getenv("INSTANCE_NAME") ? getenv("INSTANCE_NAME") : "reservatorio-slz";

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) return 0;
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    return realsize;
}

// Função para printar o QR no terminal usando caracteres de bloco
void print_qr_terminal(const char *text) {
    QRcode *qrcode = QRcode_encodeString(text, 0, QR_ECLEVEL_L, QR_MODE_8, 1);
    if (qrcode == NULL) return;

    printf("\n--- ESCANEIE O QR CODE ABAIXO PARA CONECTAR ---\n\n");
    for (int y = 0; y < qrcode->width; y++) {
        for (int x = 0; x < qrcode->width; x++) {
            // Usa blocos Unicode para formar a imagem [cite: 87]
            if (qrcode->data[y * qrcode->width + x] & 1) {
                printf("\u2588\u2588"); 
            } else {
                printf("  ");
            }
        }
        printf("\n");
    }
    printf("\n-----------------------------------------------\n");
    QRcode_free(qrcode);
}

void conectar_instancia() {
    CURL *curl = curl_easy_init();
    struct MemoryStruct chunk = {malloc(1), 0};

    if(curl) {
        struct curl_slist *headers = NULL;
        char auth[100]; sprintf(auth, "apikey: %s", API_KEY);
        headers = curl_slist_append(headers, auth);

        char url[150];
        sprintf(url, "%s/instance/connect/%s", API_URL, INSTANCE);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        if(curl_easy_perform(curl) == CURLE_OK) {
            cJSON *json = cJSON_Parse(chunk.memory);
            if(json) {
                // Pega a string 'code' que é o conteúdo real do QR
                cJSON *code = cJSON_GetObjectItemCaseSensitive(json, "code");
                if(cJSON_IsString(code) && (code->valuestring != NULL)) {
                    print_qr_terminal(code->valuestring);
                } else {
                    printf("[AVISO] Instância já conectada ou erro no servidor.\n");
                }
                cJSON_Delete(json);
            }
        }
        curl_easy_cleanup(curl);
        free(chunk.memory);
    }
}

int main() {
    curl_global_init(CURL_GLOBAL_ALL);
    conectar_instancia();
    curl_global_cleanup();
    return 0;
}
