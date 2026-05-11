#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include "cJSON.h" // Biblioteca para parse de JSON

// Configurações do ambiente Docker (conforme seu docker-compose.yml)
#define MIDDLEWARE_URL "http://interscity-api:8080/resources/RES-SLZ-001/data/last"
#define EVOLUTION_URL  "http://evolution-emanuel:8080/message/sendText/reservatorio-slz"
#define API_KEY        "SUA_CHAVE_API_AQUI" // ${EVOLUTION_API_KEY} (essa eu normalmente crio com o nome do projeto. vou deixar em abertp
#define GESTOR_NUM     "55989XXXXXXXX"      // Número para o alerta (SE vocês quiserem em grupo, eu edito. é só mudar o ID que o whatsapp envia.)
#define NIVEL_CRITICO  15.0                 // 15% definido no plano 

// Estrutura para armazenar a resposta do Middleware
struct MemoryStruct {
  char *memory;
  size_t size;
};

// Callback para o cURL processar a resposta
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(ptr == NULL) return 0;
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
  return realsize;
}

void enviar_alerta_whatsapp(double nivel) {
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if(curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        char auth_header[100];
        sprintf(auth_header, "apikey: %s", API_KEY);
        headers = curl_slist_append(headers, auth_header);

        // Construção do JSON para a Evolution API v2.3.7
        cJSON *root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "number", GESTOR_NUM);
        char msg[150];
        sprintf(msg, "!!ALERTA HÍDRICO: Nível crítico detectado em %.2f cm!!", nivel);
        cJSON_AddStringToObject(root, "text", msg);
        char *json_out = cJSON_Print(root);

        curl_easy_setopt(curl, CURLOPT_URL, EVOLUTION_URL);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_out);

        res = curl_easy_perform(curl);
        if(res == CURLE_OK) printf("[NOTIFICACAO] Alerta enviado para o gestor.\n");

        cJSON_Delete(root);
        free(json_out);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}

int main(void) {
    printf("[BOT C] Iniciando monitoramento da Camada de Notificação...\n"); 
    
    while(1) {
        CURL *curl;
        struct MemoryStruct chunk = {malloc(1), 0};

        curl = curl_easy_init();
        if(curl) {
            curl_easy_setopt(curl, CURLOPT_URL, MIDDLEWARE_URL);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

            if(curl_easy_perform(curl) == CURLE_OK) {
                // Parse dos dados recebidos do Interscity 
                cJSON *json = cJSON_Parse(chunk.memory);
                if (json) {
                    double nivel = cJSON_GetObjectItemCaseSensitive(json, "nivel")->valuedouble;
                    printf("Leitura Middleware: %.2f cm\n", nivel); 

                    if(nivel < NIVEL_CRITICO) {
                        enviar_alerta_whatsapp(nivel);
                        sleep(600); // Aguarda 10 min para evitar spam
                    }
                    cJSON_Delete(json);
                }
            }
            curl_easy_cleanup(curl);
            free(chunk.memory);
        }
        sleep(30); // Intervalo de polling conforme planejamento 
    }
    return 0;
}
