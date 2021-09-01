#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define Array(T)                      \
  struct Array {                      \
    T *contents;                      \
    uint32_t size;                    \
    uint32_t capacity;                \
    void *(*malloc)(size_t);          \
    void *(*realloc)(void *, size_t); \
    void (*free)(void *);             \
  }

#define array_init(self) \
  ((self)->size = 0, (self)->capacity = 0, (self)->malloc = malloc, (self)->realloc = realloc, (self)->free = free, (self)->contents = NULL)

#define array_new() \
  { NULL, 0, 0, malloc, realloc, free }

#define array_get(self, index) \
  (assert((uint32_t) index < (self)->size), &(self)->contents[index])

#define array_head(self) array_get(self, 0)

#define array_last(self) array_get(self, (self)->size - 1)

#define array_clear(self) ((self)->size = 0)

#define array_delete(self) array__delete((VoidArray *) self)

#define array_push(self, element) \
  (array__grow((VoidArray *) (self), 1, array__elem_size(self)), (self)->contents[(self)->size++] = (element))

#define array_insert(self, index, element) \
  array__splice((VoidArray *) (self), array__elem_size(self), index, 0, 1, &element)

#define array_grow_by(self, count)                                                 \
  (array__grow((VoidArray *) (self), count, array__element_size(self)),            \
   memset((self)->contents + (self)->size, 0, (count) *array__element_size(self)), \
   (self)->size += (count))

// Private

typedef Array(void) VoidArray;

#define array__elem_size(self) sizeof(*(self)->contents)

#define array__malloc(self) (self)->malloc
#define array__realloc(self) (self)->realloc
#define array__free(self) (self)->free

static inline void array__delete(VoidArray *self) {
  array__free(self)(self->contents);
  self->contents = NULL;
  self->size     = 0;
  self->capacity = 0;
}

static inline void array__reserve(VoidArray *self, size_t element_size, uint32_t new_capacity) {
  if (new_capacity > self->capacity) {
    if (self->contents) {
      self->contents = array__realloc(self)(self->contents, new_capacity * element_size);
    } else {
      self->contents = array__malloc(self)(new_capacity * element_size);
    }
    self->capacity = new_capacity;
  }
}

static inline void array__grow(VoidArray *self, size_t margin, size_t element_size) {
  size_t new_size = self->size + margin;
  if (new_size > self->capacity) {
    size_t new_capacity = self->capacity * 2;
    if (new_capacity < 8) {
      new_capacity = 8;
    }
    if (new_capacity < new_size) {
      new_capacity = new_size;
    }
    array__reserve(self, element_size, new_capacity);
  }
}

static inline void array__splice(VoidArray *self,
                                 size_t element_size,
                                 uint32_t index,
                                 uint32_t old_count,
                                 uint32_t new_count,
                                 const void *elements) {
  uint32_t new_size = self->size + new_count - old_count;
  uint32_t old_end  = index + old_count;
  uint32_t new_end  = index + new_count;
  assert(old_end <= self->size);

  array__reserve(self, element_size, new_size);

  char *contents = (char *) self->contents;
  if (self->size > old_end) {
    memmove(contents + new_end * element_size,
            contents + old_end * element_size,
            (self->size - old_end) * element_size);
  }
  if (new_count > 0) {
    if (elements) {
      memcpy((contents + index * element_size),
             elements,
             new_count * element_size);
    } else {
      memset((contents + index * element_size),
             0,
             new_count * element_size);
    }
  }
  self->size += new_count - old_count;
}

#ifdef __cplusplus
}
#endif
