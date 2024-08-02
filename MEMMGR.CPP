#include "MEMMGR.H"

MemoryManager::MemoryManager() : pointer_count(0)
{
    for (int i = 0; i < MAX_POINTERS; ++i)
    {
        allocated_pointers[i] = NULL;
    }
}

MemoryManager::~MemoryManager()
{
    // Free all remaining allocated memory
    for (int i = 0; i < pointer_count; ++i)
    {
        free(allocated_pointers[i]);
    }
}

size_t MemoryManager::get_pointer_count()
{
    return pointer_count;
}

void *MemoryManager::allocate_memory(size_t size)
{
    void *ptr = malloc(size);
    if (ptr != NULL && pointer_count < MAX_POINTERS)
    {
        allocated_pointers[pointer_count++] = ptr;
    }
    return ptr;
}

void MemoryManager::free_memory(void *ptr)
{
    for (int i = 0; i < pointer_count; ++i)
    {
        if (allocated_pointers[i] == ptr)
        {
            free(ptr);
            allocated_pointers[i] = allocated_pointers[--pointer_count];
            allocated_pointers[pointer_count] = NULL;
            return;
        }
    }
    println("Pointer not found in allocated list");
}

int MemoryManager::is_allocated(void *ptr)
{
    for (int i = 0; i < pointer_count; ++i)
    {
        if (allocated_pointers[i] == ptr)
        {
            return TRUE; // true
        }
    }
    return FALSE; // false
}

char **MemoryManager::allocate_string_array(size_t num_strings, size_t string_length)
{
    // Allocate memory for the array of string pointers
    char **string_array = (char **)allocate_memory(num_strings * sizeof(char *));
    if (string_array == NULL)
    {
        return NULL; // Allocation failed
    }

    // Allocate memory for each string
    for (size_t i = 0; i < num_strings; ++i)
    {
        string_array[i] = (char *)allocate_memory(string_length * sizeof(char));
        if (string_array[i] == NULL)
        {
            // Allocation failed, free previously allocated memory
            for (size_t j = 0; j < i; ++j)
            {
                free_memory(string_array[j]);
            }
            free_memory(string_array);
            return NULL;
        }
    }

    return string_array;
}

void MemoryManager::free_string_array(char **string_array, size_t num_strings)
{
    // Free each string
    for (size_t i = 0; i < num_strings; ++i)
    {
        if (is_allocated(string_array[i]) == TRUE)
            free_memory(string_array[i]);
    }
    // Free the array of string pointers
    free_memory(string_array);
}
