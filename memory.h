#pragma once
#include <stdio.h>

typedef struct Item
{
    char *key;
    char *value;
    Item *next;
    Item *prev;
} Item;

typedef struct List
{
    int size;
    Item *head;
    Item *latest;
} List;

int addItem(List *l, Item *i);

char *getItem(List *l, char *key);