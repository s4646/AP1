#pragma once

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

int getItem(List *l, char *key);