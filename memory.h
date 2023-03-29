#pragma once
#include <stdio.h>

#define BUFSIZE 128

typedef struct Item
{
    char key[BUFSIZE];
    char value[BUFSIZE];
    struct Item *next;
    struct Item *prev;
} Item;

typedef struct List
{
    Item *head;
    Item *latest;
} List;

int addItem(List *l, Item *i);

char *getItem(List *l, char *key);