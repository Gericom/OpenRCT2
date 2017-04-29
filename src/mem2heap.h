#ifndef __MEM2HEAP_H__
#define __MEM2HEAP_H__


#ifdef __cplusplus
extern "C" {
#endif

typedef struct heap_block_s heap_block_t;

struct heap_block_s
{
	uint32_t tag;
	uint32_t size;
	heap_block_t* prev;
	heap_block_t* next;
	uint8_t data[0];
};

void mem2heap_init();
void* mem2heap_alloc(int size);
void* mem2heap_realloc(void* ptr, int size);
void mem2heap_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif