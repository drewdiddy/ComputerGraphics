
# include "my_viewer.h"
# include "SnMyTorus.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>

# include <sigogl/ws_run.h>

MyViewer::MyViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{
	_nbut=0;
	_animating=false;
	build_ui ();
	build_scene ();
}

void MyViewer::build_ui ()
{
	UiPanel *p, *sp;
	UiManager* uim = WsWindow::uim();
	p = uim->add_panel ( "", UiPanel::HorizLeft );
	p->add ( new UiButton ( "View", sp=new UiPanel() ) );
	{	UiPanel* p=sp;
		p->add ( _nbut=new UiCheckButton ( "Normals", EvNormals ) ); 
	}
	p->add ( new UiButton ( "Animate", EvAnimate ) );
	p->add ( new UiButton ( "Exit", EvExit ) ); p->top()->separate();
}

bool MyViewer::checkSpace() {
	if (c->space == true)
		return true;
	else
		return false;
}

void MyViewer::add_model ( SnShape* s, GsVec p )
{
	// This method demonstrates how to add some elements to our scene graph: lines,
	// and a shape, and all in a group under a SnManipulator.
	// Therefore we are also demonstrating the use of a manipulator to allow the user to
	// change the position of the object with the mouse. If you do not need mouse interaction,
	// you can just use a SnTransform to apply a transformation instead of a SnManipulator.
	// You would then add the transform as 1st element of the group, and set g->separator(true).
	// Your scene graph should always be carefully designed according to your application needs.

	SnGroup* g = new SnGroup;
	SnLines* l = new SnLines;
	SnTransform* transform = new SnTransform;
	GsMat matrix;

	matrix.translation(p);
	if (p == GsVec(0, 0, 0)) {
		matrix.rotx(1, 0);
	}
	transform->set(matrix);
	g->add(transform);
	g->separator(true);
	g->add(s);
	g->add(l);

	c->color (GsColor::darkred );
	rootg()->add(c);
	rootg()->add(g);


	/*
	SnManipulator* manip = new SnManipulator;
	GsMat m;
	m.translation ( p );
	manip->initial_mat ( m );

	SnGroup* g = new SnGroup;
	SnLines* l = new SnLines;
	l->color(GsColor::orange);
	g->add(s);
	g->add(l);
	manip->child(g);
	// manip->visible(false); // call this to turn off mouse interaction

	rootg()->add(manip);
	*/
}

void MyViewer::build_scene ()
{
	SnPrimitive* p;

	p = new SnPrimitive(GsPrimitive::Cylinder, 5.0f, 5.0f, 0.05f);
	p->prim().material.diffuse = GsColor::magenta;
	add_model(p, GsVec(0, 0, 0));
	
	p = new SnPrimitive(GsPrimitive::Cylinder, 0.25f, 0.1f, 2.0);
	p->prim().material.diffuse = GsColor::darkred;
	add_model(p, GsVec(0, 2, 0));

	p = new SnPrimitive(GsPrimitive::Cylinder, 0.25f, 0.1f, 1.0);
	p->prim().material.diffuse = GsColor::darkblue;
	add_model(p, GsVec(0, 1, 0.25f));

}

// Below is an example of how to control the main loop of an animation:
void MyViewer::run_animation ()
{
	if ( _animating ) return; // avoid recursive calls
	_animating = true;
	int counter = 0;
	//int ind = gs_random ( 1, rootg()->size()-1 ); // pick one child
	SnGroup* lHand = rootg()->get<SnGroup>(3); // access one of the manipulators
	SnGroup* sHand = rootg()->get<SnGroup>(5);
	
	SnTransform* longHand  = lHand->get<SnTransform>(0);
	SnTransform* shortHand = sHand->get<SnTransform>(0);
	
	GsMat m = longHand->get();
	GsMat M = shortHand->get();
	GsMat tback;
	GsMat tback2;
	float e14 = tback.get(14);
	float e24 = tback.get(24);
	float e34 = tback.get(34);
	e14 = -1 * e14;
	e24 = -1 * e24;
	e34 = -1 * e34;

	float sec_e14 = tback2.get(14);
	float sec_e24 = tback2.get(24);
	float sec_e34 = tback2.get(34);
	sec_e14 = -1 * sec_e14;
	sec_e24 = -1 * sec_e24;
	sec_e34 = -1 * sec_e34;

	tback.set(14, e14);
	tback.set(24, e24);
	tback.set(34, e34);

	float theta = 90.0f;
	float phi = 90.0f;
	float upperLeft, upperRight, lowerLeft, lowerRight, upperLeft2, upperRight2, lowerLeft2, lowerRight2;
	bool checkSpace = false, checkEnter = false;

	double frdt = 1.0/30.0; // delta time to reach given number of frames per second
	double v = 4; // target velocity is 1 unit per second
	double t=0, lt=0, t0=gs_time();
	do // run for a while:
	{	while ( t-lt<frdt ) { ws_check(); t=gs_time()-t0; } // wait until it is time for next frame

		/*double yinc = (t-lt)*v;
		if ( t>2 ) yinc=-yinc; // after 2 secs: go down
		lt = t;
		m.e24 += (float)yinc;
		if ( m.e24<0 ) m.e24=0; // make sure it does not go below 0	
		*/
		theta = -(gs2pi / 60);

		upperLeft = float(cos(theta));
		lowerLeft = float(sin(theta));
		upperRight = -1 * lowerLeft;
		lowerRight = upperLeft;

		GsMat rotateMatrix;

		rotateMatrix.setl1(upperLeft, upperRight, 0, 0);
		rotateMatrix.setl2(lowerLeft, lowerRight, 0, 0);

		m = tback * rotateMatrix * m;
		longHand->set(m);

		phi = -(gs2pi / 3600);

		upperLeft2 = float(cos(phi));
		lowerLeft2 = float(sin(phi));
		upperRight2 = -1 * lowerLeft2;
		lowerRight2 = upperLeft2;

		GsMat rotateMatrix2;
		
		rotateMatrix2.setl1(upperLeft2, upperRight2, 0, 0);
		rotateMatrix2.setl2(lowerLeft2, lowerRight2, 0, 0);

		M = tback2 * rotateMatrix2 * M;
		shortHand->set(M);
		if (c->enter == true) {
			theta = 90.0f;
			phi = 90.0f;
			c->enter = false;
		}
		render(); // notify it needs redraw
		ws_check(); // redraw now
	}	while (counter < 7500 );
	_animating = false;
}

void MyViewer::show_normals ( bool view )
{
	// Note that primitives are only converted to meshes in GsModel
	// at the first draw call.
	GsArray<GsVec> fn;
	SnGroup* r = (SnGroup*)root();
	for ( int k=0; k<r->size(); k++ )
	{	SnManipulator* manip = r->get<SnManipulator>(k);
		SnShape* s = manip->child<SnGroup>()->get<SnShape>(0);
		SnLines* l = manip->child<SnGroup>()->get<SnLines>(1);
		if ( !view ) { l->visible(false); continue; }
		l->visible ( true );
		if ( !l->empty() ) continue; // build only once
		l->init();
		if ( s->instance_name()==SnPrimitive::class_name )
		{	GsModel& m = *((SnModel*)s)->model();
			m.get_normals_per_face ( fn );
			const GsVec* n = fn.pt();
			float f = 0.33f;
			for ( int i=0; i<m.F.size(); i++ )
			{	const GsVec& a=m.V[m.F[i].a]; l->push ( a, a+(*n++)*f );
				const GsVec& b=m.V[m.F[i].b]; l->push ( b, b+(*n++)*f );
				const GsVec& c=m.V[m.F[i].c]; l->push ( c, c+(*n++)*f );
			}
		}  
	}
}

int MyViewer::handle_keyboard ( const GsEvent &e )
{
	int ret = WsViewer::handle_keyboard ( e ); // 1st let system check events
	if ( ret ) return ret;

	switch ( e.key )
	{	case GsEvent::KeyEsc : gs_exit(); return 1;
		case 'n' : { bool b=!_nbut->value(); _nbut->value(b); show_normals(b); return 1; }
		case 32: {
			if (c->space == true)
				c->space = false;
			else
				c->space = true;
			return 1;
		}
		case 65293: {
			if (c->enter == false)
				c->enter = true;
		}
		default: gsout<<"Key pressed: "<<e.key<<gsnl;
	}

	return 0;
}

int MyViewer::uievent ( int e )
{
	switch ( e )
	{	case EvNormals: show_normals(_nbut->value()); return 1;
		case EvAnimate: run_animation(); return 1;
		case EvExit: gs_exit();
	}
	return WsViewer::uievent(e);
}
