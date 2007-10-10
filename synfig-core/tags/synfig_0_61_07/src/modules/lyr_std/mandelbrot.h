/* === S Y N F I G ========================================================= */
/*!	\file mandelbrot.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
**
** === N O T E S ===========================================================
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_MANDELBROT_H
#define __SYNFIG_MANDELBROT_H

/* === H E A D E R S ======================================================= */

#include <synfig/layer.h>
#include <synfig/color.h>
#include <synfig/angle.h>
#include <synfig/gradient.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

using namespace synfig;
using namespace std;
using namespace etl;

class Mandelbrot : public Layer
{
	SYNFIG_LAYER_MODULE_EXT

private:

	Real bailout;
	Real lp;
	int iterations;

	bool smooth_outside;
	bool broken;

	bool distort_inside;
	bool distort_outside;
	bool solid_inside;
	bool solid_outside;
	bool invert_inside;
	bool invert_outside;
	bool shade_outside;
	bool shade_inside;
	Real gradient_offset_inside;
	Real gradient_offset_outside;
	bool gradient_loop_inside;
	Real gradient_scale_outside;
	Gradient gradient_inside;
	Gradient gradient_outside;

public:
	Mandelbrot();

	virtual bool set_param(const String &param, const ValueBase &value);
	virtual ValueBase get_param(const String &param)const;
	virtual Color get_color(Context context, const Point &pos)const;
	virtual Vocab get_param_vocab()const;
};

/* === E N D =============================================================== */

#endif