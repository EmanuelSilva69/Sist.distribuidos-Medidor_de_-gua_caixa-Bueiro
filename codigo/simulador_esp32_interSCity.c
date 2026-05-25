#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

#define DEFAULT_POST_URL "http://servidor-mentira:5000/interscity_lh/collector/resources/f0877867-005f-407f-810b-560cd726d5a2/data"
#define SENSOR_UUID "f0877867-005f-407f-810b-560cd726d5a2"

static double clamp(double value, double min_value, double max_value) {
    if (value < min_value) return min_value;
    if (value > max_value) return max_value;
    return value;
}

static double gerar_leitura(void) {
    static double fase = 0.0;
    double base = 90.0 + 60.0 * sin(fase);
    double ruido = ((rand() % 1000) / 1000.0) * 6.0 - 3.0;
    fase += 0.28;
    if (fase > 2.0 * M_PI) {
        fase -= 2.0 * M_PI;
    }
    return clamp(base + ruido, 5.0, 180.0);
}

static char *montar_payload(double nivel) {
    cJSON *root = cJSON_CreateObject();
    if (!root) return NULL;

    cJSON *data = cJSON_CreateObject();
    if (!data) {
        cJSON_Delete(root);
        return NULL;
    }

    cJSON_AddNumberToObject(data, "niveldagua", nivel);
    cJSON_AddItemToObject(root, "data", data);

    char *json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return json;
}

static int enviar_post(const char *url, const char *payload) {
    CURL *curl = curl_easy_init();
    if (!curl) return -1;

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

    CURLcode res = curl_easy_perform(curl);
    long status = 0;
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        printf("[ESP32 MOCK] HTTP %ld\n", status);
    } else {
        fprintf(stderr, "[ESP32 MOCK] Falha ao enviar: %s\n", curl_easy_strerror(res));
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return (res == CURLE_OK) ? (int)status : -1;
}

int main(void) {
    srand((unsigned int)time(NULL));
    curl_global_init(CURL_GLOBAL_DEFAULT);

    const char *url = getenv("MIDDLEWARE_URL");
    if (!url || !*url) {
        url = DEFAULT_POST_URL;
    }

    printf("[ESP32 MOCK] Iniciando simulacao para %s\n", url);

    for (;;) {
        double nivel = gerar_leitura();
        char *payload = montar_payload(nivel);
        if (!payload) {
            fprintf(stderr, "[ESP32 MOCK] Falha ao montar JSON\n");
            break;
        }

        printf("[ESP32 MOCK] Nivel gerado: %.2f cm\n", nivel);
        printf("[ESP32 MOCK] Payload: %s\n", payload);
        enviar_post(url, payload);
        free(payload);

        sleep(10);
    }

    curl_global_cleanup();
    return 0;
}
