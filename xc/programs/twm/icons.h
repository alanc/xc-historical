/**********************************************************************
 *
 * $XConsortium: icons.h,v 1.1 89/04/12 18:58:29 jim Exp $
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
    struct IconEntry	*first;
    struct IconEntry	*last;
} IconRegion;

typedef struct IconEntry
{
    struct IconEntry	*next;
    struct IconEntry	*prev;
    int			x, y, w, h;
    TwmWindow		*twm_win;
    short 		used;
}IconEntry;

#endif /* ICONS_H */
