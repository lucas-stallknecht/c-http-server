#include "animal_controller.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int _read_html_into_buffer(const char* filename, char** buffer, size_t* size) {
    FILE* file = fopen(filename, "rb");
    if (!file)
        return -1;

    // Calculate file size
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    // Allocate read buffer
    char* temp = (char*)malloc(file_size + 1);
    if (!temp) {
        fclose(file);
        return -2;
    }

    size_t read_size = fread(temp, 1, file_size, file);
    fclose(file);

    if (read_size != file_size) {
        free(temp);
        return -3;
    }

    temp[file_size] = '\0';

    *buffer = temp;
    *size = file_size;

    return 0;
}

void index_func(char** buffer, size_t* size) {
    _read_html_into_buffer("pages/index.html", buffer, size);
}

void dog_func(char** buffer, size_t* size) {
    _read_html_into_buffer("pages/dog.html", buffer, size);
}
void cat_func(char** buffer, size_t* size) {
    _read_html_into_buffer("pages/cat.html", buffer, size);
}
