/**********************************************************************
 *
 * $XConsortium: icons.h,v 1.2 89/04/13 15:48:22 jim Exp $
 *
 * Icon releated definitions
 *
 * 10-Apr-89 Tom LaStrange        Initial Version.
 *
 **********************************************************************/

#ifndef ICONS_H
#define ICONS_H

typedef struct IconRegion
{
    struct IconRegion	*next;
    int			x, y, w, h;
    int			grav1, grav2;
    int			stepx, stepy;	/* allocation granularity */
    struct IconEntry	*entries;
} IconRegion;

typedef struct IconEntry
{
    struct IconEntry	*next;
    int			x, y, w, h;
    TwmWindow		*twm_win;
    short 		used;
}IconEntry;

#endif /* ICONS_H */
