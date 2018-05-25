#ifndef DBDM_H
#define DBDM_H

typedef struct {
  unsigned char next;
  unsigned char size;
} memNode_t;

void dbdm_init();
void *dbdm_malloc(char nBytes);
void dbdm_free(void *memory);

#endif
