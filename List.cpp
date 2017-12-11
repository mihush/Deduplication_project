//
// Created by mihush on 09/12/2017.
//
#include "generic_list.h"

struct List_node {
    ListElement data;
    struct List_node * next;
};

struct List_t {
    Node head;
    Node iterator;
    CopyListElement copyFunction;
    FreeListElement freeFunction;
    int size;
};

Node NodeCreate(ListElement element, CopyListElement copyElement) {
    Node newNode = (Node) malloc(sizeof(*newNode));
    if(!newNode){
        return NULL;
    }
    newNode->data = copyElement(element);
    if (!newNode->data) {
        free(newNode);
        return NULL;
    }
    newNode->next = NULL;
    return newNode;
}

void NodeAdd(Node a, Node b) {
    Node tempNode = a->next;
    a->next = b;
    if (b != NULL){
        b->next = tempNode;
    }
}

static void NodeDestroy(Node node, FreeListElement freeElement) {
    if (!node)
        return;
    freeElement(node->data);
    free(node);
}

List listCreate(CopyListElement copyElement, FreeListElement freeElement) {
    if(copyElement == NULL || freeElement == NULL){
        return NULL;
    }
    List list = (List) malloc(sizeof(*list));
    if(!list){
        return NULL;
    }
    list->head = NULL;
    list->iterator = NULL;
    list->copyFunction = copyElement;
    list->freeFunction = freeElement;
    list->size = 0;
    return list;
}

int listGetSize(List list) {
    if(!list){
        return -1;
    }
    return list->size;
}

ListElement listGetFirst(List list) {
    if(!list){
        return NULL;
    }
    list->iterator = list->head;
    if(!list->iterator){
        return NULL;
    }
    return list->iterator->data;
}

ListElement listGetNext(List list) {
    if(!list || !list->iterator){
        return NULL;
    }
    list->iterator = list->iterator->next;
    if(!list->iterator){
        return NULL;
    }
    return list->iterator->data;
}

ListElement listGetCurrent(List list) {
    if(!list || !list->iterator){
        return NULL;
    }
    return list->iterator->data;
}

ListResult listInsertFirst(List list, ListElement element) {
    if(!list){
        return LIST_NULL_ARGUMENT;
    }
    if(!element){
        return LIST_OUT_OF_MEMORY;
    }
    Node newNode = NodeCreate(element, list->copyFunction);
    if(!newNode){
        return LIST_OUT_OF_MEMORY;
    }
    if (!list->head) {
        list->head = newNode;
        list->size++;
        return LIST_SUCCESS;
    }
    Node head = list->head;
    newNode->next = head;
    list->head = newNode;
    ListElement tmp = newNode->data;
    newNode->data = list->head->data;
    list->head->data = tmp;
    if(list->iterator == list->head) {
        list->iterator = list->iterator->next;
    }
    list->size++;
    return LIST_SUCCESS;
}

ListResult listInsertLast(List list, ListElement element) {
    if(!list){
        return LIST_NULL_ARGUMENT;
    }
    if(!element){
        return LIST_OUT_OF_MEMORY;
    }
    Node newNode = NodeCreate(element, list->copyFunction);
    if(!newNode){
        return LIST_OUT_OF_MEMORY;
    }
    if (!list->head){
        list->head = newNode;
    } else {
        Node last = list->head;
        while(last->next){
            last = last->next;
        }
        NodeAdd(last, newNode);
    }
    list->size++;
    return LIST_SUCCESS;
}

ListResult listInsertBeforeCurrent(List list, ListElement element) {
    if(!list){
        return LIST_NULL_ARGUMENT;
    }
    if(!list->iterator){
        return LIST_INVALID_CURRENT;
    }
    if(!element){
        return LIST_OUT_OF_MEMORY;
    }
    Node newNode = NodeCreate(element, list->copyFunction);
    if(!newNode){
        return LIST_OUT_OF_MEMORY;
    }
    NodeAdd(list->iterator, newNode);
    ListElement tempElement = list->iterator->data;
    list->iterator->data = newNode->data;
    newNode->data = tempElement;
    list->size++;
    list->iterator = list->iterator->next;
    return LIST_SUCCESS;
}

ListResult listInsertAfterCurrent(List list, ListElement element) {
    if(!list){
        return LIST_NULL_ARGUMENT;
    }
    if(!list->iterator){
        return LIST_INVALID_CURRENT;
    }
    if(!element){
        return LIST_OUT_OF_MEMORY;
    }
    Node newNode = NodeCreate(element, list->copyFunction);
    if(!newNode){
        return LIST_OUT_OF_MEMORY;
    }
    if(!element){
        return LIST_OUT_OF_MEMORY;
    }
    NodeAdd(list->iterator, newNode);
    list->size++;
    return LIST_SUCCESS;
}

ListResult listRemoveCurrent(List list) {
    if(!list){
        return LIST_NULL_ARGUMENT;
    }
    if(!list->iterator){
        return LIST_INVALID_CURRENT;
    }
    if (list->iterator == list->head) {
        list->head = list->head->next;
    } else {
        Node prevNode = list->head;
        while (prevNode->next != list->iterator){
            prevNode = prevNode->next;
        }
        prevNode->next = list->iterator->next;
    }
    NodeDestroy(list->iterator, list->freeFunction);
    list->iterator = NULL;
    list->size--;
    return LIST_SUCCESS;
}
ListResult listClear(List list) {
    if(!list){
        return LIST_NULL_ARGUMENT;
    }
    Node iterator = list->head;
    while (iterator != NULL) {
        Node temp = iterator->next;
        NodeDestroy(iterator, list->freeFunction);
        iterator = temp;
        list->size--;
    }
    list->size = 0;
    list->head = NULL;
    return LIST_SUCCESS;
}

List listCopy(List list) {
    if(!list){
        return NULL;
    }
    List cloneList = listCreate(list->copyFunction, list->freeFunction);
    if(!cloneList){
        return NULL;
    }
    int current = 0;
    int location = -1;
    Node tmp = list->iterator;
    LIST_FOREACH(Node,iterator,list);
    {
        if(iterator == tmp){
            location = current;
        }
        ListResult res = listInsertLast(cloneList, iterator);
        if(res != LIST_SUCCESS) {
            listDestroy(cloneList);
            return NULL;
        }
        current++;
    }
    location++;
    if(location == -1) {
        cloneList->iterator = NULL;
    } else {
        current = 0;
        LIST_FOREACH(Node, iterator, cloneList) {
            if(location == current) {
                cloneList->iterator = tmp;
                break;
            }
            current++;
        }
        list->iterator = tmp;
    }
    return cloneList;
}

ListResult listSort(List list, CompareListElements compareFunction) {
    if(!list || !compareFunction){
        return LIST_NULL_ARGUMENT;
    }
    for (Node external = list->head; external != NULL; external = external->next) {
        for (Node inner = list->head; inner != NULL; inner = inner->next) {
            if (compareFunction(external->data, inner->data)) {
                ListElement tempElement = external->data;
                external->data = inner->data;
                inner->data = tempElement;
            }
        }
    }
    return LIST_SUCCESS;
}

List listFilter(List list, FilterListElement filterFunction, ListFilterKey key) {
    if(!list || !filterFunction){
        return NULL;
    }
    List filtered = listCreate(list->copyFunction, list->freeFunction);
    if(!filtered){
        return NULL;
    }
    LIST_FOREACH(ListElement, it, list) {
        if (filterFunction(it, key)) {
            ListResult res = listInsertLast(filtered, it);
            if (res != LIST_SUCCESS) {
                listDestroy(filtered);
                return NULL;
            }
        }
    }
    filtered->iterator = filtered->head;
    return filtered;
}

ListElement listFind(List list, ListElement toFind, CompareListElement compare) {
    Node iterator = list->head;
    while (iterator != NULL) {
        if (compare(toFind,iterator->data))
        {
            return iterator->data;
        }
        iterator = iterator->next;
    }
    return NULL;
}

void listDestroy(List list) {
    if (!list)
        return;
    listClear(list);
    free(list);
}
