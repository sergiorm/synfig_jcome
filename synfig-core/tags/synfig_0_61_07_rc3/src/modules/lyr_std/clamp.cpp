/* === S Y N F I G ========================================================= */
/*!	\file clamp.cpp
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
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "clamp.h"
#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(Layer_Clamp);
SYNFIG_LAYER_SET_NAME(Layer_Clamp,"clamp");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_Clamp,_("Clamp"));
SYNFIG_LAYER_SET_CATEGORY(Layer_Clamp,_("Filters"));
SYNFIG_LAYER_SET_VERSION(Layer_Clamp,"0.2");
SYNFIG_LAYER_SET_CVS_ID(Layer_Clamp,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

Layer_Clamp::Layer_Clamp():
	invert_negative(false),
	clamp_ceiling(true),
	ceiling(1.0f),
	floor(0.0f)
{
}

inline Color
Layer_Clamp::clamp_color(const Color &in)const
{
	Color ret(in);

	if(ret.get_a()==0)
		return Color::alpha();

	if(invert_negative)
	{
		if(ret.get_a()<floor)
			ret=-ret;

		if(ret.get_r()<floor)
		{
			ret.set_g(ret.get_g()-ret.get_r());
			ret.set_b(ret.get_b()-ret.get_r());
			ret.set_r(floor);
		}
		if(ret.get_g()<floor)
		{
			ret.set_r(ret.get_r()-ret.get_g());
			ret.set_b(ret.get_b()-ret.get_g());
			ret.set_g(floor);
		}
		if(ret.get_b()<floor)
		{
			ret.set_g(ret.get_g()-ret.get_b());
			ret.set_r(ret.get_r()-ret.get_b());
			ret.set_b(floor);
		}
	}
	else
	{
		if(ret.get_r()<floor) ret.set_r(floor);
		if(ret.get_g()<floor) ret.set_g(floor);
		if(ret.get_b()<floor) ret.set_b(floor);
		if(ret.get_a()<floor) ret.set_a(floor);
	}

	if(clamp_ceiling)
	{
		if(ret.get_r()>ceiling) ret.set_r(ceiling);
		if(ret.get_g()>ceiling) ret.set_g(ceiling);
		if(ret.get_b()>ceiling) ret.set_b(ceiling);
		if(ret.get_a()>ceiling) ret.set_a(ceiling);
	}
	return ret;
}

bool
Layer_Clamp::set_param(const String & param, const ValueBase &value)
{
	IMPORT(invert_negative);
	IMPORT(clamp_ceiling);
	IMPORT(ceiling);
	IMPORT(floor);

	return false;
}

ValueBase
Layer_Clamp::get_param(const String &param)const
{
	EXPORT(invert_negative);
	EXPORT(clamp_ceiling);

	EXPORT(ceiling);
	EXPORT(floor);

	EXPORT_NAME();
	EXPORT_VERSION();

	return ValueBase();
}

Layer::Vocab
Layer_Clamp::get_param_vocab()const
{
	Layer::Vocab ret;

	ret.push_back(ParamDesc("invert_negative")
		.set_local_name(_("Invert Negative"))
	);

	ret.push_back(ParamDesc("clamp_ceiling")
		.set_local_name(_("Clamp Ceiling"))
	);

	ret.push_back(ParamDesc("ceiling")
		.set_local_name(_("Ceiling"))
	);

	ret.push_back(ParamDesc("floor")
		.set_local_name(_("Floor"))
	);

	return ret;
}

Color
Layer_Clamp::get_color(Context context, const Point &pos)const
{
	return clamp_color(context.get_color(pos));
}

bool
Layer_Clamp::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	SuperCallback supercb(cb,0,9500,10000);

	if(!context.accelerated_render(surface,quality,renddesc,&supercb))
		return false;

	int x,y;

	Surface::pen pen(surface->begin());

	for(y=0;y<renddesc.get_h();y++,pen.inc_y(),pen.dec_x(x))
		for(x=0;x<renddesc.get_w();x++,pen.inc_x())
			pen.put_value(clamp_color(pen.get_value()));

	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;

	return true;
}


Rect
Layer_Clamp::get_full_bounding_rect(Context context)const
{
	return context.get_full_bounding_rect();
}