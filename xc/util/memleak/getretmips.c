/* lw $31,const($sp) is : 100 011 11101 11111 const */
/*                        1000 1111 1011 1111       */

#define RESTORE_RETURNVAL	0x8fbf0000
#define RESTORE_RETURNVAL_MASK	0xffff0000

/* addiu $sp, $sp, const is 001 001 11101 11101 const */
/*                          0010 0111 1011 1101 const */

#define ADJUST_STACKP		0x27bd0000
#define ADJUST_STACKP_MASK	0xffff0000

#define CALL(f)	    (0x0c000000 | (((int) (f)) >> 2))

#define HASH_SIZE   256

typedef struct _returnCache {
    unsigned long   *returnAddress;
    short	    raOffset;
    short	    spAdjust;
} ReturnCacheRec, *ReturnCachePtr;

static ReturnCacheRec	returnCache[HASH_SIZE];

#define HASH(ra)    ((((int) (ra)) >> 2) & (HASH_SIZE - 1))

typedef int Bool;
#define TRUE 1
#define FALSE 0

getStackTrace (results, max)
    unsigned long   *results;
    int		    max;
{
    extern unsigned long    *getReturnAddress (), *getStackPointer ();
    extern int	    main ();
    unsigned long   *ra;
    unsigned long   *sp;
    unsigned long   inst;
    unsigned long   mainCall;
    short	    ra_offset;
    short	    sp_adjust;
    Bool	    found_ra_offset, found_sp_adjust;
    ReturnCachePtr  rc;

    ra = getReturnAddress ();
    sp = getStackPointer ();
    mainCall = CALL(main);
    while (ra && max) {
	rc = &returnCache[HASH(ra)];
	if (rc->returnAddress != ra)
	{
	    found_ra_offset = FALSE;
	    found_sp_adjust = FALSE;
	    rc->returnAddress = ra;
	    while (!found_ra_offset || !found_sp_adjust)
	    {
	    	inst = *ra;
	    	if ((inst & RESTORE_RETURNVAL_MASK) == RESTORE_RETURNVAL)
	    	{
	    	    ra_offset = inst & ~RESTORE_RETURNVAL_MASK;
		    found_ra_offset = TRUE;
	    	}
	    	if ((inst & ADJUST_STACKP_MASK) == ADJUST_STACKP)
	    	{
	    	    sp_adjust = inst & ~ADJUST_STACKP_MASK;
		    found_sp_adjust = TRUE;
	    	}
	    	ra++;
	    }
	    rc->raOffset = ra_offset;
	    rc->spAdjust = sp_adjust;
	}
	ra = (unsigned long *) sp[rc->raOffset>>2];
	sp += rc->spAdjust >> 2;
	*results++ = ((unsigned long) ra) - 8;
	if (ra[-2] == mainCall)
	{
	    *results++ = 0;
	    break;
	}
	max--;
    }
}

#ifdef DEBUG
main ()
{
    foo ();
}

foo ()
{
    bar ();
}

bar ()
{
    unsigned long   trace[16];
    int		    i;

    getStackTrace (trace, sizeof trace / sizeof trace[0]);
    for (i = 0; i < 16 && trace[i]; i++) {
	printf ("%x\n", trace[i]);
    }
}
#endif
