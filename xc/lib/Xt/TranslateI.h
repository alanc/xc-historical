/* $XConsortium: TranslateI.h,v 1.30 91/01/21 09:13:23 rws Exp $ */

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* 
 * TranslateI.h - Header file private to translation management
 * 
 * Author:	Charles Haynes
 * 		Digital Equipment Corporation
 * 		Western Research Laboratory
 * Date:	Sat Aug 29 1987
 */

#ifndef _TranslateI_h
#define _TranslateI_h

/*#define REFCNT_TRANSLATIONS*/
#define CACHE_TRANSLATIONS

#define TM_NO_MATCH (-2)

#define _XtRStateTablePair "_XtStateTablePair"

typedef unsigned char TMByteCard;
typedef unsigned short TMShortCard;
typedef unsigned long TMLongCard;
typedef short TMShortInt;

typedef Boolean (*MatchProc)();
    /* Event *parsed, TMEventPtr incoming */

typedef struct _ModToKeysymTable {
    Modifiers mask;
    int count;
    int index;
} ModToKeysymTable;

typedef struct _LateBindings {
    unsigned int knot:1;
    unsigned int pair:1;
    KeySym keysym;
} LateBindings, *LateBindingsPtr;

typedef short ModifierMask;

typedef struct _ActionsRec *ActionPtr;
typedef struct _ActionsRec {
    int index;			/* index into quarkTable to find proc */
    String *params;		/* pointer to array of params */
    Cardinal num_params;	/* number of params */
    ActionPtr next;		/* next action to perform */
} ActionRec;

typedef struct _StateRec {
    unsigned int	isCycleStart:1;
    unsigned int	isCycleEnd:1;
    TMShortCard		typeIndex;
    TMShortCard		modIndex;
    ActionPtr		actions;	/* rhs list of actions to perform */
    StatePtr 		nextLevel;
}StateRec;


#define XtTableReplace	0
#define XtTableAugment	1
#define XtTableOverride	2
#define XtTableUnmerge  3

typedef unsigned int _XtTranslateOp;

/*
 * New Definitions for TMLite
 */
typedef struct _TMModifierMatchRec{
    TMLongCard	 modifiers;
    TMLongCard	 modifierMask;
    LateBindingsPtr lateModifiers;
    Boolean	 standard;
}TMModifierMatchRec, *TMModifierMatch;

typedef struct _TMTypeMatchRec{
    TMLongCard	 eventType;
    TMLongCard	 eventCode;
    TMLongCard	 eventCodeMask;
    MatchProc	 matchEvent;
}TMTypeMatchRec, *TMTypeMatch;

typedef struct _TMBranchHeadRec {
    unsigned int	isSimple:1;
    unsigned int	hasActions:1;
    unsigned int	hasCycles:1;
    int			more:13;
    TMShortCard		typeIndex;
    TMShortCard		modIndex;
}TMBranchHeadRec, *TMBranchHead;

/* NOTE: elements of this structure must match those of
 * TMComplexStateTreeRec and TMParseStateTreeRec.
 */
typedef struct _TMSimpleStateTreeRec{
    unsigned int	isSimple:1;
    unsigned int	isAccelerator:1;
    unsigned int	mappingNotifyInterest:1;
    unsigned int	refCount:13;
    TMShortCard		numBranchHeads;
    TMShortCard		numQuarks;   /* # of entries in quarkTbl */
    TMShortCard		unused;	     /* to ensure same alignment */
    TMBranchHeadRec	*branchHeadTbl;
    XrmQuark		*quarkTbl;  /* table of quarkified rhs*/
}TMSimpleStateTreeRec, *TMSimpleStateTree;    

/* NOTE: elements of this structure must match those of
 * TMSimpleStateTreeRec and TMSimpleStateTreeRec.
 */
typedef struct _TMComplexStateTreeRec{
    unsigned int	isSimple:1;
    unsigned int	isAccelerator:1;
    unsigned int	mappingNotifyInterest:1;
    unsigned int	refCount:13;
    TMShortCard		numBranchHeads;
    TMShortCard		numQuarks;   /* # of entries in quarkTbl */
    TMShortCard		numComplexBranchHeads;
    TMBranchHeadRec	*branchHeadTbl;
    XrmQuark		*quarkTbl;  /* table of quarkified rhs*/
    StatePtr		*complexBranchHeadTbl;
}TMComplexStateTreeRec, *TMComplexStateTree;    

/* NOTE: elements of this structure must match those of
 * TMSimpleStateTreeRec and TMComplexStateTreeRec.
 */
typedef struct _TMParseStateTreeRec{
    unsigned int	isSimple:1;
    unsigned int	isAccelerator:1;
    unsigned int	mappingNotifyInterest:1;
    unsigned int	isStackQuarks:1;
    unsigned int	isStackBranchHeads:1;
    unsigned int	isStackComplexBranchHeads:1;
    unsigned int	unused:10; /* to ensure correct alignment */
    TMShortCard		numBranchHeads;
    TMShortCard		numQuarks;   /* # of entries in quarkTbl */
    TMShortCard		numComplexBranchHeads;
    TMBranchHeadRec	*branchHeadTbl;
    XrmQuark		*quarkTbl;  /* table of quarkified rhs*/
    StatePtr		*complexBranchHeadTbl;
    TMShortCard		branchHeadTblSize;
    TMShortCard		quarkTblSize; /*total size of quarkTbl */
    TMShortCard		complexBranchHeadTblSize;
    StatePtr		head;
}TMParseStateTreeRec, *TMParseStateTree;    

typedef union _TMStateTreeRec{
    TMSimpleStateTreeRec	simple;
    TMParseStateTreeRec		parse;
    TMComplexStateTreeRec	complex;
}*TMStateTree, **TMStateTreePtr, **TMStateTreeList;

typedef struct _TMSimpleBindProcsRec {
    XtActionProc	*procs;
}TMSimpleBindProcsRec, *TMSimpleBindProcs;

typedef struct _TMComplexBindProcsRec {
    Widget	 	widget;		/*widgetID to pass to action Proc*/
    XtTranslations	aXlations;
    XtActionProc	*procs;
}TMComplexBindProcsRec, *TMComplexBindProcs;

/* NOTE: elements of this structure must match those of TMComplexBindDataRec */
typedef struct _TMSimpleBindDataRec {
    unsigned int		isComplex:1;
    TMSimpleBindProcsRec	bindTbl[1]; /* WARNING, variable length */
}TMSimpleBindDataRec, *TMSimpleBindData;

/* NOTE: elements of this structure must match those of TMSimpleBindDataRec */
typedef struct _TMComplexBindDataRec {
    unsigned int		isComplex:1;
    TMComplexBindProcsRec	bindTbl[1]; /* WARNING, variable length */
}TMComplexBindDataRec, *TMComplexBindData;

typedef union _TMBindDataRec{
    TMSimpleBindDataRec		simple;
    TMComplexBindDataRec	complex;
}*TMBindData;

typedef struct _TranslationData{
    unsigned char		operation; /*replace,augment,override*/
    TMShortCard			numStateTrees;
    struct _TranslationData    	*composers[2];
    EventMask			eventMask;
    TMStateTree			stateTreeTbl[1]; /* WARNING, variable length */
}TranslationData;

typedef struct _TMConvertRec {
    XtTranslations	old; /* table to merge into */
    XtTranslations	new; /* table to merge from */
} TMConvertRec;

#define _XtEventTimerEventType ((TMLongCard)~0L)
#define KeysymModMask		(1L<<27) /* private to TM */
#define AnyButtonMask		(1L<<28) /* private to TM */

typedef struct _EventRec {
    TMLongCard modifiers;
    TMLongCard modifierMask;
    LateBindingsPtr lateModifiers;
    TMLongCard eventType;
    TMLongCard eventCode;
    TMLongCard eventCodeMask;
    MatchProc matchEvent;
    Boolean standard;
} Event;

typedef struct _EventSeqRec *EventSeqPtr;
typedef struct _EventSeqRec {
    Event event;	/* X event description */
    StatePtr state;	/* private to state table builder */
    EventSeqPtr next;	/* next event on line */
    ActionPtr actions;	/* r.h.s.   list of actions to perform */
} EventSeqRec;

typedef EventSeqRec EventRec;
typedef EventSeqPtr EventPtr;

typedef struct _TMEventRec {
    XEvent *xev;
    Event event;
}TMEventRec,*TMEventPtr;

typedef struct _ActionHookRec {
    struct _ActionHookRec* next; /* must remain first */
    XtAppContext app;
    XtActionHookProc proc;
    XtPointer closure;
} ActionHookRec, *ActionHook;

/* choose a number between 2 and 8 */
#define TMKEYCACHELOG2 6
#define TMKEYCACHESIZE (1<<TMKEYCACHELOG2)

typedef struct _KeyCacheRec {
    Modifiers modifiers_return; /* constant for a give XtKeyProc */
    KeyCode keycode[TMKEYCACHESIZE];
    unsigned short modifiers[TMKEYCACHESIZE];
    KeySym keysym[TMKEYCACHESIZE];
} TMKeyCache;

typedef struct _TMKeyContextRec {
    XEvent *event;
    unsigned long serial;
    KeySym keysym;
    Modifiers modifiers;
    TMKeyCache keycache;  /* keep this last, to keep offsets to others small */
} TMKeyContextRec, *TMKeyContext;

typedef struct _TMGlobalRec{
    TMTypeMatchRec 		**typeMatchSegmentTbl;
    TMShortCard			numTypeMatches;
    TMShortCard			numTypeMatchSegments;
    TMShortCard			typeMatchSegmentTblSize;
    TMModifierMatchRec 		**modMatchSegmentTbl;
    TMShortCard			numModMatches;
    TMShortCard			numModMatchSegments;
    TMShortCard			modMatchSegmentTblSize;
    Boolean			newMatchSemantics;
#ifdef TRACE_TM
    XtTranslations		*tmTbl;
    TMShortCard			numTms;
    TMShortCard			tmTblSize;
    struct _TMBindCacheRec	**bindCacheTbl;
    TMShortCard			numBindCache;
    TMShortCard			bindCacheTblSize;
    TMShortCard			numLateBindings;
    TMShortCard			numBranchHeads;
    TMShortCard			numComplexStates;
    TMShortCard			numComplexActions;
#endif /* TRACE_TM */
}TMGlobalRec;

extern TMGlobalRec _XtGlobalTM;

#define TM_MOD_SEGMENT_SIZE 	16
#define TM_TYPE_SEGMENT_SIZE 	16

#define TMGetTypeMatch(index) \
  ((TMTypeMatch) \
   &((_XtGlobalTM.typeMatchSegmentTbl[((index) >> 4)])[(index) & 15]))
#define TMGetModifierMatch(index) \
  ((TMModifierMatch) \
   &((_XtGlobalTM.modMatchSegmentTbl[(index) >> 4])[(index) & 15]))

/* Useful Access Macros */
#define TMNewMatchSemantics() (_XtGlobalTM.newMatchSemantics)
#define TMBranchMore(branch) (branch->more)
#define TMComplexBranchHead(tree, br) \
  (((TMComplexStateTree)tree)->complexBranchHeadTbl[TMBranchMore(br)])

#define TMGetComplexBindEntry(bindData, idx) \
  ((TMComplexBindProcs)&(((TMComplexBindData)bindData)->bindTbl[idx]))

#define TMGetSimpleBindEntry(bindData, idx) \
  ((TMSimpleBindProcs)&(((TMSimpleBindData)bindData)->bindTbl[idx]))


#define _InitializeKeysymTables(dpy, pd) \
    if (pd->keysyms == NULL) \
        _XtBuildKeysymTables(dpy, pd)

/* 
 * Internal Functions
 */

#if NeedWidePrototypes
#define Boolean int
#endif

extern void _XtPopup(
#if NeedFunctionPrototypes
    Widget      /* widget */,
    XtGrabKind  /* grab_kind */,
    Boolean     /* spring_loaded */
#endif
);

extern String _XtPrintXlations(
#if NeedFunctionPrototypes
    Widget		/* w */,
    XtTranslations 	/* xlations */,
    Widget		/* accelWidget */,
    Boolean		/* includeRHS */
#endif
);

extern void _XtRegisterGrabs(
#if NeedFunctionPrototypes
    Widget	/* widget */
#endif
);

#undef Boolean


extern void _XtAddEventSeqToStateTree();

extern void _XtInitializeStateTable(); /* stateTable */
    /* _XtTranslations *stateTable; */

extern Boolean _XtMatchUsingStandardMods();
extern Boolean _XtMatchUsingDontCareMods();
extern Boolean _XtRegularMatch();
extern Boolean _XtMatchAtom();
extern void _XtConvertCase();
extern void _XtBuildKeysymTables();
#ifndef NO_MIT_HACKS
extern void  _XtDisplayTranslations();
extern void  _XtDisplayAccelerators();
extern void _XtDisplayInstalledAccelerators();
#endif
extern void _XtPopupInitialize();

extern void _XtBindActions(
#if NeedFunctionPrototypes
    Widget	/* widget */,
    XtTM 	/* tm_rec */
#endif
);

extern Boolean _XtComputeLateBindings(
#if NeedFunctionPrototypes
    LateBindingsPtr	/* lateModifiers */,
    TMEventPtr		/* eventSeq */,
    Modifiers*		/* computed */,
    Modifiers*		/* computedMask */
#endif
);

extern XtTranslations _XtCreateXlations(
#if NeedFunctionPrototypes
    TMStateTree *	/* stateTrees */,
    TMShortCard		/* numStateTrees */,
    XtTranslations 	/* first */,
    XtTranslations	/* second */
#endif
);

extern Boolean _XtCvtMergeTranslations(
#if NeedFunctionPrototypes
    Display*	/* dpy */,
    XrmValuePtr	/* args */,
    Cardinal*	/* num_args */,
    XrmValuePtr	/* from */,
    XrmValuePtr	/* to */,
    XtPointer*	/* closure_ret */
#endif
);

void _XtFreeTranslations(
#if NeedFunctionPrototypes
    XtAppContext	/* app */,
    XrmValuePtr		/* toVal */,
    XtPointer		/* closure */,
    XrmValuePtr		/* args */,
    Cardinal*		/* num_args */
#endif
);

extern TMShortCard _XtGetModifierIndex(
#if NeedFunctionPrototypes
    Event*	/* event */
#endif
);
   
extern TMShortCard _XtGetQuarkIndex(
#if NeedFunctionPrototypes
    TMParseStateTree	/* stateTreePtr */,
    XrmQuark		/* quark */
#endif
);

extern TMShortCard _XtGetTypeIndex(
#if NeedFunctionPrototypes
    Event*	/* event */
#endif
);

extern void _XtGrabInitialize(
#if NeedFunctionPrototypes
    XtAppContext	/* app */
#endif
);

extern XtPointer _XtInitializeActionData(
#if NeedFunctionPrototypes
    struct _XtActionsRec *	/* actions */,
    Cardinal 			/* count */,
    Boolean			/* inPlace */
#endif
);

extern void _XtInstallTranslations(
#if NeedFunctionPrototypes
    Widget		/* widget */
#endif
);

extern void _XtMergeTranslations(
#if NeedFunctionPrototypes
    Widget		/* widget */,
    XtTranslations	/* newXlations */
#endif
);

extern TMStateTree _XtParseTreeToStateTree(
#if NeedFunctionPrototypes
    TMParseStateTree 	/* parseTree */
#endif
);

extern String _XtPrintActions(
#if NeedFunctionPrototypes
    ActionRec*	/* actions */,
    XrmQuark*	/* quarkTbl */
#endif
);

extern String _XtPrintEventSeq(
#if NeedFunctionPrototypes
    EventSeqPtr	/* eventSeq */,
    Display*	/* dpy */
#endif
);

typedef Boolean (*_XtTraversalProc)(
#if NeedFunctionPrototypes
    StatePtr	/* state */,
    XtPointer	/* data */
#endif
);
				    
extern void _XtTraverseStateTree(
#if NeedFunctionPrototypes
    TMStateTree		/* tree */,
    _XtTraversalProc	/* func */,				 
    XtPointer		/* data */
#endif
);

extern void _XtSetTMOperation(
#if NeedFunctionPrototypes
    XtTranslations	/* xlations */,
    _XtTranslateOp	/* operation */
#endif
);

extern void _XtTranslateInitialize(
#if NeedFunctionPrototypes
    void
#endif
);

extern void _XtUnbindActions(
#if NeedFunctionPrototypes
    Widget		/* widget */,
    XtTranslations	/* xlations */,
    TMBindData		/* bindData */
#endif
);

extern void _XtUnmergeTranslations(
#if NeedFunctionPrototypes
    Widget		/* widget */,
    XtTranslations 	/* xlations */
#endif
);

#endif /* TranslateI_h */
