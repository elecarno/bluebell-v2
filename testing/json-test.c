#define CJSON_IMPLEMENTATION
#include "../includes/cJSON.h"
#include <stdio.h>

int main(void) { 
    // open json file
    FILE *fp = fopen("../data/transactions.json", "r");
    if (fp == NULL) {
        printf("Error: Unable to open the file.\n");
        return 1;
    }

    // read file contents into a string
    char buffer[1024];
    int len = fread(buffer, 1, sizeof(buffer), fp);
    fclose(fp);

    // parse JSON data
    cJSON *json = cJSON_Parse(buffer);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            printf("Error: %s\n", error_ptr);
        }
        cJSON_Delete(json);
        return 1;
    }

    // access JSON data
    cJSON *name = cJSON_GetObjectItemCaseSensitive(json, "name");
    if (cJSON_IsString(name) && (name->valuestring != NULL)) {
        printf("Name: %s\n", name->valuestring);
    }

    // delete JSON object
    cJSON_Delete(json);
    return 0;
}