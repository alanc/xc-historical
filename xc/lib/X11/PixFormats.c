#include "copyright.h"

#include <stdio.h>
#include "Xlibint.h"

/*
 * XListPixmapFormats - return info from connection setup
 */

XPixmapFormatValues *XListPixmapFormats (dpy, count)
    Display *dpy;
    int *count;
{
    XPixmapFormatValues *formats =
      (XPixmapFormatValues *) Xmalloc (dpy->nformats *
				       sizeof (XPixmapFormatValues));

    *count = dpy->nformats;

    if (formats) {
	register int i;
	register XPixmapFormatValues *f;
	register ScreenFormat *sf;

	/*
	 * copy data from internal Xlib data structure in display
	 */
	for (i = dpy->nformats, f = formats, sf = dpy->pixmap_format; i > 0;
	     i--, f++, sf++) {
	    f->depth = sf->depth;
	    f->bits_per_pixel = sf->bits_per_pixel;
	    f->scanline_pad = sf->scanline_pad;
	}
    }
    return formats;
}
