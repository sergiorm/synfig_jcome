/* === S Y N F I G ========================================================= */
/*!	\file state_zoom.cpp
**	\brief Zoom Toole Implementation File
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

#include <sigc++/signal.h>
#include <sigc++/object.h>

#include <ETL/handle>
#include <synfig/vector.h>


#include "state_zoom.h"
#include "event_mouse.h"
#include "canvasview.h"
#include "workarea.h"
#include "app.h"
#include "dialog_tooloptions.h"
#include "toolbox.h"
#include <synfigapp/main.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */
StateZoom studio::state_zoom;

const float ZOOMFACTOR = 1.25f;

/* === C L A S S E S & S T R U C T S ======================================= */

class studio::StateZoom_Context : public sigc::trackable
{
	etl::handle<CanvasView> canvas_view_;
	CanvasView::IsWorking is_working;

	Point p1,p2;

	bool prev_workarea_layer_status_;

	//Toolbox settings
	synfigapp::Settings& settings;

	//Toolbox display
	Gtk::Table options_table;

public:

	void refresh_tool_options(); //to refresh the toolbox

	//events
	Smach::event_result event_stop_handler(const Smach::event& x);
	Smach::event_result event_refresh_handler(const Smach::event& x);
	Smach::event_result event_mouse_click_handler(const Smach::event& x);
	Smach::event_result event_refresh_tool_options(const Smach::event& x);

	//constructor destructor
	StateZoom_Context(CanvasView* canvas_view);
	~StateZoom_Context();

	//Canvas interaction
	const etl::handle<CanvasView>& get_canvas_view()const{return canvas_view_;}
	etl::handle<synfigapp::CanvasInterface> get_canvas_interface()const{return canvas_view_->canvas_interface();}
	synfig::Canvas::Handle get_canvas()const{return canvas_view_->get_canvas();}
	WorkArea * get_work_area()const{return canvas_view_->get_work_area();}

	//Modifying settings etc.
	void load_settings();
	void save_settings();
	void reset();

	//void zoom(const Point& p1, const Point& p2);

};	// END of class StateGradient_Context

/* === M E T H O D S ======================================================= */

StateZoom::StateZoom():
	Smach::state<StateZoom_Context>("zoom")
{
	insert(event_def(EVENT_STOP,&StateZoom_Context::event_stop_handler));
	insert(event_def(EVENT_REFRESH,&StateZoom_Context::event_refresh_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DOWN,&StateZoom_Context::event_mouse_click_handler));
	//insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_DRAG,&StateZoom_Context::event_mouse_click_handler));
	insert(event_def(EVENT_WORKAREA_BOX,&StateZoom_Context::event_mouse_click_handler));
	//insert(event_def(EVENT_WORKAREA_BUTTON_CLICK,&StateZoom_Context::event_mouse_click_handler));
	insert(event_def(EVENT_WORKAREA_MOUSE_BUTTON_UP,&StateZoom_Context::event_mouse_click_handler));
	insert(event_def(EVENT_REFRESH_TOOL_OPTIONS,&StateZoom_Context::event_refresh_tool_options));
}

StateZoom::~StateZoom()
{
}

void
StateZoom_Context::load_settings()
{
	String value;

	//parse the arguments yargh!
	/*if(settings.get_value("circle.feather",value))
		set_feather(atof(value.c_str()));
	else
		set_feather(0);*/
}

void
StateZoom_Context::save_settings()
{
	//settings.set_value("circle.fallofftype",strprintf("%d",get_falloff()));
}

void
StateZoom_Context::reset()
{
	//refresh_ducks();
}

StateZoom_Context::StateZoom_Context(CanvasView* canvas_view):
	canvas_view_(canvas_view),
	is_working(*canvas_view),
	prev_workarea_layer_status_(get_work_area()->allow_layer_clicks),
	settings(synfigapp::Main::get_selected_input_device()->settings())
{
	// Set up the tool options dialog
	//options_table.attach(*manage(new Gtk::Label(_("Zoom Tool"))), 0, 2, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL, 0, 0);

	load_settings();

	options_table.show_all();

	refresh_tool_options();
	App::dialog_tool_options->present();

	// Turn off layer clicking
	get_work_area()->allow_layer_clicks=false;

	// clear out the ducks
	get_work_area()->clear_ducks(); //???

	// Refresh the work area
	get_work_area()->queue_draw();

	// Hide the tables if they are showing
	//prev_table_status=get_canvas_view()->tables_are_visible();
	//if(prev_table_status)get_canvas_view()->hide_tables();

	// Disable the time bar
	//get_canvas_view()->set_sensitive_timebar(false);

	// Connect a signal
	//get_work_area()->signal_user_click().connect(sigc::mem_fun(*this,&studio::StateZoom_Context::on_user_click));
	get_canvas_view()->work_area->set_cursor(Gdk::CROSSHAIR);

	App::toolbox->refresh();
}

void
StateZoom_Context::refresh_tool_options()
{
	App::dialog_tool_options->clear();
	App::dialog_tool_options->set_widget(options_table);
	App::dialog_tool_options->set_local_name(_("Zoom Tool"));
	App::dialog_tool_options->set_name("zoom");
}

Smach::event_result
StateZoom_Context::event_refresh_tool_options(const Smach::event& x)
{
	refresh_tool_options();
	return Smach::RESULT_ACCEPT;
}

StateZoom_Context::~StateZoom_Context()
{
	save_settings();

	// Restore layer clicking
	get_work_area()->allow_layer_clicks=prev_workarea_layer_status_;
	get_canvas_view()->work_area->reset_cursor();

	App::dialog_tool_options->clear();

	// Enable the time bar
	//get_canvas_view()->set_sensitive_timebar(true);

	// Bring back the tables if they were out before
	//if(prev_table_status)get_canvas_view()->show_tables();

	// Refresh the work area
	get_work_area()->queue_draw();

	App::toolbox->refresh();

	get_canvas_view()->get_smach().process_event(EVENT_REFRESH_DUCKS);
}

Smach::event_result
StateZoom_Context::event_stop_handler(const Smach::event& x)
{
	throw Smach::egress_exception();
}

Smach::event_result
StateZoom_Context::event_refresh_handler(const Smach::event& x)
{
	return Smach::RESULT_ACCEPT;
}

Smach::event_result
StateZoom_Context::event_mouse_click_handler(const Smach::event& x)
{
	if(x.key==EVENT_WORKAREA_BOX)
	{
		const EventBox& event(*reinterpret_cast<const EventBox*>(&x));

		if(event.button==BUTTON_LEFT)
		{
			//respond to event box...


			//Center the new position at the center of the box

			//OH MY GOD HACK - the space is -1* and offset (by the value of the center of the canvas)...
			Point newpos;
			{
				const Point evcenter = (event.p1+event.p2)/2;
				const Point realcenter = (get_work_area()->get_window_tl() + get_work_area()->get_window_br())/2;
				newpos = -(evcenter - realcenter) + get_work_area()->get_focus_point();
			}

			//The zoom will be whatever the required factor to convert current box size to desired box size
			Point tl = get_work_area()->get_window_tl();
			Point br = get_work_area()->get_window_br();

			Vector	span = br - tl;
			Vector	v = event.p2 - event.p1;

			//get the minimum zoom as long as it's greater than 1...
			v[0] = abs(v[0])/abs(span[0]);
			v[1] = abs(v[1])/abs(span[1]);

			float zdiv = max(v[0],v[1]);
			if(zdiv < 1 && zdiv > 0) //must be zoomable
			{
				get_work_area()->set_focus_point(newpos);
				get_work_area()->set_zoom(get_work_area()->get_zoom()/zdiv);
			}

			return Smach::RESULT_ACCEPT;
		}
	}

	if(x.key==EVENT_WORKAREA_MOUSE_BUTTON_UP)
	{
		const EventMouse& event(*reinterpret_cast<const EventMouse*>(&x));

		if(event.button==BUTTON_LEFT)
		{
			Point evpos;

			//make the event pos be in the same space...
			//   The weird ass inverted center normalized space...
			{
				const Point realcenter = (get_work_area()->get_window_tl() + get_work_area()->get_window_br())/2;
				evpos = -(event.pos - realcenter) + get_work_area()->get_focus_point();
			}

			/*	Zooming:
				focus point must zoom about the point evpos...

				trans about an origin not 0:
				p' = A(p - o) + o
			*/

			Vector v = get_work_area()->get_focus_point() - evpos;

			if(event.modifier & Gdk::CONTROL_MASK) //zoom out...
			{
				v*=ZOOMFACTOR;
				//get_work_area()->zoom_out();
				get_work_area()->set_focus_point(evpos + v);
				get_work_area()->set_zoom(get_work_area()->get_zoom()/ZOOMFACTOR);
			}else //zoom in
			{
				v/=ZOOMFACTOR;
				//get_work_area()->zoom_in();
				get_work_area()->set_focus_point(evpos + v);
				get_work_area()->set_zoom(get_work_area()->get_zoom()*ZOOMFACTOR);
			}

			return Smach::RESULT_ACCEPT;
		}
	}

	return Smach::RESULT_OK;
}