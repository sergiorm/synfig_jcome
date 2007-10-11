/* === S Y N F I G ========================================================= */
/*!	\file dock_children.cpp
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

#include "dock_children.h"
#include "app.h"

#include <gtkmm/scrolledwindow.h>
#include <cassert>
#include "instance.h"
#include <sigc++/signal.h>
#include <sigc++/hide.h>
#include <sigc++/retype_return.h>
#include <sigc++/slot.h>
#include "childrentreestore.h"
#include "childrentree.h"
#include "canvasview.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Dock_Children::Dock_Children():
	Dock_CanvasSpecific("children",_("Children"),Gtk::StockID("synfig-children"))
{
	set_use_scrolled(false);
/*
	add(*create_action_tree());

	add_button(
		Gtk::StockID("synfig-clear_redo"),
		_("Clear the REDO Stack")
	)->signal_clicked().connect(
		sigc::mem_fun(
			*this,
			&Dock_Children::clear_redo
		)
	);
*/
}

Dock_Children::~Dock_Children()
{
}

void
Dock_Children::init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	Glib::RefPtr<ChildrenTreeStore> children_tree_store;
	children_tree_store=ChildrenTreeStore::create(canvas_view->canvas_interface());

	ChildrenTree* children_tree(new ChildrenTree());
	children_tree->set_model(children_tree_store);
	children_tree->set_time_adjustment(canvas_view->time_adjustment());


	canvas_view->set_tree_model(get_name(),children_tree_store);
	canvas_view->set_ext_widget(get_name(),children_tree);
}

void
Dock_Children::changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	if(canvas_view)
	{
		Gtk::Widget* tree_view(canvas_view->get_ext_widget(get_name()));

		add(*tree_view);
		tree_view->show();
	}
	else clear_previous();

}