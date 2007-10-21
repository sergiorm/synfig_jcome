/* === S Y N F I G ========================================================= */
/*!	\file valuenode_switch.cpp
**	\brief Template File
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

#include "valuenode_switch.h"
#include "valuenode_const.h"
#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Switch::ValueNode_Switch(const ValueBase::Type &x):
	LinkableValueNode(x)
{
}

ValueNode_Switch::ValueNode_Switch(const ValueNode::Handle &x):
	LinkableValueNode(x->get_type())
{
	set_link("link_off",x);
	set_link("link_on",x);
	set_link("switch",ValueNode_Const::create(bool(false)));
}

ValueNode_Switch*
ValueNode_Switch::create(const ValueBase &x)
{
	return new ValueNode_Switch(ValueNode_Const::create(x));
}

LinkableValueNode*
ValueNode_Switch::create_new()const
{
	return new ValueNode_Switch(get_type());
}

ValueNode_Switch::~ValueNode_Switch()
{
	unlink_all();
}

bool
ValueNode_Switch::set_link_vfunc(int i,ValueNode::Handle x)
{
	assert(i>=0 && i<link_count());
	switch(i)
	{
	case 0:
		if(x->get_type()!=get_type() && !PlaceholderValueNode::Handle::cast_dynamic(x))
			return false;
		link_off_=x;
		signal_child_changed()(i);signal_value_changed()();
		return true;
	case 1:
		if(x->get_type()!=get_type() && !PlaceholderValueNode::Handle::cast_dynamic(x))
			return false;
		link_on_=x;
		signal_child_changed()(i);signal_value_changed()();
		return true;
	case 2:
		switch_=x;
		signal_child_changed()(i);signal_value_changed()();
		return true;
	}
	return true;
}

ValueNode::LooseHandle
ValueNode_Switch::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());
	switch(i)
	{
	case 0: return link_off_;
	case 1: return link_on_;
	case 2: return switch_;
	}
	return 0;
}

int
ValueNode_Switch::link_count()const
{
	return 3;
}

String
ValueNode_Switch::link_name(int i)const
{
	assert(i>=0 && i<link_count());
	switch(i)
	{
	case 0: return "link_off";
	case 1: return "link_on";
	case 2: return "switch";
	}
	return String();
}

String
ValueNode_Switch::link_local_name(int i)const
{
	assert(i>=0 && i<link_count());
	switch(i)
	{
	case 0: return "Link Off";
	case 1: return "Link On";
	case 2: return "Switch";
	}
	return String();
}

int
ValueNode_Switch::get_link_index_from_name(const String &name)const
{
	if(name=="link_off") return 0;
	if(name=="link_on" ) return 1;
	if(name=="switch"  ) return 2;
	throw Exception::BadLinkName(name);
}

ValueBase
ValueNode_Switch::operator()(Time t)const
{
	return (*switch_)(t).get(bool()) ? (*link_on_)(t) : (*link_off_)(t);
}


String
ValueNode_Switch::get_name()const
{
	return "switch";
}

String
ValueNode_Switch::get_local_name()const
{
	return _("Switch");
}

bool
ValueNode_Switch::check_type(ValueBase::Type type)
{
	if(type)
		return true;
	return false;
}