#include "simple_logger.h"

#include "gfc_types.h"
#include "gfc_list.h"

void gfc_list_delete(GFC_List *list)
{
    if (!list)return;
    if (list->elements)
    {
        free(list->elements);
    }
    free(list);
}

GFC_List *gfc_list_new()
{
    return gfc_list_new_size(16);
}

GFC_List *gfc_list_copy(GFC_List *old)
{
    GFC_List *new;
    if (!old)return 0;
    if (old->size <= 0)return NULL;
    new = gfc_list_new_size(old->size);
    if (!new)return NULL;
    if (old->count <= 0)return new;
    memcpy(new->elements,old->elements,sizeof(GFC_ListElementData)*old->count);
    new->count = old->count;
    return new;
}

GFC_List *gfc_list_new_size(Uint32 count)
{
    GFC_List *l;
    if (!count)
    {
        slog("cannot make a list of size zero");
        return NULL;
    }
    l = (GFC_List *)malloc(sizeof(GFC_List));
    if (!l)
    {
        slog("failed to allocate space for the list");
        return NULL;
    }
    memset(l,0,sizeof(GFC_List));
    l->size = count;
    l->elements = gfc_allocate_array(sizeof(GFC_ListElementData),count);
    if (!l->elements)
    {
        slog("failed to allocate space for list elements");
        free(l);
        return NULL;
    }
    return l;
}

void gfc_list_set_nth(GFC_List *list,Uint32 n,void *data)
{
    if (!list)return;
    if ((n >= list->count)||(n >= list->size))return;
    list->elements[n].data = data;
}

void gfc_list_swap_indices(GFC_List *list,Uint32 a, Uint32 b)
{
    void *temp = NULL;
    if (!list)return;
    if (a == b)return;
    if ((a >= list->count)||(b >= list->count))return;
    if ((a >= list->size)||(b >= list->size))return;
    temp = list->elements[a].data;
    list->elements[a].data = list->elements[b].data;
    list->elements[b].data = temp;
}

void *gfc_list_get_nth(GFC_List *list,Uint32 n)
{
    if (!list)
    {
        return NULL;
    }
    if ((n >= list->count)||(n >= list->size))return NULL;
    return list->elements[n].data;
}

GFC_List *gfc_list_expand(GFC_List *list)
{
    GFC_List *l;
    if (!list)
    {
        slog("no list provided");
        return NULL;
    }
    if (!list->size)list->size = 8;
    l = gfc_list_new_size(list->size * 2);
    if (!l)
    {
        return list;
    }
    if (list->count > 0)
    {
        memcpy(l->elements,list->elements,sizeof(GFC_ListElementData)*list->count);
    }
    list->size = l->size;// update the new size
    free(list->elements);//free the old data
    list->elements = l->elements;//point to new memory address
    free(l);//free the temp list
    return list;//for backward compatibility
}

GFC_List *gfc_list_append(GFC_List *list,void *data)
{
    if (!list)
    {
        list = gfc_list_new();
    }
    if (list->count >= list->size)
    {
        list = gfc_list_expand(list);
        if (!list)
        {
            slog("append failed due to lack of memory");
            return NULL;
        }
    }
    list->elements[list->count].data = data;
    list->count++;
    return list;
}

GFC_List *gfc_list_concat(GFC_List *a,GFC_List *b)
{
    int i,count;
    void *data;
    if ((!a) || (!b))
    {
        slog("missing list data");
        return NULL;
    }
    count = gfc_list_get_count(b);
    for (i = 0; i < count;i++)
    {
        data = gfc_list_get_nth(b,i);
        a = gfc_list_append(a,data);
        if (a == NULL)return NULL;
    }
    return a;
}

GFC_List *gfc_list_concat_free(GFC_List *a,GFC_List *b)
{
    a = gfc_list_concat(a,b);
    if (a == NULL)return NULL;
    gfc_list_delete(b);
    return a;
}

GFC_List *gfc_list_prepend(GFC_List *list,void *data)
{
    return gfc_list_insert(list,data,0);
}

GFC_List *gfc_list_insert(GFC_List *list,void *data,Uint32 n)
{
    if (!list)
    {
        slog("no list provided");
        return NULL;
    }
    if (n > list->size + 1)
    {
        slog("attempting to insert element beyond length of list");
        return list;
    }
    if (list->count >= list->size)
    {
        list = gfc_list_expand(list);
        if (!list)return NULL;
    }
    memmove(&list->elements[n+1],&list->elements[n],sizeof(GFC_ListElementData)*(list->count - n));//copy all elements after n
    list->elements[n].data = data;
    list->count++;
    return list;
}


int gfc_list_delete_first(GFC_List *list)
{
    return gfc_list_delete_nth(list,0);
}

int gfc_list_delete_last(GFC_List *list)
{
    if (!list)
    {
        slog("no list provided");
        return -1;
    }
    return gfc_list_delete_nth(list,list->count-1);
}

int gfc_list_get_item_index(GFC_List *list,void *data)
{
    int i;
    if (!list)
    {
        slog("no list provided");
        return -1;
    }
    if (!data)return 0;
    for (i = 0; i < list->count;i++)
    {
        if (list->elements[i].data == data)
        {
            // found it
            return i;
        }
    }
    return -1;    
}

int gfc_list_delete_data(GFC_List *list,void *data)
{
    int i;
    if (!list)
    {
        slog("no list provided");
        return -1;
    }
    if (!data)return 0;
    for (i = 0; i < list->count;i++)
    {
        if (list->elements[i].data == data)
        {
            // found it, now delete it
            gfc_list_delete_nth(list,i);
            return 0;
        }
    }
    slog("data not found");
    return -1;
}

void gfc_list_clear(GFC_List *list)
{
    if (!list)return;
    memset(list->elements,0,list->size);//zero out all the data;
    list->count = 0;
}

int gfc_list_delete_nth(GFC_List *list,Uint32 n)
{
    if (!list)
    {
        slog("no list provided");
        return -1;
    }
    if (n >= list->count)
    {
        slog("attempting to delete beyond the length of the list");
        return -1;
    }
    if (n == (list->count - 1))
    {
        list->count--;// last element in the array, this is easy
        list->elements[n].data = NULL;
        return 0;
    }
    memmove(&list->elements[n],&list->elements[n+1],sizeof(GFC_ListElementData)*(list->count - n));//copy all elements after n
    list->count--;
    return 0;
}

Uint32 gfc_list_get_count(GFC_List *list)
{
    if (!list)return 0;
    return list->count;
}

void gfc_list_foreach(GFC_List *list,void (*function)(void *data))
{
    int i;
    if (!list)
    {
        return;
    }
    if (!function)
    {
        slog("no function provided");
    }
    for (i = 0;i < list->count;i++)
    {
        function(list->elements[i].data);
    }
}

void gfc_list_foreach_context(GFC_List *list,void (*function)(void *data,void *context),void *contextData)
{
    int i;
    if (!list)
    {
        slog("no list provided");
        return;
    }
    if (!function)
    {
        slog("no function provided");
    }
    for (i = 0;i < list->count;i++)
    {
        function(list->elements[i].data,contextData);
    }
}

/*eol@eof*/
