/*
 * Copyright (c) 2000-2001 Greg Haerr <greg@censoft.com>
 *
 * Device-independent arc, pie and ellipse routines.
 * GdArc is integer only and requires start/end points.
 * GdArcAngle requires floating point and uses angles.
 * GdArcAngle uses qsin() and qcos() instead of sin() / cos() 
 * so no math lib needed.
 *
 * Portions Copyright (c) 1991 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Arc line clipping and integer qsin/qcos routines used by permission:
 * Copyright (C) 1997-1998 by Eero Tamminen
 * Bugfixed by Greg Haerr
 */

#include <stdio.h>
#include "device.h"

/* argument holder for pie, arc and ellipse functions*/
typedef struct {
	PSD	psd;
	MWCOORD	x0, y0;
	MWCOORD	rx, ry;
	MWCOORD	ax, ay;
	MWCOORD	bx, by;
	int	adir, bdir;
	int	type;
} SLICE;

extern void drawpoint(PSD psd, MWCOORD x, MWCOORD y);
extern void drawrow(PSD psd, MWCOORD x1, MWCOORD x2, MWCOORD y);

/*
 * Clip a line segment for arc or pie drawing.
 * Returns 0 if line is clipped or on acceptable side, 1 if it's vertically
 * on other side, otherwise 3.
 */
static int
clip_line(SLICE *slice, MWCOORD xe, MWCOORD ye, int dir, MWCOORD y, MWCOORD *x0,
	MWCOORD *x1)
{
#if 0
	/*
	 * kluge: handle 180 degree case
	 */
	if (y >= 0 && ye == 0) {
/*printf("cl %d,%d %d,%d %d,%d %d,%d %d,%d\n", xe, ye, y, dir,
slice->ax, slice->ay, slice->bx, slice->by, slice->adir, slice->bdir);*/
		/* bottom 180*/
		if (slice->adir < 0) {
			if (slice->ay || slice->by)
				return 1;
			if (slice->ax == -slice->bx)
				return 0;
		}
		return 3;
	}
#endif
	/* hline on the same vertical side with the given edge? */
	if ((y >= 0 && ye >= 0) || (y < 0 && ye < 0)) {
		MWCOORD x;

		if (ye == 0) x = xe; else
		x = (MWCOORD)(long)xe * y / ye;

		if (x >= *x0 && x <= *x1) {
			if (dir > 0)
				*x0 = x;
			else
				*x1 = x;
			return 0;
		} else {
			if (dir > 0) {
				if (x <= *x0)
					return 0;
			} else {
				if (x >= *x1)
					return 0;
			}
		}
		return 3;
	}
	return 1;
}

/* relative offsets, direction from left to right. */
static void
draw_line(SLICE *slice, MWCOORD x0, MWCOORD y, MWCOORD x1)
{
	int	dbl = (slice->adir > 0 && slice->bdir < 0);
	int 	discard, ret;
	MWCOORD	x2 = x0, x3 = x1;

	if (y == 0) {
		if (slice->type != MWPIE)
			return;
		/* edges on different sides */
		if ((slice->ay <= 0 && slice->by >= 0) ||
		    (slice->ay >= 0 && slice->by <= 0)) {
			if (slice->adir < 0)  {
				if (x1 > 0)
					x1 = 0;
			}
			if (slice->bdir > 0) {
				if (x0 < 0)
					x0 = 0;
			}
		} else {
			if (!dbl) {
				/* FIXME leaving in draws dot in center*/
				drawpoint(slice->psd, slice->x0, slice->y0);
				return;
			}
		}
		drawrow(slice->psd, slice->x0 + x0, slice->x0 + x1, slice->y0);
		return;
	}

	/* clip left edge / line */
	ret = clip_line(slice, slice->ax, slice->ay, slice->adir, y, &x0, &x1);

	if (dbl) {
		if (!ret) {
			/* edges separate line to two parts */
			drawrow(slice->psd, slice->x0 + x0, slice->x0 + x1,
				slice->y0 + y);
			x0 = x2;
			x1 = x3;
		}
	} else {
		if (ret > 1) {
			return;
		}
	}

	discard = ret;
	ret = clip_line(slice, slice->bx, slice->by, slice->bdir, y, &x0, &x1);

	discard += ret;
	if (discard > 2 && !(dbl && ret == 0 && discard == 3)) {
		return;
	}
	if (discard == 2) {
		/* line on other side than slice */
		if (slice->adir < 0 || slice->bdir > 0) {
			return;
		}
	}
	drawrow(slice->psd, slice->x0 + x0, slice->x0 + x1, slice->y0 + y);
}

/* draw one line segment or set of points, called from drawarc routine*/
static void
drawarcsegment(SLICE *slice, MWCOORD xp, MWCOORD yp)
{
	switch (slice->type) {
	case MWELLIPSEFILL:
		/* draw ellipse fill segment*/
		drawrow(slice->psd, slice->x0-xp, slice->x0+xp, slice->y0-yp);
		drawrow(slice->psd, slice->x0-xp, slice->x0+xp, slice->y0+yp);
		return;

	case MWELLIPSE:
		/* set four points symmetrically situated around a point*/
		drawpoint(slice->psd, slice->x0 + xp, slice->y0 + yp);
		drawpoint(slice->psd, slice->x0 - xp, slice->y0 + yp);
		drawpoint(slice->psd, slice->x0 + xp, slice->y0 - yp);
		drawpoint(slice->psd, slice->x0 - xp, slice->y0 - yp);
		return;

	case MWPIE:
		/* draw top and bottom halfs of pie*/
		draw_line(slice, -xp, -yp, +xp);
		draw_line(slice, -xp, +yp, +xp);
		return;

	default:	/* MWARC, MWARCOUTLINE*/
		/* set four points symmetrically around a point and clip*/
		draw_line(slice, +xp, +yp, +xp);
		draw_line(slice, -xp, +yp, -xp);
		draw_line(slice, +xp, -yp, +xp);
		draw_line(slice, -xp, -yp, -xp);
		return;
	}
}

/* General routine to plot points on an arc.  Used by arc, pie and ellipse*/
static void
drawarc(SLICE *slice)
{
	MWCOORD xp, yp;		/* current point (based on center) */
	MWCOORD rx, ry;
	long Asquared;		/* square of x semi axis */
	long TwoAsquared;
	long Bsquared;		/* square of y semi axis */
	long TwoBsquared;
	long d;
	long dx, dy;

	rx = slice->rx;
	ry = slice->ry;

	xp = 0;
	yp = ry;
	Asquared = rx * rx;
	TwoAsquared = 2 * Asquared;
	Bsquared = ry * ry;
	TwoBsquared = 2 * Bsquared;
	d = Bsquared - Asquared * ry + (Asquared >> 2);
	dx = 0;
	dy = TwoAsquared * ry;

	while (dx < dy) {
		drawarcsegment(slice, xp, yp);
		if (d > 0) {
			yp--;
			dy -= TwoAsquared;
			d -= dy;
		}
		xp++;
		dx += TwoBsquared;
		d += (Bsquared + dx);
	}

	d += ((3L * (Asquared - Bsquared) / 2L - (dx + dy)) >> 1);

	while (yp >= 0) {
		drawarcsegment(slice, xp, yp);
		if (d < 0) {
			xp++;
			dx += TwoBsquared;
			d += dx;
		}
		yp--;
		dy -= TwoAsquared;
		d += (Asquared - dy);
	}

}

/* 
 * Draw an arc or pie using start/end points.
 * Integer only routine.  To specify start/end angles, 
 * use GdArcAngle, which requires floating point.
 */
void
GdArc(PSD psd, MWCOORD x0, MWCOORD y0, MWCOORD rx, MWCOORD ry,
	MWCOORD ax, MWCOORD ay, MWCOORD bx, MWCOORD by, int type)
{
	MWCOORD	adir, bdir;
	SLICE	slice;

	if (rx <= 0 || ry <= 0)
		return;

	/*
	 * Calculate right/left side clipping, based on quadrant.
	 * dir is positive when right side is filled and negative when
	 * left side is to be filled.
	 *
	 * >= 0 is bottom half
	 */
	if (ay >= 0)
		adir = 1;
	else
		adir = -1;

	if (by >= 0)
		bdir = -1;
	else
		bdir = 1;

	/*
	 * The clip_line routine has problems around the 0 and
	 * 180 degree axes.
	 * This <fix> is required to make the clip_line algorithm
	 * work.  Getting these routines to work for all angles is
	 * a bitch.  And they're still buggy.  Doing this causes
	 * half circles to be outlined with a slightly bent line
	 * on the x axis. FIXME
	 */
	if (ay == 0) ++ay;
	if (by == 0) ++by;

	/* swap rightmost edge first */
	if (bx > ax) {
		MWCOORD swap;

		swap = ax;
		ax = bx;
		bx = swap;

		swap = ay;
		ay = by;
		by = swap;

		swap = adir;
		adir = bdir;
		bdir = swap;
	}

	/* check for entire area clipped, draw with per-point clipping*/
	if (GdClipArea(psd, x0-rx, y0-ry, x0+rx, y0+ry) == CLIP_INVISIBLE)
		return;

	slice.psd = psd;
	slice.x0 = x0;
	slice.y0 = y0;
	slice.rx = rx;
	slice.ry = ry;
	slice.ax = ax;
	slice.ay = ay;
	slice.bx = bx;
	slice.by = by;
	slice.adir = adir;
	slice.bdir = bdir;
	slice.type = type;

	drawarc(&slice);

	if (type & MWOUTLINE) {
		/* draw two lines from rx,ry to arc endpoints*/
		GdLine(psd, x0, y0, x0+ax, y0+ay, TRUE);
		GdLine(psd, x0, y0, x0+bx, y0+by, TRUE);
	}

// by yuanlii@29bbs.net, 1/27/2005 11:19:01
//	GdFixCursor(psd);
// by yuanlii@29bbs.net
}

/*
 * Draw an ellipse using the current clipping region and foreground color.
 * This draws in the outline of the ellipse, or fills it.
 * Integer only routine.
 */
void
GdEllipse(PSD psd, MWCOORD x, MWCOORD y, MWCOORD rx, MWCOORD ry, MWBOOL fill)
{
	SLICE	slice;

	if (rx < 0 || ry < 0)
		return;

	/* Check if the ellipse bounding box is either totally visible
	 * or totally invisible.  Draw with per-point clipping.
	 */
	switch (GdClipArea(psd, x - rx, y - ry, x + rx, y + ry)) {
	case CLIP_VISIBLE:
		/*
		 * For size considerations, there's no low-level ellipse
		 * draw, so we've got to draw all ellipses
		 * with per-point clipping for the time being
		psd->DrawEllipse(psd, x, y, rx, ry, fill, gr_foreground);
		GdFixCursor(psd);
		return;
		 */
		break;

	case CLIP_INVISIBLE:
		return;
  	}

	slice.psd = psd;
	slice.x0 = x;
	slice.y0 = y;
	slice.rx = rx;
	slice.ry = ry;
	slice.type = fill? MWELLIPSEFILL: MWELLIPSE;
	/* other elements unused*/

	drawarc(&slice);

// by yuanlii@29bbs.net, 1/27/2005 11:09:30
//	GdFixCursor(psd);
// by yuanlii@29bbs.net
}
