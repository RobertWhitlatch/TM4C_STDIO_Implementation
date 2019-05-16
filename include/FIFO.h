#ifndef __FIFO_H__
#define __FIFO_H__ 1

#include "Master.h"

#define AddIndexFifo(NAME, SIZE, TYPE, SUCCESS, FAIL)   \
    uint32_t volatile NAME##PutI;                       \
    uint32_t volatile NAME##GetI;                       \
    TYPE static NAME##Fifo[SIZE];                       \
    void NAME##Fifo_Init(void) {                        \
        long sr;                                        \
        sr = StartCritical();                           \
        NAME##PutI = NAME##GetI = 0;                    \
        EndCritical(sr);                                \
    }                                                   \
    int NAME##Fifo_Put(TYPE data) {                     \
        if ((NAME##PutI - NAME##GetI) & ~(SIZE - 1)) {  \
            return (FAIL);                              \
        }                                               \
        NAME##Fifo[NAME##PutI & (SIZE - 1)] = data;     \
        NAME##PutI++;                                   \
        return (SUCCESS);                               \
    }                                                   \
    int NAME##Fifo_Get(TYPE *datapt) {                  \
        if (NAME##PutI == NAME##GetI) {                 \
            return (FAIL);                              \
        }                                               \
        *datapt = NAME##Fifo[NAME##GetI & (SIZE - 1)];  \
        NAME##GetI++;                                   \
        return (SUCCESS);                               \
    }                                                   \
    unsigned short NAME##Fifo_Size(void) {              \
        return ((uint32_t)(NAME##PutI - NAME##GetI));   \
    }

#define AddPointerFifo(NAME, SIZE, TYPE, SUCCESS, FAIL)                           \
    TYPE volatile *NAME##PutPt;                                                   \
    TYPE volatile *NAME##GetPt;                                                   \
    TYPE static NAME##Fifo[SIZE];                                                 \
    void NAME##Fifo_Init(void) {                                                  \
        long sr;                                                                  \
        sr = StartCritical();                                                     \
        NAME##PutPt = NAME##GetPt = &NAME##Fifo[0];                               \
        EndCritical(sr);                                                          \
    }                                                                             \
    int NAME##Fifo_Put(TYPE data) {                                               \
        TYPE volatile *nextPutPt;                                                 \
        nextPutPt = NAME##PutPt + 1;                                              \
        if (nextPutPt == &NAME##Fifo[SIZE]) {                                     \
            nextPutPt = &NAME##Fifo[0];                                           \
        }                                                                         \
        if (nextPutPt == NAME##GetPt) {                                           \
            return (FAIL);                                                        \
        } else {                                                                  \
            *(NAME##PutPt) = data;                                                \
            NAME##PutPt = nextPutPt;                                              \
            return (SUCCESS);                                                     \
        }                                                                         \
    }                                                                             \
    int NAME##Fifo_Get(TYPE *datapt) {                                            \
        if (NAME##PutPt == NAME##GetPt) {                                         \
            return (FAIL);                                                        \
        }                                                                         \
        *datapt = *(NAME##GetPt##++);                                             \
        if (NAME##GetPt == &NAME##Fifo[SIZE]) {                                   \
            NAME##GetPt = &NAME##Fifo[0];                                         \
        }                                                                         \
        return (SUCCESS);                                                         \
    }                                                                             \
    unsigned short NAME##Fifo_Size(void) {                                        \
        if (NAME##PutPt < NAME##GetPt) {                                          \
            return ((uint32_t)(NAME##PutPt - NAME##GetPt + (SIZE * sizeof(TYPE))) \
              / sizeof(TYPE));                                                    \
        }                                                                         \
        return ((uint32_t)(NAME##PutPt - NAME##GetPt) / sizeof(TYPE));            \
    }

#endif // __FIFO_H__
