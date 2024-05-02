#pragma once

typedef struct Handle {
    unsigned int index;
    unsigned int generation;
} Handle;

#define GENARRAY_DEFINE(T, capacity, prefix)                            \
struct T##ArrayEntry {                                                  \
    bool occupied;                                                      \
    union {                                                             \
        size_t next_free;                                               \
        T value;                                                        \
    };                                                                  \
    size_t generation;                                                  \
};                                                                      \
struct T##Array {                                                       \
    struct T##ArrayEntry data[(capacity)];                              \
    size_t free_head;                                                   \
};                                                                      \
void prefix##_array_init(struct T##Array* array);                       \
/* gets pointer to value at the handle provided, */                     \
/* returns null if invalid or unoccupied */                             \
T* prefix##_array_get(struct T##Array* array, Handle handle);           \
Handle prefix##_array_insert_new(struct T##Array* array);               \
Handle prefix##_array_insert(struct T##Array* array, T value);          \
void prefix##_array_remove(struct T##Array* array, Handle handle);      \
void prefix##_array_copy_into(struct T##Array* array, T* value);

#define GENARRAY_IMPL(T, capacity, prefix)                              \
void prefix##_array_init(struct T##Array* array) {                      \
    array->free_head = 0;                                               \
    for (int i = 0; i < (capacity) - 1; i++) {                          \
        array->data[i].next_free = i + 1;                               \
    }                                                                   \
}                                                                       \
T* prefix##_array_get(EntityArray* array, Handle handle) {              \
    struct EntityArrayEntry entry = array->data[handle.index];          \
    if (entry.occupied && entry.generation == handle.generation) {      \
        return &array->data[handle.index].value;                        \
    }                                                                   \
    return 0; /* TODO: nullptr? */                                      \
}                                                                       \
Handle prefix##_array_insert_new(struct T##Array* array) {              \
    return ent_array_insert(array, (Entity) {0});                       \
}                                                                       \
Handle prefix##_array_insert(struct T##Array* array, T value) {         \
    struct T##ArrayEntry* entry = &array->data[array->free_head];       \
    assert(!entry->occupied && "Entry at the free_head is occupied");   \
    Handle handle = {                                                   \
        .generation = entry->generation,                                \
        .index = array->free_head,                                      \
    };                                                                  \
    array->free_head = entry->next_free;                                \
    entry->occupied = true;                                             \
    value.handle = handle;                                              \
    entry->value = value;                                               \
    return handle;                                                      \
}                                                                       \
void prefix##_array_remove(struct T##Array* array, Handle handle) {     \
    struct EntityArrayEntry* entry = &array->data[handle.index];        \
    if (entry->occupied) {                                              \
        if (entry->generation != handle.generation) {                   \
            /* caller tried to remove an older generation, do nothing */\
            return;                                                     \
        }                                                               \
        entry->generation += 1;                                         \
        entry->occupied = false;                                        \
        array->free_head = handle.index;                                \
    }                                                                   \
    /* if entry->occupied is false, caller is trying to remove an */    \
    /* already-removed key, do nothing */                               \
}                                                                       \
void prefix##_array_copy_into(struct T##Array* array, T* value) {       \
    Handle handle = value->handle;                                      \
    array->data[handle.index].occupied = true;                          \
    array->data[handle.index].value = *value;                           \
}
