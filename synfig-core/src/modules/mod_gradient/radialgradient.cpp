/* === S Y N F I G ========================================================= */
/*!	\file mod_gradient/radialgradient.cpp
**	\brief Implementation of the "Radial Gradient" layer
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
**	Copyright (c) 2011 Carlos López
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

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>

#include "radialgradient.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace std;
using namespace synfig;

/* === G L O B A L S ======================================================= */

SYNFIG_LAYER_INIT(RadialGradient);
SYNFIG_LAYER_SET_NAME(RadialGradient,"radial_gradient");
SYNFIG_LAYER_SET_LOCAL_NAME(RadialGradient,N_("Radial Gradient"));
SYNFIG_LAYER_SET_CATEGORY(RadialGradient,N_("Gradients"));
SYNFIG_LAYER_SET_VERSION(RadialGradient,"0.1");
SYNFIG_LAYER_SET_CVS_ID(RadialGradient,"$Id$");

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

RadialGradient::RadialGradient():
	Layer_Composite(1.0,Color::BLEND_COMPOSITE),
	gradient(Color::black(),Color::white()),
	center(0,0),
	radius(0.5),
	loop(false),
	zigzag(false)
{
	Layer::Vocab voc(get_param_vocab());
	Layer::fill_static(voc);
}

bool
RadialGradient::set_param(const String & param, const ValueBase &value)
{
	IMPORT(gradient);
	IMPORT(center);
	IMPORT(radius);
	IMPORT(loop);
	IMPORT(zigzag);

	return Layer_Composite::set_param(param,value);
}

ValueBase
RadialGradient::get_param(const String &param)const
{
	EXPORT(gradient);
	EXPORT(center);
	EXPORT(radius);
	EXPORT(loop);
	EXPORT(zigzag);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Layer::Vocab
RadialGradient::get_param_vocab()const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("gradient")
		.set_local_name(_("Gradient"))
		.set_description(_("Gradient to apply"))
	);

	ret.push_back(ParamDesc("center")
		.set_local_name(_("Center"))
		.set_description(_("Center of the gradient"))
	);

	ret.push_back(ParamDesc("radius")
		.set_local_name(_("Radius"))
		.set_description(_("This is the radius of the circle"))
		.set_is_distance()
		.set_origin("center")
	);

	ret.push_back(ParamDesc("loop")
		.set_local_name(_("Loop"))
		.set_description(_("When checked the gradient is looped"))
	);

	ret.push_back(ParamDesc("zigzag")
		.set_local_name(_("ZigZag"))
		.set_description(_("When checked the gradient is summetrical at the center"))
	);

	return ret;
}

inline Color
RadialGradient::color_func(const Point &point, float supersample)const
{
	Real dist((point-center).mag()/radius);

	if(zigzag)
	{
		dist*=2.0;
		supersample*=2.0;
		if(dist>1)dist=2.0-dist;
	}

	if(loop)
	{
		dist-=floor(dist);

		if(dist+supersample*0.5>1.0)
		{
			float  left(supersample*0.5-(dist-1.0));
			float right(supersample*0.5+(dist-1.0));
			Color pool(gradient(1.0-(left*0.5),left).premult_alpha()*left/supersample);
			if (zigzag) pool+=gradient(1.0-right*0.5,right).premult_alpha()*right/supersample;
			else		pool+=gradient(right*0.5,right).premult_alpha()*right/supersample;
			return pool.demult_alpha();
		}
		if(dist-supersample*0.5<0.0)
		{
			float  left(supersample*0.5-dist);
			float right(supersample*0.5+dist);
			Color pool(gradient(right*0.5,right).premult_alpha()*right/supersample);
			if (zigzag) pool+=gradient(left*0.5,left).premult_alpha()*left/supersample;
			else		pool+=gradient(1.0-left*0.5,left).premult_alpha()*left/supersample;
			return pool.demult_alpha();
		}
	}

	return gradient(dist,supersample);
}


float
RadialGradient::calc_supersample(const synfig::Point &/*x*/, float pw,float /*ph*/)const
{
//	return sqrt(pw*pw+ph*ph)/radius;
	return 1.2*pw/radius;
}

synfig::Layer::Handle
RadialGradient::hit_check(synfig::Context context, const synfig::Point &point)const
{
	if(get_blend_method()==Color::BLEND_STRAIGHT && get_amount()>=0.5)
		return const_cast<RadialGradient*>(this);
	if(get_amount()==0.0)
		return context.hit_check(point);
	if((get_blend_method()==Color::BLEND_STRAIGHT || get_blend_method()==Color::BLEND_COMPOSITE) && color_func(point).get_a()>0.5)
		return const_cast<RadialGradient*>(this);
	return context.hit_check(point);
}

Color
RadialGradient::get_color(Context context, const Point &pos)const
{
	const Color color(color_func(pos));

	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
		return color;
	else
		return Color::blend(color,context.get_color(pos),get_amount(),get_blend_method());
}

bool
RadialGradient::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	SuperCallback supercb(cb,0,9500,10000);

	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
	{
		surface->set_wh(renddesc.get_w(),renddesc.get_h());
	}
	else
	{
		if(!context.accelerated_render(surface,quality,renddesc,&supercb))
			return false;
		if(get_amount()==0)
			return true;
	}


	int x,y;

	Surface::pen pen(surface->begin());
	const Real pw(renddesc.get_pw()),ph(renddesc.get_ph());
	Point pos;
	Point tl(renddesc.get_tl());
	const int w(surface->get_w());
	const int h(surface->get_h());

	if(get_amount()==1.0 && get_blend_method()==Color::BLEND_STRAIGHT)
	{
		for(y=0,pos[1]=tl[1];y<h;y++,pen.inc_y(),pen.dec_x(x),pos[1]+=ph)
			for(x=0,pos[0]=tl[0];x<w;x++,pen.inc_x(),pos[0]+=pw)
				pen.put_value(color_func(pos,calc_supersample(pos,pw,ph)));
	}
	else
	{
		for(y=0,pos[1]=tl[1];y<h;y++,pen.inc_y(),pen.dec_x(x),pos[1]+=ph)
			for(x=0,pos[0]=tl[0];x<w;x++,pen.inc_x(),pos[0]+=pw)
				pen.put_value(Color::blend(color_func(pos,calc_supersample(pos,pw,ph)),pen.get_value(),get_amount(),get_blend_method()));
	}

	// Mark our progress as finished
	if(cb && !cb->amount_complete(10000,10000))
		return false;

	return true;
}
