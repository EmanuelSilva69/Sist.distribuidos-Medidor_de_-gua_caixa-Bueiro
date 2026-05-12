#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include "cJSON.h" 

// Fallbacks caso as variáveis de ambiente não existam
#define DEFAULT_MIDDLEWARE "https://cidadesinteligentes.lsdi.ufma.br/interscity_lh/collector/resources/f0877867-005f-407f-810b-560cd726d5a2/data/last"
#define DEFAULT_EVOLUTION  "http://evolution-emanuel:8080/message/sendText/reservatorio-slz"
#define DEFAULT_JID        "559882777091@s.whatsapp.net" // JID padrão com sufixo
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
    
    // Busca variáveis de base do ambiente
    char *base_url = getenv("EVOLUTION_URL") ? getenv("EVOLUTION_URL") : "http://evolution-reservatorio:8080";
    char *instance = getenv("INSTANCE_NAME") ? getenv("INSTANCE_NAME") : "reservatorio-slz";
    char *api_key = getenv("EVOLUTION_API_KEY") ? getenv("EVOLUTION_API_KEY") : "monitor_hidrico_ufma_2026";
    char *gestor_jid = getenv("GESTOR_JID") ? getenv("GESTOR_JID") : DEFAULT_JID;

    // MONTA A URL DINAMICAMENTE: base + endpoint + instancia
    char full_url[256];
    snprintf(full_url, sizeof(full_url), "%s/message/sendText/%s", base_url, instance);

    curl = curl_easy_init();
    if(curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        
        char auth_header[150];
        snprintf(auth_header, sizeof(auth_header), "apikey: %s", api_key);
        headers = curl_slist_append(headers, auth_header);

        cJSON *root = cJSON_CreateObject();
        cJSON_AddStringToObject(root, "number", gestor_jid);
        
        char msg[200];
        snprintf(msg, sizeof(msg), "⚠️ *ALERTA HÍDRICO UFMA*\nNível crítico: %.2f cm.\nVerifique o reservatório!", nivel);
        cJSON_AddStringToObject(root, "text", msg);
        
        char *json_out = cJSON_Print(root);

        // Usamos a URL completa montada acima
        curl_easy_setopt(curl, CURLOPT_URL, full_url);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_out);

        res = curl_easy_perform(curl);
        if(res == CURLE_OK) {
            printf("[NOTIFICACAO] Alerta enviado com sucesso para %s\n", gestor_jid);
        } else {
            fprintf(stderr, "[ERRO] Falha na Evolution API: %s\n", curl_easy_strerror(res));
        }
        fflush(stdout); 

        cJSON_Delete(root);
        free(json_out);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}

int main(void) {
    // Desativa o buffer para logs instantâneos no Docker
    setvbuf(stdout, NULL, _IONBF, 0); 

    printf("[BOT C] Iniciando monitoramento multinível...\n"); 
    
    // Lista de URLs dos sensores (Antigo e Novo)
    char *urls[] = {
        "https://cidadesinteligentes.lsdi.ufma.br/interscity_lh/collector/resources/f0877867-005f-407f-810b-560cd726d5a2/data/last",
        "https://cidadesinteligentes.lsdi.ufma.br/interscity_lh/collector/resources/39554372-31e8-47dd-8bdd-c23ea4d6e1c8/data/last"
    };
    int total_sensores = 2;

    while(1) {
        for(int i = 0; i < total_sensores; i++) {
            printf("\n[SENSOR %d] Verificando: %s\n", i + 1, urls[i]);
            
            CURL *curl;
            struct MemoryStruct chunk = {malloc(1), 0};

            curl = curl_easy_init();
            if(curl) {
                curl_easy_setopt(curl, CURLOPT_URL, urls[i]);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

                // Executa a requisição para o sensor atual
                if(curl_easy_perform(curl) == CURLE_OK) {
                    cJSON *json = cJSON_Parse(chunk.memory);
                    if (json) {
                        cJSON *data_obj = cJSON_GetObjectItemCaseSensitive(json, "data");
                        if (data_obj) {
                            cJSON *nivel_item = cJSON_GetObjectItemCaseSensitive(data_obj, "niveldagua");
                            
                            if (cJSON_IsNumber(nivel_item)) {
                                double nivel = nivel_item->valuedouble;
                                printf("Leitura Sensor %d: %.2f cm\n", i + 1, nivel);

                                if(nivel < NIVEL_CRITICO) {
                                    printf("[ALERTA] Sensor %d crítico! Enviando WhatsApp...\n", i + 1);
                                    enviar_alerta_whatsapp(nivel);
                                }
                            }
                        }
                        cJSON_Delete(json);
                    }
                } else {
                    printf("[ERRO] Falha ao conectar no sensor %d.\n", i + 1);
                }
                curl_easy_cleanup(curl);
                free(chunk.memory);
            }
            fflush(stdout); 
        }

        printf("\n[AGUARDANDO] Próximo ciclo em 30 segundos...\n");
        sleep(30); 
    }
    return 0;
}