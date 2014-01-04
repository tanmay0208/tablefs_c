#ifndef VECTOR_H__
#define VECTOR_H__

struct vector {
    void** data;
    int size;
    int count;
};

typedef struct vector vector;

void vector_init(vector*);
int vector_count(vector*);
void vector_add(vector*, void*);
void vector_set(vector*, int, void*);
void *vector_get(vector*, int);
void vector_delete(vector*, int);
void vector_free(vector*);
int vector_size(vector*);

#endif
