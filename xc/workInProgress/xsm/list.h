typedef struct _List {
    struct _List	*prev;
    struct _List	*next;
    void	*thing;
} List;

#if	0
extern List *ListInit();
extern List *ListFirst(List *);
extern List *ListNext(List *);
extern void ListFreeAll(List *);
extern void ListFreeOne(List *);
extern List *ListAddFirst(List *, void *);
extern List *ListAddLast(List *, void *);
extern int ListCount(List *);
#else
extern List *ListInit();
extern List *ListFirst();
extern List *ListNext();
extern void ListFreeAll();
extern void ListFreeOne();
extern List *ListAddFirst();
extern List *ListAddLast();
extern int ListCount();
#endif
