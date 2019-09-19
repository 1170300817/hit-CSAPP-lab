/*
 * mm.c - 显示分离链表实现
 * 1170300817 林之浩
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "mm.h"
#include "memlib.h"

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

/* Basic constants and macros */
#define WSIZE       4       /* word size (bytes) */
#define DSIZE       8       /* doubleword size (bytes) */
#define INITCHUNKSIZE (1<<6)    /* 初始的堆大小 */
#define CHUNKSIZE (1<<11)   /* 每次增加的堆大小 */
#define OVERHEAD    8       /* overhead of header and footer (bytes) */

#define LISTLIMIT 20
#define REALLOC_BUFFER  (1<<7)

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

// Pack a size and allocated bit into a word
#define PACK(size, alloc) ((size) | (alloc))

// Read and write a word at address p
#define GET(p)            (*(unsigned int *)(p))
#define PUT(p, val)       (*(unsigned int *)(p) = (val) | GET_TAG(p))
#define PUT_NOTAG(p, val) (*(unsigned int *)(p) = (val))

// Store predecessor or successor pointer for free blocks
#define SET_PTR(p, ptr) (*(unsigned int *)(p) = (unsigned int)(ptr))

// 读取p指针指向的块的大小，是否分配和realloc_tag
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define GET_TAG(p)   (GET(p) & 0x2)
#define SET_RATAG(p)   (GET(p) |= 0x2)
#define REMOVE_RATAG(p) (GET(p) &= ~0x2)

// Address of block's header and footer（书本）
#define HDRP(ptr) ((char *)(ptr) - WSIZE)
#define FTRP(ptr) ((char *)(ptr) + GET_SIZE(HDRP(ptr)) - DSIZE)

//获取下一个块和前一个块的地址（书本）
#define NEXT_BLKP(ptr) ((char *)(ptr) + GET_SIZE((char *)(ptr) - WSIZE))
#define PREV_BLKP(ptr) ((char *)(ptr) - GET_SIZE((char *)(ptr) - DSIZE))

//获取前后空闲块的地址
#define PRED_PTR(ptr) ((char *)(ptr))
#define SUCC_PTR(ptr) ((char *)(ptr) + WSIZE)

//获取目标块在空闲列表上的前后块
#define PRED(ptr) (*(char **)(ptr))
#define SUCC(ptr) (*(char **)(SUCC_PTR(ptr)))

static char *heap_listp; // 指向堆的头部的指针
void *seg_f_list_set[20];//空闲链表集合

static void *extend_heap(size_t size);
static void *coalesce(void *ptr);
static void *place(void *ptr, size_t asize);
static void Insert_node(void *ptr, size_t size);
static void Delete_node(void *ptr);
team_t team =
{
    /* Team name */
    "HIT IS MAD",
    /* First member's full name */
    "林之浩1170300817",
    /* First member's email address */
    "630073498@qq.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

static void *extend_heap(size_t size)
{
    void *ptr;
    size_t asize;
    asize = ALIGN(size);        // 把传入的size对齐
    if ((ptr = mem_sbrk(asize)) == (void *)-1)  //扩展堆
        return NULL;
    PUT_NOTAG(HDRP(ptr), PACK(asize, 0));       //设置新扩展的堆的头部和脚部
    PUT_NOTAG(FTRP(ptr), PACK(asize, 0));
    PUT_NOTAG(HDRP(NEXT_BLKP(ptr)), PACK(0, 1));        //设置终止头部
    Insert_node(ptr, asize);    //在空闲链表插入这个新分配的空闲块
    return coalesce(ptr);       //如果前面也是空闲块，合并后返回
}

static void Insert_node(void *ptr, size_t size)
{
    int list_index = 0;
    void *search_ptr = NULL;
    void *insert_ptr = NULL;
    while ((list_index < LISTLIMIT - 1) && (size > 1))      //选择要插入的空闲链表是哪一个
    {
        size >>= 1;
        list_index++;
    }
    // Keep size ascending order and search
    search_ptr = seg_f_list_set[list_index];        //搜索插入位置，初始化搜索index为表头（实际是表尾）
    while ((search_ptr != NULL) && (size > GET_SIZE(HDRP(search_ptr))))     //不满足大小要求
    {
        insert_ptr = search_ptr;
        search_ptr = PRED(search_ptr);  //就取前一个节点（更大）
    }

    // Set predecessor and successor
    if (search_ptr != NULL)     //插入位置不是第一个节点
    {
        if (insert_ptr != NULL)     //插在表中间
        {
            SET_PTR(PRED_PTR(ptr), search_ptr);     //设置插入块的前块为search_ptr指向的块
            SET_PTR(SUCC_PTR(search_ptr), ptr);     //设置search_ptr指向的块为插入块
            SET_PTR(SUCC_PTR(ptr), insert_ptr);     //设置插入块的后块为insert_ptr指向的块（原当前位置的块）
            SET_PTR(PRED_PTR(insert_ptr), ptr);     //设置原当前位置的块的前块为插入块
        }       //插入完成
        else        //插在表尾
        {
            SET_PTR(PRED_PTR(ptr), search_ptr);     //设置插入块的前块为search_ptr指向的块
            SET_PTR(SUCC_PTR(search_ptr), ptr);     //设置search_ptr指向的块为插入块
            SET_PTR(SUCC_PTR(ptr), NULL);           //设置插入块的后块为空（插在表尾）
            seg_f_list_set[list_index] = ptr;       //更新表尾节点
        }
    }
    else//插入位置是第一个节点
    {
        if (insert_ptr != NULL)
        {
            SET_PTR(PRED_PTR(ptr), NULL);
            SET_PTR(SUCC_PTR(ptr), insert_ptr);
            SET_PTR(PRED_PTR(insert_ptr), ptr);
        }
        else
        {
            SET_PTR(PRED_PTR(ptr), NULL);
            SET_PTR(SUCC_PTR(ptr), NULL);
            seg_f_list_set[list_index] = ptr;
        }
    }

    return;
}

static void Delete_node(void *ptr)
{
    int list_index = 0;
    size_t size = GET_SIZE(HDRP(ptr));
    while ((list_index < LISTLIMIT - 1) && (size > 1))
    {
        size >>= 1;
        list_index++;
    }
    if (PRED(ptr) != NULL)
    {
        if (SUCC(ptr) != NULL)
        {
            SET_PTR(SUCC_PTR(PRED(ptr)), SUCC(ptr));
            SET_PTR(PRED_PTR(SUCC(ptr)), PRED(ptr));
        }
        else
        {
            SET_PTR(SUCC_PTR(PRED(ptr)), NULL);
            seg_f_list_set[list_index] = PRED(ptr);
        }
    }
    else
    {
        if (SUCC(ptr) != NULL)
        {
            SET_PTR(PRED_PTR(SUCC(ptr)), NULL);
        }
        else
        {
            seg_f_list_set[list_index] = NULL;
        }
    }
    return;
}

static void *coalesce(void *ptr)
{
    size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(ptr)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));    //记录前后块是否已分配
    size_t size = GET_SIZE(HDRP(ptr));      //记录当前块的大小
    //如果前块的realloc_tag是1，则把前块视为已分配，不进行合并
    if (GET_TAG(HDRP(PREV_BLKP(ptr))))
        prev_alloc = 1;
    if (prev_alloc && next_alloc)                           // 情况 1
    {
        return ptr;
    }
    else if (prev_alloc && !next_alloc)                     // 情况 2
    {
        Delete_node(ptr);
        Delete_node(NEXT_BLKP(ptr));
        size += GET_SIZE(HDRP(NEXT_BLKP(ptr)));
        PUT(HDRP(ptr), PACK(size, 0));
        PUT(FTRP(ptr), PACK(size, 0));
    }
    else if (!prev_alloc && next_alloc)                     // 情况 3
    {
        Delete_node(ptr);
        Delete_node(PREV_BLKP(ptr));
        size += GET_SIZE(HDRP(PREV_BLKP(ptr)));
        PUT(FTRP(ptr), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
        ptr = PREV_BLKP(ptr);
    }
    else                                                    // 情况 4
    {
        Delete_node(ptr);
        Delete_node(PREV_BLKP(ptr));
        Delete_node(NEXT_BLKP(ptr));
        size += GET_SIZE(HDRP(PREV_BLKP(ptr))) + GET_SIZE(HDRP(NEXT_BLKP(ptr)));
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(ptr)), PACK(size, 0));
        ptr = PREV_BLKP(ptr);
    }
    Insert_node(ptr, size);
    return ptr;
}

static void *place(void *ptr, size_t asize)
{
    size_t ptr_size = GET_SIZE(HDRP(ptr));
    size_t remainder = ptr_size - asize;
    Delete_node(ptr);
    if (remainder <= DSIZE * 2)
    {
        PUT(HDRP(ptr), PACK(ptr_size, 1));
        PUT(FTRP(ptr), PACK(ptr_size, 1));
    }
    else if (asize >= 100)
    {
        PUT(HDRP(ptr), PACK(remainder, 0));
        PUT(FTRP(ptr), PACK(remainder, 0));
        PUT_NOTAG(HDRP(NEXT_BLKP(ptr)), PACK(asize, 1));
        PUT_NOTAG(FTRP(NEXT_BLKP(ptr)), PACK(asize, 1));
        Insert_node(ptr, remainder);
        return NEXT_BLKP(ptr);
    }
    else
    {
        PUT(HDRP(ptr), PACK(asize, 1));
        PUT(FTRP(ptr), PACK(asize, 1));
        PUT_NOTAG(HDRP(NEXT_BLKP(ptr)), PACK(remainder, 0));
        PUT_NOTAG(FTRP(NEXT_BLKP(ptr)), PACK(remainder, 0));
        Insert_node(NEXT_BLKP(ptr), remainder);
    }
    return ptr;
}

/*
 * mm_init - 初始化堆
 */
int mm_init(void)
{
    int list_index;
    for (list_index = 0; list_index < LISTLIMIT; list_index++)
    {
        seg_f_list_set[list_index] = NULL;// 初始化分离的空闲链表数组
    }
    /* create the initial empty heap */
    if ((long)(heap_listp = mem_sbrk(4 * WSIZE)) == 0)
        return -1;
    PUT_NOTAG(heap_listp, 0);                            /* 对齐用的填充字 */
    PUT_NOTAG(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); /* 序言块的头部：8字节，已分配 */
    PUT_NOTAG(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); /* 序言块的脚部：8字节，已分配 */
    PUT_NOTAG(heap_listp + (3 * WSIZE), PACK(0, 1));     /* 结尾块：大小为0，已分配 */
    if (extend_heap(INITCHUNKSIZE) == NULL)     //扩展堆大小为初始大小
        return -1;
    return 0;
}
/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 */
void *mm_malloc(size_t size)
{
    size_t asize;            //调整最小块大小
    size_t extendsize;      //扩展大小
    void *ptr = NULL;
    if (size == 0)
        return NULL;
    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = ALIGN(size+DSIZE);        //计算最小块
    int list_index = 0;
    size_t searchsize = asize;
    //搜索合适的空闲块
    while (list_index < LISTLIMIT)
    {
        if ((list_index == LISTLIMIT - 1) || ((searchsize <= 1) && (seg_f_list_set[list_index] != NULL)))
        {
            ptr = seg_f_list_set[list_index];
            //忽略太小的块，和realloc_tag是1的块
            while ((ptr != NULL) && ((asize > GET_SIZE(HDRP(ptr))) || (GET_TAG(HDRP(ptr)))))
            {
                ptr = PRED(ptr);        //满足条件
            }
            if (ptr != NULL)
                break;
        }
        searchsize >>= 1;
        list_index++;
    }
    // 如果没有空闲块，扩展堆大小
    if (ptr == NULL)
    {
        extendsize = MAX(asize, CHUNKSIZE);// 计算扩展堆大小
        if ((ptr = extend_heap(extendsize)) == NULL)
            return NULL;
    }
    // 放置目标块，分割
    ptr = place(ptr, asize);

    // 返回新放置的块
    return ptr;
}

/*
 * mm_free  释放一个块
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    REMOVE_RATAG(HDRP(NEXT_BLKP(ptr)));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    Insert_node(ptr, size);
    coalesce(ptr);
    return;
}

/*
 * mm_realloc 将ptr指针指向的块扩展为size大小
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *new_ptr = ptr;    /* 返回指针 */
    size_t new_size = size; /* 新块的大小 */
    int remainder;
    int extendsize;         /* S可能的扩展堆大小 */
    int block_buffer;
    if (size == 0)   //参数为0直接返回
        return NULL;
    if (new_size <= DSIZE)
        new_size = 2 * DSIZE;       //对齐
    else
        new_size = ALIGN(size+DSIZE);       //对齐
    new_size += REALLOC_BUFFER;
    block_buffer = GET_SIZE(HDRP(ptr)) - new_size;
    if (block_buffer < 0)
    {
        //如果后块没有分配或者是结束块，就使用或扩展使用下一个块*/
        if (!GET_ALLOC(HDRP(NEXT_BLKP(ptr))) || !GET_SIZE(HDRP(NEXT_BLKP(ptr))))
        {
            remainder = GET_SIZE(HDRP(ptr)) + GET_SIZE(HDRP(NEXT_BLKP(ptr))) - new_size;
            if (remainder < 0)
            {
                extendsize = MAX(-remainder, CHUNKSIZE);
                if (extend_heap(extendsize) == NULL)
                    return NULL;
                remainder += extendsize;
            }
            Delete_node(NEXT_BLKP(ptr));
            PUT_NOTAG(HDRP(ptr), PACK(new_size + remainder, 1));
            PUT_NOTAG(FTRP(ptr), PACK(new_size + remainder, 1));
        }
        else        //否则直接申请新的
        {
            new_ptr = mm_malloc(new_size - DSIZE);
            memcpy(new_ptr, ptr, MIN(size, new_size));
            mm_free(ptr);
        }
        block_buffer = GET_SIZE(HDRP(new_ptr)) - new_size;
    }
    if (block_buffer < 2 * REALLOC_BUFFER)
        SET_RATAG(HDRP(NEXT_BLKP(new_ptr)));
    return new_ptr;
}


//int mm_check()
//{
//    puts("mm_check：");
//    void *search_ptr;
//    void *insert_ptr;
//    int count_1=0;
//    int count_2=0;
//    for(int i=0; i<LISTLIMIT; i++)
//    {
//        search_ptr = seg_f_list_set[i];
//        if(search_ptr==NULL)
//            continue;
//        while (search_ptr != NULL)
//        {
//            insert_ptr = search_ptr;
//            if(!GET_ALLOC(HDRP(NEXT_BLKP(insert_ptr)))|| !GET_ALLOC(HDRP(PREV_BLKP(insert_ptr))))
//            {
//                puts("存在未合并空闲块");
//                return 0;
//            }
//            if(GET_ALLOC(HDRP(insert_ptr)))
//            {
//                puts("列表中有非空闲块");
//                return 0;
//            }
//            count_1+=1;
//            search_ptr = PRED(search_ptr);
//        }
//    }
//    search_ptr = heap_listp;
//    while(GET_SIZE(HDRP(search_ptr))>0)
//    {
//        if(!GET_ALLOC(HDRP(search_ptr)))
//        {
//            count_2++;
//        }
//        search_ptr = NEXT_BLKP(search_ptr);
//    }
//    if(count_2!=count_1)
//    {
//        puts("表和堆中块非一一对应");
//        return 0;
//    }
//    return 1;
//}
