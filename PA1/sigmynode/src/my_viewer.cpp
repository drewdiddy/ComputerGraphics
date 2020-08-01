
# include "my_viewer.h"
# include "SnMyTorus.h"

# include <sigogl/gl_tools.h>
# include <sigogl/ui_button.h>

MyViewer::MyViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{
	SnMyTorus* c = new SnMyTorus;
	add_ui ();
	add_mynode ();
}

void MyViewer::add_ui ()
{
	UiPanel *p;
	UiManager* uim = WsWindow::uim();
	p = uim->add_panel ( "", UiPanel::HorizLeft );
	p->add ( new UiButton ( "Add", EvAdd ) );
	p->add ( new UiButton ( "Info", EvInfo ) );
	p->add ( new UiButton ( "Exit", EvExit ) );
}

void MyViewer::add_mynode ()
{
	//SnMyTorus* c = new SnMyTorus;

	float r=0.15f;
	//c->init.set ( gs_random(-r,r), gs_random(-r,r), gs_random(-r,r) );
	//c->color ( GsColor::random() );
	// Example how to print/debug your generated data:
	// gsout<<n<<": "<<c->color()<<gsnl;
	rootg()->add(c);
	//c->touch();
	//render();

	
}

int MyViewer::handle_keyboard ( const GsEvent &e )
{
	int ret = WsViewer::handle_keyboard ( e ); // 1st let system check events
	if ( ret ) return ret;

	switch ( e.key )
	{	case GsEvent::KeyEsc : gs_exit(); return 1;
		case GsEvent::KeyLeft: gsout<<"Left\n"; return 1;
		//q was pressed
		case 113: 
			c->resolution = c->resolution + 0.1f;
			c->touch();
			render();
			return 1;
		//a was pressed
		case 97:
			c->resolution = c->resolution - 0.1f;
			c->touch();
			render();
			return 1;
		//w was pressed
		case 119:
			c->r = c->r + 0.1f;
			c->touch();
			render();
			return 1;
		//s was pressed
		case 115:
			c->r = c->r - 0.1f;
			c->touch();
			render();
			return 1;
		//e was pressed
		case 101:
			c->R = c->R + 0.1f;
			c->touch();
			render();
			return 1;
		//d was pressed
		case 100:
			c->R = c->R - 0.1f;
			c->touch();
			render();
			return 1;
		// etc
		default: gsout<<"Key pressed: "<<e.key<<gsnl;
	}

	return 0;
}

int MyViewer::uievent ( int e )
{
	switch ( e )
	{	case EvAdd: add_mynode(); return 1;

		case EvInfo:
		{	if ( output().len()>0 ) { output(""); return 1; }
			output_pos(0,30);
			activate_ogl_context(); // we need an active context
			GsOutput o; o.init(output()); gl_print_info(&o); // print info to viewer
			return 1;
		}

		case EvExit: gs_exit();
	}
	return WsViewer::uievent(e);
}
