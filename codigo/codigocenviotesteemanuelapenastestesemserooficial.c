#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"

// Endpoint oficial do Collector do LSDI/UFMA com seu UUID
#define COLLECTOR_URL "https://cidadesinteligentes.lsdi.ufma.br/interscity_lh/collector/resources/f0877867-005f-407f-810b-560cd726d5a2/data"

int main(void) {
    CURL *curl;
    CURLcode res;

    // Inicializa o cURL
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if(curl) {
        // 1. Construção do JSON seguindo o padrão InterSCity
        // Formato esperado: {"data": {"niveldagua": valor}}
        cJSON *root = cJSON_CreateObject();
        cJSON *data = cJSON_CreateObject();
        
        cJSON_AddNumberToObject(data, "niveldagua", 145.18); // Valor simulado
        cJSON_AddItemToObject(root, "data", data);
        
        char *json_string = cJSON_Print(root);

        // 2. Configuração do Header
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        // 3. Configuração da Requisição POST
        curl_easy_setopt(curl, CURLOPT_URL, COLLECTOR_URL);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Ignora verificação SSL (útil para ambiente acadêmico da UFMA)
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

        printf("[SIMULADOR] Enviando dados para o InterSCity UFMA...\n");
        printf("[PAYLOAD] %s\n", json_string);

        // 4. Execução
        res = curl_easy_perform(curl);

        if(res != CURLE_OK) {
            fprintf(stderr, "[ERRO] Falha no POST: %s\n", curl_easy_strerror(res));
        } else {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            printf("[SUCESSO] Código de Resposta: %ld\n", response_code);
            printf("[STATUS] Dado persistido no UUID: f0877867-005f-407f-810b-560cd726d5a2\n");
        }

        // Limpeza
        cJSON_Delete(root);
        free(json_string);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return 0;
}
