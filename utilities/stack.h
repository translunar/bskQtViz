/*
 ISC License

 Copyright (c) 2016-2017, Autonomous Vehicle Systems Lab, University of Colorado at Boulder

 Permission to use, copy, modify, and/or distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.

 THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

 */
/*
* stack.h
*
* Defines stack implementation for use in shunting yard algorithm
*
* University of Colorado, Autonomous Vehicle Systems (AVS) Lab
* Unpublished Copyright (c) 2012-2015 University of Colorado, All Rights Reserved
*/

#ifndef _STACK_H_
#define _STACK_H_

#ifndef __cplusplus
#define bool  int
#define false 0
#define true  1
#endif

typedef struct StackItem_t {
    char               *val;
    int                 flags;
    struct StackItem_t *next;
} StackItem_t;

typedef struct Stack_t {
    StackItem_t *current;
    StackItem_t *top;
} Stack_t;

Stack_t      *stackAlloc(void);
void          stackPush(Stack_t *list, char *val, int flags);
void          stackPushUnalloc(Stack_t *list, char *val, int flags);
// Pop from the stack
char         *stackPop(Stack_t *list);
// Pop from the stack and return the entire item
StackItem_t  *stackPopItem(Stack_t *list);
// Pop from the stack, but only return a single character so memory doesn't have to be freed
char          stackPopChar(Stack_t *list);
// Return the value of the item from the top of the stack without popping
char         *stackTop(Stack_t *list);
// Return the item from the top of the stack without popping
StackItem_t  *stackTopItem(Stack_t *list);
void          stackDisplay(Stack_t *list);
bool          stackIsEmpty(Stack_t *list);
void          stackFree(Stack_t *list);
// Free an item from the stack after popping it
void          stackFreeItem(StackItem_t *item);

#endif
