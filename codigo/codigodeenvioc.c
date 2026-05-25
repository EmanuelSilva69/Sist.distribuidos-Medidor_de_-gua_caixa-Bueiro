#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include "cJSON.h" 

// Fallbacks caso as variáveis de ambiente não existam
#define DEFAULT_MIDDLEWARE "https://cidadesinteligentes.lsdi.ufma.br/interscity_lh/collector/resources/f0877867-005f-407f-810b-560cd726d5a2/data/last"
#define DEFAULT_EVOLUTION  "http://evolution-emanuel:8080/message/sendText/reservatorio-slz"
#define DEFAULT_JID        "55989XXXXXXXX@s.whatsapp.net" // JID padrão com sufixo
#define NIVEL_CRITICO      15.0 

struct MemoryStruct {
  char *memory;
  size_t size;
};

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
    
    // Busca variáveis de ambiente para a Camada de Notificação
    char *evolution_url = getenv("EVOLUTION_URL") ? getenv("EVOLUTION_URL") : DEFAULT_EVOLUTION;
    char *api_key = getenv("EVOLUTION_API_KEY") ? getenv("EVOLUTION_API_KEY") : "CHAVE_NAO_CONFIGURADA";
    char *gestor_jid = getenv("GESTOR_JID") ? getenv("GESTOR_JID") : DEFAULT_JID;

    curl = curl_easy_init();
    if(curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        
        char auth_header[150];
        snprintf(auth_header, sizeof(auth_header), "apikey: %s", api_key);
        headers = curl_slist_append(headers, auth_header);

        // Construção do JSON usando o JID
        cJSON *root = cJSON_CreateObject();
        // O campo "number" na Evolution API aceita o JID completo
        cJSON_AddStringToObject(root, "number", gestor_jid);
        
        char msg[200];
        snprintf(msg, sizeof(msg), "⚠️ *ALERTA HÍDRICO UFMA*\nNível crítico detectado: %.2f cm.\nVerifique o sistema.", nivel);
        cJSON_AddStringToObject(root, "text", msg);
        
        char *json_out = cJSON_Print(root);

        curl_easy_setopt(curl, CURLOPT_URL, evolution_url);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_out);

        res = curl_easy_perform(curl);
        if(res == CURLE_OK) {
            printf("[NOTIFICACAO] Alerta enviado para o JID: %s\n", gestor_jid);
        } else {
            fprintf(stderr, "[ERRO] Falha no envio: %s\n", curl_easy_strerror(res));
        }

        cJSON_Delete(root);
        free(json_out);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}

int main(void) {
    printf("[BOT C] Iniciando monitoramento da Camada de Notificação (v2 - JID Logic)...\n"); 
    
    // Captura a URL do middleware uma única vez ou por loop se preferir
    char *middleware_url = getenv("MIDDLEWARE_URL") ? getenv("MIDDLEWARE_URL") : DEFAULT_MIDDLEWARE;

    while(1) {
        CURL *curl;
        struct MemoryStruct chunk = {malloc(1), 0};

        curl = curl_easy_init();
        if(curl) {
            curl_easy_setopt(curl, CURLOPT_URL, middleware_url);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

            if(curl_easy_perform(curl) == CURLE_OK) {
                cJSON *json = cJSON_Parse(chunk.memory);
                if (json) {
                    // Assume que o Middleware retorna um JSON com a chave "nivel"
                    cJSON *nivel_item = cJSON_GetObjectItemCaseSensitive(json, "nivel");
                    if (cJSON_IsNumber(nivel_item)) {
                        double nivel = nivel_item->valuedouble;
                        printf("Leitura Middleware: %.2f cm\n", nivel); 

                        if(nivel < NIVEL_CRITICO) {
                            enviar_alerta_whatsapp(nivel);
                            sleep(600); // Aguarda 10 min (Anti-Spam conforme metodologia)
                        }
                    }
                    cJSON_Delete(json);
                }
            }
            curl_easy_cleanup(curl);
            free(chunk.memory);
        }
        sleep(30); // Intervalo de polling (Semana 12 do cronograma)
    }
    return 0;
}
