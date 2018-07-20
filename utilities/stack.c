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
* stack.c
*
* University of Colorado, Autonomous Vehicle Systems (AVS) Lab
* Unpublished Copyright (c) 2012-2015 University of Colorado, All Rights Reserved
*/

#include "stack.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *duplicateStr(const char *s);
char *duplicateStr(const char *s)
{
    char *d = (char *)calloc(strlen(s) + 1, sizeof(char));
    if(d == NULL) {
        return NULL;
    }
    strcpy(d, s);
    return d;
}

Stack_t *stackAlloc(void)
{
    Stack_t *list = (Stack_t *)calloc(1, sizeof(Stack_t));
    return list;
}

void stackPush(Stack_t *list, char *val, int flags)
{
    list->current = (StackItem_t *)calloc(1, sizeof(StackItem_t));
    list->current->val = duplicateStr(val);
    list->current->flags = flags;
    list->current->next = (StackItem_t *)list->top;
    list->top = list->current;
}

void stackPushUnalloc(Stack_t *list, char *val, int flags)
{
    stackPush(list, val, flags);
    free(val);
}

char *stackPop(Stack_t *list)
{
    StackItem_t *item = stackPopItem(list);
    char *val = duplicateStr(item->val);
    stackFreeItem(item);
    return val;
}

StackItem_t *stackPopItem(Stack_t *list)
{
    StackItem_t *item;

    if(stackIsEmpty(list)) {
        printf("Stack_t underflow\n");
        return NULL;
    }

    item = list->top;
    list->top = (StackItem_t *)list->top->next;

    return item;
}

char stackPopChar(Stack_t *list)
{
    char *val = stackPop(list);
    char valChar = val[0];
    free(val);

    return valChar;
}

char *stackTop(Stack_t *list)
{
    return list->top->val;
}

StackItem_t *stackTopItem(Stack_t *list)
{
    return list->top;
}

void stackDisplay(Stack_t *list)
{
    StackItem_t *p = list->top;
    while(p != NULL) {
        printf("%s ", p->val);
        p = (StackItem_t *)p->next;
    }
    printf("\n");
}

bool stackIsEmpty(Stack_t *list)
{
    StackItem_t *p = list->top;
    return (bool)(p == NULL);
}

void stackFree(Stack_t *list)
{
    StackItem_t *p = list->top;
    while(p != NULL) {
        p = (StackItem_t *)p->next;
        free(stackPop(list));
    }
    free(list);
}

void stackFreeItem(StackItem_t *item)
{
    free(item->val);
    free(item);
}
