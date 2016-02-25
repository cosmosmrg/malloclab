/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "5680669+5681091",
    /* First member's full name */
    "Arparnuch Vai-ngamsamer",
    /* First member's email address */
    "5680669",
    /* Second member's full name (leave blank if none) */
    "Sawarin Sodsangsook",
    /* Second member's email address (leave blank if none) */
    "5691091"
};

/* single word (4) or double word (8) alignment */
#define single_word 4
#define double_word 8
#define CHUNKSIZE (1 << 12)

#define PACK(size, alloc) (size | alloc)

#define GET(p) (*(unsigned int *) (p))
#define PUT(p, val) (*(unsigned int *) (p) = (val))
/* rounds up to the nearest multiple of double_word */
#define ALIGN(size) (((size) + (double_word-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* 
 * mm_init - initialize the malloc package.
 */
char *prologue;
int mm_init(void)
{
    char *avai = mem_sbrk((single_word)*4)); 
    if (avai == -1){
        return -1
    }
    
    PUT(avai, PACK(0,0));
    PUT(avai + (1*single_word), PACK(double_word, 1)); // prologue header
    PUT(avai + (2*single_word), PACK(double_word,1)); // prologue footer
    PUT(avai + (3*single_word), PACK(0,1)); // epilogue 
    prologue = avai+double_word;
    return 1;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    // check if anyfreeblock >=size
    // if have return the ptr to the first block
    // if not initial the new block to handle this size
    // >> check if the size use more than or equal to 8 bytes
    //      if no make size equal to 8 bytes
    // ALIGN(size+header+footer) // round it to be divisible by 8
    int newsize = ALIGN(size + SIZE_T_SIZE); // round up to the should-be malloc size
    // mem_sbrk(new size)
    void *p = mem_sbrk(newsize); // Expands the heap and returns the first byte of the allocated area

    // make it to the last block of the heap by get the epiloque out and change the previous ptr 
    // by looping the previous block to find the previous freelist (also make the previous free block p 
    // to point to this block) and make the next ptr to null
    // and return that

    if (p == (void *)-1) )return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}















