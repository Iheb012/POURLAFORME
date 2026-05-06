#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h>

// Structure to handle the API response
struct string {
    char *ptr;
    size_t len;
};

// Initialize the response buffer
void init_string(struct string *s) {
    s->len = 0;
    s->ptr = malloc(1);
    s->ptr[0] = '\0';
}

// Callback for writing API response
size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s) {
    size_t new_len = s->len + size * nmemb;
    s->ptr = realloc(s->ptr, new_len + 1);
    memcpy(s->ptr + s->len, ptr, size * nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;
    return size * nmemb;
}

int main() {
    CURL *curl;
    CURLcode res;
    struct string response;
    char user_input[512];
    char api_key[128];

    // ✅ Read API key from file (key.txt)
    FILE *keyfile = fopen("key.txt", "r");
    if (!keyfile) {
        fprintf(stderr, "❌ Error: could not open key.txt\n");
        fprintf(stderr, "Please create a file named key.txt containing your Groq API key (starts with gsk_)\n");
        return 1;
    }
    fgets(api_key, sizeof(api_key), keyfile);
    api_key[strcspn(api_key, "\n")] = 0; // remove newline
    fclose(keyfile);

    printf("🤖 Midou : Hello! I’m Midou, your AI Gym  assistant.\n");
    printf("Type 'bye' to exit.\n");

    while (1) {
        printf("\nYou: ");
        fflush(stdout);

        if (!fgets(user_input, sizeof(user_input), stdin))
            break;
        user_input[strcspn(user_input, "\n")] = 0;

        if (strcmp(user_input, "bye") == 0)
            break;

        curl = curl_easy_init();
        if (curl) {
            init_string(&response);

            // ✅ Correct Groq model (live and supported)
            char json_data[1024];
            snprintf(json_data, sizeof(json_data),
                     "{\"model\":\"llama-3.1-8b-instant\","
                     "\"messages\":[{\"role\":\"user\",\"content\":\"%s\"}]}",
                     user_input);

            struct curl_slist *headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");

            char auth_header[256];
            snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", api_key);
            headers = curl_slist_append(headers, auth_header);

            curl_easy_setopt(curl, CURLOPT_URL, "https://api.groq.com/openai/v1/chat/completions");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

            res = curl_easy_perform(curl);

            if (res == CURLE_OK) {
                json_t *root, *choices, *message, *content;
                json_error_t error;

                root = json_loads(response.ptr, 0, &error);
                if (root) {
                    choices = json_object_get(root, "choices");
                    if (json_is_array(choices) && json_array_size(choices) > 0) {
                        message = json_object_get(json_array_get(choices, 0), "message");
                        content = json_object_get(message, "content");
                        if (json_is_string(content)) {
                            printf("Midou: %s\n", json_string_value(content));
                        } else {
                            printf("⚠️ No message content found.\n");
                        }
                    } else {
                        printf("⚠️ Invalid or empty response from API.\n");
                    }
                    json_decref(root);
                } else {
                    printf("⚠️ Could not parse API response.\n");
                }
            } else {
                printf("⚠️ Request failed: %s\n", curl_easy_strerror(res));
            }

            free(response.ptr);
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
        }
    }

    printf("👋 Midou: Goodbye!\n");
    return 0;
}