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
#define wsize 4
#define dsize 8
#define CHUNKSIZE (1 << 12)

#define MAX(x,y) ((x) > (y)? (x) : (y))

#define PACK(size, alloc) (size | alloc)

#define GET(p) (*(unsigned int *) (p))
#define PUT(p, val) (*(unsigned int *) (p) = (val))

/* read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/*Given block ptr bp, compute address of its header and footer*/
#define HDRP(bp) ((char *)(bp)-wsize)
#define FTRP(bp) ((char *)(bp)+GET_SIZE(HDRP(bp)) - dsize)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - wsize)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - dsize)))

/* rounds up to the nearest multiple of dsize */
#define ALIGN(size) (((size) + (dsize-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* 
 * mm_init - initialize the malloc package.
 */
void *heap_listp;
static void *coalesce(void *bp){
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if(prev_alloc && next_alloc){ // no freeblock nearby
        return bp;
    }
    else if (prev_alloc && !next_alloc){ // the freeblock is on the right
        size+= GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
    }
    else if (!prev_alloc && next_alloc){ // the free block is on the left
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp),PACK(size,0));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        bp = PREV_BLKP(bp);
    }
    else{
        size += GET_SIZE(HDRP(PREV_BLKP(bp)))+ GET_SIZE(HDRP(NEXT_BLKP(bp))); // both leftside and rightside block are free block
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
        bp = PREV_BLKP(bp);
    }
    return bp;

}
static void *extend_heap(size_t words){
    char *bp;
    size_t size;

    /*Allocate an even number of words tp maintain allignment */
    size = (words%2)? (words+1)*wsize : words *wsize;
    if((long)(bp = mem_sbrk(size)) == -1){
        return NULL;
    }
    /* Initialize free block header/footer and the epiloque header */
    PUT(HDRP(bp),PACK(size,0)); /*free block header */
    PUT(FTRP(bp),PACK(size,0)); /*freeblock footer */
    PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1)); /*new epiloque header*/

    /*Calesce if the previous block was free */
    return coalesce(bp);
}


static void *find_fit(size_t asize){
    /*first-fit search*/
    void *bp;
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))){
            return bp;
        }
    }
    return NULL; /* no fit */
}

static void place(void *bp, size_t asize){
    size_t csize = GET_SIZE(HDRP(bp));
    if ((csize - asize) >= (2*dsize)){
        PUT(HDRP(bp), PACK(asize,1));
        PUT(FTRP(bp), PACK(asize,1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize-asize,0));
        PUT(FTRP(bp), PACK(csize-asize,0));
    }
    else{
        PUT(HDRP(bp),PACK(csize,1));
        PUT(FTRP(bp),PACK(csize,1));
    }
}

int mm_init(void)
{
    heap_listp = mem_sbrk((wsize)*4); 
    if (heap_listp == (void *)-1){
        return -1;
    }
    
    PUT(heap_listp, PACK(0,0));
    PUT(heap_listp + (1*wsize), PACK(dsize, 1)); // prologue header
    PUT(heap_listp + (2*wsize), PACK(dsize,1)); // prologue footer
    PUT(heap_listp + (3*wsize), PACK(0,1)); // epilogue 
    heap_listp = heap_listp+dsize;

    /* Extend the empty heap with a free block of Chunksize bytes */
    if (extend_heap(CHUNKSIZE/wsize) == NULL){
        return -1;
    }
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
    // make it to the last block of the heap by get the epiloque out and change the previous ptr 
    // by looping the previous block to find the previous freelist (also make the previous free block p 
    // to point to this block) and make the next ptr to null
    // and return that
    size_t asize; /*Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit*/
    char *bp;

    /*Ignore spurious requests */
    if (size == 0){
        return NULL;
    }

    /* Adjust block size to include overhead and alignment reqs . */
    if (size <= dsize){
        asize = 2*dsize;
    }
    else{
        asize = dsize * ((size + (dsize) + (dsize-1)) / dsize);
    }

    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL){
        place(bp,asize);
        return bp;
    }
    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize,CHUNKSIZE);
    if ((bp = extend_heap(extendsize/wsize)) == NULL){
        return NULL;
    }
    place(bp,asize);
    return bp;
}



/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr),PACK(size,0));
    PUT(FTRP(ptr),PACK(size,0));
    coalesce(ptr);
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
    if (newptr == NULL){
        return NULL;   
    }
      
    copySize = GET_SIZE(HDRP(oldptr)); // get old size
    if (size < copySize){ // new size < old size
        // remainder = copySize - size
        // PUT(oldptr, PACK(copySize, 1)) create new header
        // PUT(oldptr + size + 1, PACK(copySize, 1)) create new footer
        // PUT(oldptr, PACK(copySize, 1))
        copySize = size;
    }
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}
















