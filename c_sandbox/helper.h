//
//  helper.h
//  c_sandbox
//
//  Created by Daniel Rehman on 1803062.
//  Copyright © 2018 Daniel Rehman. All rights reserved.
//

#ifndef helper_h
#define helper_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef size_t size;
typedef uint_fast32_t integer;
typedef int_fast32_t signed_integer;
typedef integer boolean;

#define true 1
#define false 0
#define helper static inline
#define interface

#define is_in(element, array, length) bsearch(&element, array, length, sizeof(integer), &compare_integers)
#define is_not_in(element, array, length) !bsearch(&element, array, length, sizeof(integer), &compare_integers)

#define i(count) (integer i = 0; i < count; ++i)

// comparison function for integers.
int compare_integers(const void* first, const void* second) {
    return (*(int*)first - *(int*)second);
}

// print function for an array.
interface void print_array(integer *array, integer length) {
    printf("[");
    for i(length) {
        printf("%d", array[i]);
        if (i != length - 1) printf(" ");
    }
    printf("]\n");
}

helper integer count_spaces(char* string) {
    integer count = 0;
    for (integer i = 0; string[i] != '\0'; i++) {
        if (string[i] == ' ') count++;
    }
    return count;
}

helper void char_append(char** array, integer* length, char character) {
    *array = realloc(*array, sizeof(char) * (*length + 1));
    (*array)[(*length)++] = character;
}

helper void integer_append(integer** array, integer* length, integer number) {
    *array = realloc(*array, sizeof(integer) * (*length + 1));
    (*array)[(*length)++] = number;
}

helper void empty_char_array(char** array, integer* length) {
    free(*array);
    *array = NULL;
    *length = 0;
}

// delcare a new array on the stack, using a string initializer.
interface integer* new_array(const char* raw_string, integer* length) {
    
    char* string = malloc(sizeof(char) * (strlen(raw_string) + 1));
    strcpy(string, raw_string);
    
    integer str_len = (integer) strlen(string);
    if (string[str_len-1] != ' ') {
        char_append(&string, &str_len, ' ');
        char_append(&string, &str_len, '\0');
    }
    
    integer* result = malloc(sizeof(integer) * count_spaces(string));
    integer result_count = 0;
    
    char *current_string_number = NULL;
    integer current_string_number_length = 0;
    
    for (integer i = 0; string[i] != '\0'; i++) {
        if (string[i] == ' ') {
            char_append(&current_string_number, &current_string_number_length, '\0');
            
            int new_number = atoi(current_string_number);
            result[result_count++] = new_number;
            empty_char_array(&current_string_number, &current_string_number_length);
        } else {
            char_append(&current_string_number, &current_string_number_length, string[i]);
        }
    }
    free(current_string_number);
    free(string);
    *length = result_count;
    return result;
}

// deallocate the array that was made earlier.
interface void delete_array(integer **array) {
    free(*array);
    *array = NULL;
}

// inserts a element into an array, and keeps it sorted.
interface void insert(integer new_element, integer** array, integer* length) {
    integer place_toinsert = 0;
    while ((*array)[place_toinsert] < new_element && place_toinsert < *length) place_toinsert++;
    *array = realloc(*array, sizeof(integer) * (*length + 1));
    memmove((*array) + place_toinsert + 1, (*array) + place_toinsert, sizeof(integer) * (*length - place_toinsert));
    (*array)[place_toinsert] = new_element;
    (*length)++;
}

#endif /* helper_h */
