#ifndef CLOSESTR_H
#define CLOSESTR_H

#define	NEED_REPLIES
#include "Xproto.h"
#include "closure.h"
#include "dix.h"
#include "misc.h"

/* closure structures */
typedef struct _OFclosure {
    ClientPtr   client;
    short       current_fpe;
    short       num_fpes;
    FontPathElementPtr *fpe_list;
    Mask        flags;
    Bool        slept;

/* XXX -- get these from request buffer instead? */
    XID         fontid;
    char       *fontname;
    int         fnamelen;
}           OFclosureRec;

typedef struct _LFclosure {
    ClientPtr   client;
    int         npaths;
    short       current_fpe;
    short       num_fpes;
    FontPathElementPtr *fpe_list;
    FontPathPtr *all_paths;
    FontPathPtr paths;
    char       *pattern;
    int         maxnames;
    int         patlen;
    Bool        slept;
}           LFclosureRec;

typedef struct _LFWIclosure {
    ClientPtr   client;
    short       current_fpe;
    short       num_fpes;
    FontPathElementPtr *fpe_list;
    xListFontsWithInfoReply *reply;
    int         length;
    char       *pattern;
    int         maxNames;
    int         patlen;
    int         numfonts;
    char       *fontname;
    int         namelen;
    Bool        slept;
    pointer     private;
}           LFWIclosureRec;

#endif				/* CLOSESTR_H */
