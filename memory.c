#include "memory.h"

int addItem(List *l, Item *i)
{
    if (l->head == NULL)
    {
        l->head = i;
        l->latest = i;
        l->latest->next = NULL;
        l->latest->prev = NULL;

    }
    else
    {
        l->latest->next = i;
        i->prev = l->latest;
        l->latest = i;
    }
    
    return 0;
}

char *getItem(List *l, char *key)
{
    Item *p = l->head;
    while (p != NULL)
    {
        if (!strcmp(p->key, key))
            return p->value;
        p = p->next;
    }

    return NULL;
}