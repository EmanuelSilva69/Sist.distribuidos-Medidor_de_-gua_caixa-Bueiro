#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

#define DEFAULT_POST_URL "http://interscity-resource-adaptor:3000/resources/f47ac10b-58cc-4372-a567-0e02b2c3d474/data"

static double gerar_nivel_critico(long iteracao) {
    double progresso = (double)(iteracao % 720) / 720.0;
    double base = 55.0 + 45.0 * cos(progresso * 2.0 * M_PI);
    double ruido = ((rand() % 200) / 100.0) - 1.0;
    return base + ruido;
}

// FORMATO IDÊNTICO AO DA OUTRA EQUIPE
static char *montar_payload(double nivel) {
    cJSON *root = cJSON_CreateObject();
    cJSON *data = cJSON_CreateObject();
    cJSON *array_leituras = cJSON_CreateArray();
    cJSON *leitura = cJSON_CreateObject();
    
    // Gera o Timestamp atual no formato ISO 8601 (ex: 2026-06-02T14:30:00Z)
    char timestamp[30];
    time_t now = time(NULL);
    struct tm *tm_info = gmtime(&now);
    strftime(timestamp, 30, "%Y-%m-%dT%H:%M:%SZ", tm_info);

    // Insere o "value" e o "timestamp" como a outra equipe fez
    cJSON_AddNumberToObject(leitura, "value", nivel); 
    cJSON_AddStringToObject(leitura, "timestamp", timestamp);
    
    cJSON_AddItemToArray(array_leituras, leitura);
    cJSON_AddItemToObject(data, "niveldagua", array_leituras);
    cJSON_AddItemToObject(root, "data", data);

    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return json;
}

static void enviar_post(const char *url, const char *payload) {
    CURL *curl = curl_easy_init();
    if (!curl) return;

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
        long status;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        printf("[OK] HTTP %ld\n", status);
    } else {
        fprintf(stderr, "[ERRO] POST: %s\n", curl_easy_strerror(res));
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

int main(void) {
    srand(time(NULL));
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    long iteracao = 0;
    const char *url = getenv("MIDDLEWARE_POST_URL") ? getenv("MIDDLEWARE_POST_URL") : DEFAULT_POST_URL;

    printf("[SIMULADOR] Iniciando. Ciclo de 2h ativo.\n");
    printf("[ALVO] Disparando para o Adaptor: %s\n", url);
    fflush(stdout);

    while(1) {
        double nivel = gerar_nivel_critico(iteracao++);
        char *payload = montar_payload(nivel);
        
        printf("[LEITURA] Iteracao %ld | Nivel: %.2f cm | Payload: %s\n", iteracao, nivel, payload);
        fflush(stdout); 
        
        enviar_post(url, payload);
        fflush(stdout);
        
        free(payload);
        sleep(10); 
    }

    curl_global_cleanup();
    return 0;
}