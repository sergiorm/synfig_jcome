/* === S Y N F I G ========================================================= */
/*!	\file synfig/main.cpp
**	\brief \writeme
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

//#define SYNFIG_NO_ANGLE

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <iostream>
#include "version.h"
#include "general.h"
#include "module.h"
#include <cstdlib>
#include <ltdl.h>
#include <stdexcept>
#include "target.h"
#include <ETL/stringf>
#include "listimporter.h"
#include "color.h"
#include "vector.h"
#include <fstream>
#include "layer.h"
#include "valuenode.h"

#include "main.h"
#include "loadcanvas.h"

#include "guid.h"

#include "mutex.h"

#ifdef DEATH_TIME
#include <time.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#endif

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

#define MODULE_LIST_FILENAME	"synfig_modules.cfg"

/* === S T A T I C S ======================================================= */

static etl::reference_counter synfig_ref_count_(0);

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */








const char *
synfig::get_version()
{
#ifdef VERSION
	return VERSION;
#else
	return "Unknown";
#endif
}

const char *
synfig::get_build_date()
{
	return __DATE__;
}

const char *
synfig::get_build_time()
{
	return __TIME__;
}

extern const char *get_build_time();

bool
synfig::check_version_(int version,int vec_size, int color_size,int canvas_size,int layer_size)
{
	bool ret=true;

	CHECK_EXPIRE_TIME();

	if(version!=SYNFIG_LIBRARY_VERSION)
	{
		synfig::error(_("API Version mismatch (LIB:%d, PROG:%d)"),SYNFIG_LIBRARY_VERSION,version);
		ret=false;
	}
	if(vec_size!=sizeof(Vector))
	{
		synfig::error(_("Size of Vector mismatch (app:%d, lib:%d)"),vec_size,sizeof(Vector));
		ret=false;
	}
	if(color_size!=sizeof(Color))
	{
		synfig::error(_("Size of Color mismatch (app:%d, lib:%d)"),color_size,sizeof(Color));
		ret=false;
	}
	if(canvas_size!=sizeof(Canvas))
	{
		synfig::error(_("Size of Canvas mismatch (app:%d, lib:%d)"),canvas_size,sizeof(Canvas));
		ret=false;
	}
	if(layer_size!=sizeof(Layer))
	{
		synfig::error(_("Size of Layer mismatch (app:%d, lib:%d)"),layer_size,sizeof(Layer));
		ret=false;
	}

	return ret;
}

static void broken_pipe_signal (int /*sig*/)  {
	synfig::warning("Broken Pipe...");
}

bool retrieve_modules_to_load(String filename,std::list<String> &modules_to_load)
{
	if(filename=="standard")
	{
		return false;
/*
		if(find(modules_to_load.begin(),modules_to_load.end(),"trgt_bmp")==modules_to_load.end())
			modules_to_load.push_back("trgt_bmp");
		if(find(modules_to_load.begin(),modules_to_load.end(),"trgt_gif")==modules_to_load.end())
			modules_to_load.push_back("trgt_gif");
		if(find(modules_to_load.begin(),modules_to_load.end(),"trgt_dv")==modules_to_load.end())
			modules_to_load.push_back("trgt_dv");
		if(find(modules_to_load.begin(),modules_to_load.end(),"mod_ffmpeg")==modules_to_load.end())
			modules_to_load.push_back("mod_ffmpeg");
		if(find(modules_to_load.begin(),modules_to_load.end(),"mod_imagemagick")==modules_to_load.end())
			modules_to_load.push_back("mod_imagemagick");
		if(find(modules_to_load.begin(),modules_to_load.end(),"lyr_std")==modules_to_load.end())
			modules_to_load.push_back("lyr_std");
		if(find(modules_to_load.begin(),modules_to_load.end(),"lyr_freetype")==modules_to_load.end())
			modules_to_load.push_back("lyr_freetype");
#ifdef HAVE_LIBPNG
		if(find(modules_to_load.begin(),modules_to_load.end(),"trgt_png")==modules_to_load.end())
			modules_to_load.push_back("trgt_png");
#endif
#ifdef HAVE_OPENEXR
		if(find(modules_to_load.begin(),modules_to_load.end(),"mod_openexr")==modules_to_load.end())
			modules_to_load.push_back("mod_openexr");
#endif
*/
	}
	else
	{
		std::ifstream file(filename.c_str());
		if(!file)
		{
		//	warning("Cannot open "+filename);
			return false;
		}
		while(file)
		{
			String modulename;
			getline(file,modulename);
			if(!modulename.empty() && find(modules_to_load.begin(),modules_to_load.end(),modulename)==modules_to_load.end())
				modules_to_load.push_back(modulename);
		}
	}



	return true;
}





synfig::Main::Main(const synfig::String& basepath,ProgressCallback *cb):
	ref_count_(synfig_ref_count_)
{
	if(ref_count_.count())
		return;

	synfig_ref_count_.reset();
	ref_count_=synfig_ref_count_;

	// Add initialization after this point


	CHECK_EXPIRE_TIME();

	String prefix=basepath+"/..";
	unsigned int i;
#ifdef _DEBUG
	std::set_terminate(__gnu_cxx::__verbose_terminate_handler);
#endif

#if defined(HAVE_SIGNAL_H) && defined(SIGPIPE)
	signal(SIGPIPE, broken_pipe_signal);
#endif

	//_config_search_path=new vector"string.h"();

	// Init the subsystems
	if(cb)cb->amount_complete(0, 100);
	if(cb)cb->task(_("Starting Subsystem \"Modules\""));
	if(!Module::subsys_init(prefix))
		throw std::runtime_error(_("Unable to initialize subsystem \"Module\""));

	if(cb)cb->task(_("Starting Subsystem \"Layers\""));
	if(!Layer::subsys_init())
	{
		Module::subsys_stop();
		throw std::runtime_error(_("Unable to initialize subsystem \"Layers\""));
	}

	if(cb)cb->task(_("Starting Subsystem \"Targets\""));
	if(!Target::subsys_init())
	{
		Layer::subsys_stop();
		Module::subsys_stop();
		throw std::runtime_error(_("Unable to initialize subsystem \"Targets\""));
	}

	if(cb)cb->task(_("Starting Subsystem \"Importers\""));
	if(!Importer::subsys_init())
	{
		Target::subsys_stop();
		Layer::subsys_stop();
		Module::subsys_stop();
		throw std::runtime_error(_("Unable to initialize subsystem \"Importers\""));
	}

	if(cb)cb->task(_("Starting Subsystem \"ValueNodes\""));
	if(!ValueNode::subsys_init())
	{
		Importer::subsys_stop();
		Target::subsys_stop();
		Layer::subsys_stop();
		Module::subsys_stop();
		throw std::runtime_error(_("Unable to initialize subsystem \"ValueNodes\""));
	}

	// Load up the list importer
	Importer::book()[String("lst")]=ListImporter::create;



	// Load up the modules
	std::list<String> modules_to_load;
	std::vector<String> locations;

	if(!getenv("SYNFIG_MODULE_LIST"))
	{
		locations.push_back("standard");
		locations.push_back("./"MODULE_LIST_FILENAME);	//1
		locations.push_back("../etc/"MODULE_LIST_FILENAME);	//1
		locations.push_back("~/.synfig/"MODULE_LIST_FILENAME); //2
		locations.push_back(prefix+"/etc/"+MODULE_LIST_FILENAME); //3
		locations.push_back("/usr/local/etc/"MODULE_LIST_FILENAME);
	#ifdef SYSCONFDIR
		locations.push_back(SYSCONFDIR"/"MODULE_LIST_FILENAME);
	#endif
	#ifdef __APPLE__
		locations.push_back("/Library/Frameworks/synfig.framework/Resources/"MODULE_LIST_FILENAME);
		locations.push_back("/Library/Synfig/"MODULE_LIST_FILENAME);
		locations.push_back("~/Library/Synfig/"MODULE_LIST_FILENAME);
	#endif
	#ifdef WIN32
		locations.push_back("C:\\Program Files\\Synfig\\etc\\"MODULE_LIST_FILENAME);
	#endif
	}
	else
	{
		locations.push_back(getenv("SYNFIG_MODULE_LIST"));
	}
/*
	const char *locations[]=
	{
		"standard",	//0
		"./"MODULE_LIST_FILENAME,	//1
		"../etc/"MODULE_LIST_FILENAME,	//1
		"~/.synfig/"MODULE_LIST_FILENAME, //2
		"/usr/local/lib/synfig/modules/"MODULE_LIST_FILENAME, //3
		"/usr/local/etc/"MODULE_LIST_FILENAME,
#ifdef SYSCONFDIR
		SYSCONFDIR"/"MODULE_LIST_FILENAME,
#endif
#ifdef __APPLE__
		"/Library/Frameworks/synfig.framework/Resources/"MODULE_LIST_FILENAME,
		"/Library/SYNFIG/"MODULE_LIST_FILENAME,
		"~/Library/SYNFIG/"MODULE_LIST_FILENAME,
#endif
#ifdef WIN32
		"C:\\Program Files\\SYNFIG\\etc\\"MODULE_LIST_FILENAME,
#endif
	};
*/

	for(i=0;i<locations.size();i++)
		if(retrieve_modules_to_load(locations[i],modules_to_load))
			if(cb)cb->task(strprintf(_("Loading modules from %s"),locations[i].c_str()));

	std::list<String>::iterator iter;

	for(i=0,iter=modules_to_load.begin();iter!=modules_to_load.end();++iter,i++)
	{
		Module::Register(*iter,cb);
		if(cb)cb->amount_complete((i+1)*100,modules_to_load.size()*100);
	}

//	load_modules(cb);

	CHECK_EXPIRE_TIME();


	if(cb)cb->amount_complete(100, 100);
	if(cb)cb->task(_("DONE"));
}

synfig::Main::~Main()
{
	ref_count_.detach();
	if(!synfig_ref_count_.unique())
		return;
	synfig_ref_count_.detach();

	// Add deinitialization after this point

	if(get_open_canvas_map().size())
	{
		synfig::warning("Canvases still open!");
		std::map<synfig::String, etl::loose_handle<Canvas> >::iterator iter;
		for(iter=get_open_canvas_map().begin();iter!=get_open_canvas_map().end();++iter)
		{
			synfig::warning("%s: count()=%d",iter->first.c_str(), iter->second.count());
		}
	}

	synfig::info("ValueNode::subsys_stop()");
	ValueNode::subsys_stop();
	synfig::info("Importer::subsys_stop()");
	Importer::subsys_stop();
	synfig::info("Target::subsys_stop()");
	Target::subsys_stop();
	synfig::info("Layer::subsys_stop()");
	Layer::subsys_stop();
	/*! \fixme For some reason, uncommenting the next line will cause things to crash.
			   This needs to be looked into at some point. */
 	// synfig::info("Module::subsys_stop()");
	// Module::subsys_stop();
	synfig::info("Exiting");
	
#if defined(HAVE_SIGNAL_H) && defined(SIGPIPE)
	signal(SIGPIPE, SIG_DFL);
#endif
}

void
synfig::error(const char *format,...)
{
	va_list args;
	va_start(args,format);
	error(vstrprintf(format,args));
}

void
synfig::error(const String &str)
{
	static Mutex mutex; Mutex::Lock lock(mutex);
	cerr<<"synfig("<<getpid()<<"): "<<_("error")<<": "+str<<endl;
}

void
synfig::warning(const char *format,...)
{
	va_list args;
	va_start(args,format);
	warning(vstrprintf(format,args));
}

void
synfig::warning(const String &str)
{
	static Mutex mutex; Mutex::Lock lock(mutex);
	cerr<<"synfig("<<getpid()<<"): "<<_("warning")<<": "+str<<endl;
}

void
synfig::info(const char *format,...)
{
	va_list args;
	va_start(args,format);
	info(vstrprintf(format,args));
}

void
synfig::info(const String &str)
{
	static Mutex mutex; Mutex::Lock lock(mutex);
	cerr<<"synfig("<<getpid()<<"): "<<_("info")<<": "+str<<endl;
}