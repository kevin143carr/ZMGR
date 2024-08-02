#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdlib.h> // For size_t
#include <stdio.h>  // For printf
#include "UTILS.H"

#define MAX_POINTERS 100 // Maximum number of pointers to track

class MemoryManager
{
private:
    void *allocated_pointers[MAX_POINTERS];
    size_t pointer_count;

public:
    MemoryManager();
    ~MemoryManager();
    size_t get_pointer_count();
    void *allocate_memory(size_t size);
    void free_memory(void *ptr);
    int is_allocated(void *ptr); // Using int instead of bool
    char **allocate_string_array(size_t num_strings, size_t string_length);
    void free_string_array(char **string_array, size_t num_strings);
};

#endif // MEMORY_MANAGER_H
