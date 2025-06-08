#ifndef SMALL_CONTROLLER_H
#define SMALL_CONTROLLER_H

#include <stddef.h>

int _read_html_into_buffer(const char* file_path, char** buffer, size_t* size);

void index_func(char** buffer, size_t* size);
void dog_func(char** buffer, size_t* size);
void cat_func(char** buffer, size_t* size);
void not_found_func(char** buffer, size_t* size);

#endif // SMALL_CONTROLLER_H
