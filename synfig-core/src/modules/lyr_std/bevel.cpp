/* === S Y N F I G ========================================================= */
/*!	\file bevel.cpp
**	\brief Implementation of the "Bevel" layer
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

#include "bevel.h"

#include <synfig/string.h>
#include <synfig/time.h>
#include <synfig/context.h>
#include <synfig/paramdesc.h>
#include <synfig/renddesc.h>
#include <synfig/surface.h>
#include <synfig/value.h>
#include <synfig/valuenode.h>
#include <synfig/segment.h>

#include <cstring>
#include <ETL/pen>
#include <ETL/misc>

#endif

using namespace synfig;
using namespace etl;
using namespace std;

/*#define TYPE_BOX			0
#define TYPE_FASTGUASSIAN	1
#define TYPE_FASTGAUSSIAN	1
#define TYPE_CROSS			2
#define TYPE_GUASSIAN		3
#define TYPE_GAUSSIAN		3
#define TYPE_DISC			4
*/

/* -- G L O B A L S --------------------------------------------------------- */

SYNFIG_LAYER_INIT(Layer_Bevel);
SYNFIG_LAYER_SET_NAME(Layer_Bevel,"bevel");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_Bevel,N_("Bevel"));
SYNFIG_LAYER_SET_CATEGORY(Layer_Bevel,N_("Stylize"));
SYNFIG_LAYER_SET_VERSION(Layer_Bevel,"0.2");
SYNFIG_LAYER_SET_CVS_ID(Layer_Bevel,"$Id$");

/* -- F U N C T I O N S ----------------------------------------------------- */

inline void clamp(synfig::Vector &v)
{
	if(v[0]<0.0)v[0]=0.0;
	if(v[1]<0.0)v[1]=0.0;
}

Layer_Bevel::Layer_Bevel():
	Layer_Composite	(0.75,Color::BLEND_ONTO),
	softness(0.1),
	type(Blur::FASTGAUSSIAN),
	color1(Color::white()),
	color2(Color::black()),
	depth(0.2)
{
	angle=Angle::deg(135);
	calc_offset();
	use_luma=false;
	solid=false;
	Layer::Vocab voc(get_param_vocab());
	Layer::fill_static(voc);
}

void
Layer_Bevel::calc_offset()
{
	offset[0]=Angle::cos(angle).get()*depth;
	offset[1]=Angle::sin(angle).get()*depth;

	offset45[0]=Angle::cos(angle-Angle::deg(45)).get()*depth*0.707106781;
	offset45[1]=Angle::sin(angle-Angle::deg(45)).get()*depth*0.707106781;
}

bool
Layer_Bevel::set_param(const String &param, const ValueBase &value)
{
	IMPORT_PLUS(softness,softness=softness>0?softness:0);
	IMPORT(color1);
	IMPORT(color2);
	IMPORT_PLUS(depth,calc_offset());
	IMPORT_PLUS(angle,calc_offset());
	IMPORT(type);
	IMPORT(use_luma);
	IMPORT(solid);

	return Layer_Composite::set_param(param,value);
}

ValueBase
Layer_Bevel::get_param(const String &param)const
{
	EXPORT(type);
	EXPORT(softness);
	EXPORT(color1);
	EXPORT(color2);
	EXPORT(depth);
	EXPORT(angle);
	EXPORT(use_luma);
	EXPORT(solid);

	EXPORT_NAME();
	EXPORT_VERSION();

	return Layer_Composite::get_param(param);
}

Color
Layer_Bevel::get_color(Context context, const Point &pos)const
{
	const Vector size(softness,softness);
	Point blurpos = Blur(size,type)(pos);

	if(get_amount()==0.0)
		return context.get_color(pos);

	Color shade;

	Real hi_alpha(1.0f-context.get_color(blurpos+offset).get_a());
	Real lo_alpha(1.0f-context.get_color(blurpos-offset).get_a());

	Real shade_alpha(hi_alpha-lo_alpha);
	if(shade_alpha>0)
		shade=color1,shade.set_a(shade_alpha);
	else
		shade=color2,shade.set_a(-shade_alpha);

	return Color::blend(shade,context.get_color(pos),get_amount(),get_blend_method());
}

bool
Layer_Bevel::accelerated_render(Context context,Surface *surface,int quality, const RendDesc &renddesc, ProgressCallback *cb)const
{
	int x,y;
	SuperCallback stageone(cb,0,5000,10000);
	SuperCallback stagetwo(cb,5000,10000,10000);

	const int	w = renddesc.get_w(),
				h = renddesc.get_h();
	const Real	pw = renddesc.get_pw(),
				ph = renddesc.get_ph();
	const Vector size(softness,softness);

	RendDesc	workdesc(renddesc);
	Surface		worksurface;
	etl::surface<float> blurred;

	//callbacks depend on how long the blur takes
	if(size[0] || size[1])
	{
		if(type == Blur::DISC)
		{
			stageone = SuperCallback(cb,0,5000,10000);
			stagetwo = SuperCallback(cb,5000,10000,10000);
		}
		else
		{
			stageone = SuperCallback(cb,0,9000,10000);
			stagetwo = SuperCallback(cb,9000,10000,10000);
		}
	}
	else
	{
		stageone = SuperCallback(cb,0,9999,10000);
		stagetwo = SuperCallback(cb,9999,10000,10000);
	}

	//expand the working surface to accommodate the blur

	//the expanded size = 1/2 the size in each direction rounded up
	int	halfsizex = (int) (abs(size[0]*.5/pw) + 3),
		halfsizey = (int) (abs(size[1]*.5/ph) + 3);

	int offset_u(round_to_int(offset[0]/pw)),offset_v(round_to_int(offset[1]/ph));
	int offset_w(w+abs(offset_u)*2),offset_h(h+abs(offset_v)*2);

	workdesc.set_subwindow(
		-abs(offset_u),
		-abs(offset_v),
		w+abs(offset_u),
		h+abs(offset_v)
	);

	//expand by 1/2 size in each direction on either side
	switch(type)
	{
		case Blur::DISC:
		case Blur::BOX:
		case Blur::CROSS:
		{
			workdesc.set_subwindow(-max(1,halfsizex),-max(1,halfsizey),offset_w+2*max(1,halfsizex),offset_h+2*max(1,halfsizey));
			break;
		}
		case Blur::FASTGAUSSIAN:
		{
			if(quality < 4)
			{
				halfsizex*=2;
				halfsizey*=2;
			}
			workdesc.set_subwindow(-max(1,halfsizex),-max(1,halfsizey),offset_w+2*max(1,halfsizex),offset_h+2*max(1,halfsizey));
			break;
		}
		case Blur::GAUSSIAN:
		{
		#define GAUSSIAN_ADJUSTMENT		(0.05)
			Real	pw = (Real)workdesc.get_w()/(workdesc.get_br()[0]-workdesc.get_tl()[0]);
			Real 	ph = (Real)workdesc.get_h()/(workdesc.get_br()[1]-workdesc.get_tl()[1]);

			pw=pw*pw;
			ph=ph*ph;

			halfsizex = (int)(abs(pw)*size[0]*GAUSSIAN_ADJUSTMENT+0.5);
			halfsizey = (int)(abs(ph)*size[1]*GAUSSIAN_ADJUSTMENT+0.5);

			halfsizex = (halfsizex + 1)/2;
			halfsizey = (halfsizey + 1)/2;
			workdesc.set_subwindow( -halfsizex, -halfsizey, offset_w+2*halfsizex, offset_h+2*halfsizey );

			break;
		}
	}

	//render the background onto the expanded surface
	if(!context.accelerated_render(&worksurface,quality,workdesc,&stageone))
		return false;

	// Copy over the alpha
	blurred.set_wh(worksurface.get_w(),worksurface.get_h());
	if(!use_luma)
	{
		for(int j=0;j<worksurface.get_h();j++)
			for(int i=0;i<worksurface.get_w();i++)
			{
				blurred[j][i]=worksurface[j][i].get_a();
			}
	}
	else
	{
		for(int j=0;j<worksurface.get_h();j++)
			for(int i=0;i<worksurface.get_w();i++)
			{
				blurred[j][i]=worksurface[j][i].get_a()*worksurface[j][i].get_y();
			}
	}

	//blur the image
	Blur(size,type,&stagetwo)(blurred,workdesc.get_br()-workdesc.get_tl(),blurred);

	//be sure the surface is of the correct size
	surface->set_wh(renddesc.get_w(),renddesc.get_h());

	int u = halfsizex+abs(offset_u), v = halfsizey+abs(offset_v);
	for(y=0;y<renddesc.get_h();y++,v++)
	{
		u = halfsizex+abs(offset_u);
		for(x=0;x<renddesc.get_w();x++,u++)
		{
			Real alpha(0);
			Color shade;

			{
				const float u2(offset[0]/pw),v2(offset[1]/ph);
				alpha+=1.0f-blurred.linear_sample(u2+u,v2+v);
			}
			{
				const float u2(-offset[0]/pw),v2(-offset[1]/ph);
				alpha-=1.0f-blurred.linear_sample(u2+u,v2+v);
			}
			{
				const float u2(offset45[0]/pw),v2(offset45[1]/ph);
				alpha+=1.0f-blurred.linear_sample(u2+u,v2+v)*0.5f;
			}
			{
				const float u2(offset45[1]/ph),v2(-offset45[0]/pw);
				alpha+=1.0f-blurred.linear_sample(u2+u,v2+v)*0.5f;
			}
			{
				const float u2(-offset45[0]/pw),v2(-offset45[1]/ph);
				alpha-=1.0f-blurred.linear_sample(u2+u,v2+v)*0.5f;
			}
			{
				const float u2(-offset45[1]/ph),v2(offset45[0]/pw);
				alpha-=1.0f-blurred.linear_sample(u2+u,v2+v)*0.5f;
			}

			if(solid)
			{
				alpha/=4.0f;
				alpha+=0.5f;
				shade=Color::blend(color1,color2,alpha,Color::BLEND_STRAIGHT);
			}
			else
			{
				alpha/=2;
				if(alpha>0)
					shade=color1,shade.set_a(shade.get_a()*alpha);
				else
					shade=color2,shade.set_a(shade.get_a()*-alpha);
			}



			if(shade.get_a())
			{
				(*surface)[y][x]=Color::blend(shade,worksurface[v][u],get_amount(),get_blend_method());
			}
			else (*surface)[y][x] = worksurface[v][u];
		}
	}

	if(cb && !cb->amount_complete(10000,10000))
	{
		//if(cb)cb->error(strprintf(__FILE__"%d: Accelerated Renderer Failure",__LINE__));
		return false;
	}

	return true;
}

Layer::Vocab
Layer_Bevel::get_param_vocab(void)const
{
	Layer::Vocab ret(Layer_Composite::get_param_vocab());

	ret.push_back(ParamDesc("type")
		.set_local_name(_("Type"))
		.set_description(_("Type of blur to use"))
		.set_hint("enum")
		.add_enum_value(Blur::BOX,"box",_("Box Blur"))
		.add_enum_value(Blur::FASTGAUSSIAN,"fastgaussian",_("Fast Gaussian Blur"))
		.add_enum_value(Blur::CROSS,"cross",_("Cross-Hatch Blur"))
		.add_enum_value(Blur::GAUSSIAN,"gaussian",_("Gaussian Blur"))
		.add_enum_value(Blur::DISC,"disc",_("Disc Blur"))
	);

	ret.push_back(ParamDesc("color1")
		.set_local_name(_("Hi-Color"))
	);
	ret.push_back(ParamDesc("color2")
		.set_local_name(_("Lo-Color"))
	);
	ret.push_back(ParamDesc("angle")
		.set_local_name(_("Light Angle"))
	);
	ret.push_back(ParamDesc("depth")
		.set_is_distance()
		.set_local_name(_("Depth of Bevel"))
	);
	ret.push_back(ParamDesc("softness")
		.set_is_distance()
		.set_local_name(_("Softness"))
	);
	ret.push_back(ParamDesc("use_luma")
		.set_local_name(_("Use Luma"))
	);
	ret.push_back(ParamDesc("solid")
		.set_local_name(_("Solid"))
	);

	return ret;
}

Rect
Layer_Bevel::get_full_bounding_rect(Context context)const
{
	if(is_disabled())
		return context.get_full_bounding_rect();

	Rect under(context.get_full_bounding_rect());

	if(Color::is_onto(get_blend_method()))
		return under;

	Rect bounds(under.expand(softness));
	bounds.expand_x(abs(depth));
	bounds.expand_y(abs(depth));

	return bounds;
}
