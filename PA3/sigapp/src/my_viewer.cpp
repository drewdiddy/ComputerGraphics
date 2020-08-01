
# include "my_viewer.h"
# include "SnMyTorus.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>
# include <sig/sn_model.h>

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
	//makes matrix with point p
	matrix.translation(p);

	if (p == GsVec(0, 0, -28)) {
		//rotates the upper arm to look more realistic
		matrix.rotx(gs2pi / 6);
		//transforms it back to where its supposed to be, keeping the previous rotation
		matrix.rcombtrans(GsVec(0, 0, -28));
	}
	
	transform->set(matrix);
	g->add(transform);
	g->separator(true);
	g->add(s);

	//add transform, separator, shape, and lines to a group
	//g->add(transform);
	//g->separator(true);
	//g->add(s);
	g->add(l);

	//add the group to our root
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
	SnModel* upperArm = new SnModel;
	bool loaded = false;
	upperArm->model()->load("C:/Users/haloc/Desktop/PA3/arm/rupperarm.m");
	add_model(upperArm, GsVec(0, 0, -28));

	SnModel* lowerArm = new SnModel;
	lowerArm->model()->load("C:/Users/haloc/Desktop/PA3/arm/rlowerarm.m");
	add_model(lowerArm, GsVec(0, 0, 0));

	SnModel* hand = new SnModel;
	hand->model()->load("C:/Users/haloc/Desktop/PA3/arm/rhand.m");
	add_model(hand, GsVec(0, 0, 27));	
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
		
		render(); // notify it needs redraw
		ws_check(); // redraw now
	}	while (counter < 7500 );
	_animating = false;
}

void MyViewer::moveArm(float theta) {
	//float theta = (gs2pi / 60);
	float a, b, d, e, f, g, z, z1;
	float upperLeft, upperRight, lowerLeft, lowerRight;
	//float upperLeft2, upperRight2, lowerLeft2, lowerRight2;

	SnGroup* Arm = rootg()->get<SnGroup>(0); // access the group
	SnGroup* leArm = rootg()->get<SnGroup>(1);
	SnGroup* h = rootg()->get<SnGroup>(2);

	SnTransform* upArm = Arm->get<SnTransform>(0); //access the transforms
	SnTransform* lowArm = leArm->get<SnTransform>(0);
	SnTransform* han = h->get<SnTransform>(0);

	GsMat upperArm = upArm->get();
	GsMat lowerArm = lowArm->get();
	GsMat hand = han->get();

	//upperArm movement
	if (c->uArm == true) {

		a = upperArm.get(14);
		b = upperArm.get(24);
		d = upperArm.get(34);
		
		e = -a;
		f = -b;
		g = -d;

		z = lowerArm.get(34);
		z1 = hand.get(34);

		GsMat origin;
		GsMat temp, temp2;
		origin.setl1(1, 0, 0, e);
		origin.setl2(0, 1, 0, f);
		origin.setl3(0, 0, 1, g);

		upperLeft = float(cos(theta));	//cos()
		lowerLeft = float(sin(theta));	//sin()
		upperRight = -1 * lowerLeft;	//-sin()
		lowerRight = upperLeft;			//cos()

		GsMat rotateMatrix;

		rotateMatrix.setl2(0, upperLeft, upperRight, 0);
		rotateMatrix.setl3(0, lowerLeft, lowerRight, 0);

		temp = origin * rotateMatrix * upperArm;
		upArm->set(temp);

		/*temp2 = lowerArm;

		if (c->lArm == true) {
			temp2.rcombtrans(GsVec(0, 0, z - 1.5f));
			lowArm->set(temp2);
			c->lArm = false;
		}
		else {
			temp2.rcombtrans(GsVec(0, 0, z + 1.5f));
			lowArm->set(temp2);
		}
		*/


		temp = lowerArm * rotateMatrix;
		//temp.rcombtrans(GsVec(e, f, g));
		lowArm->set(temp);

		temp = hand * rotateMatrix;
		han->set(temp);
		
		//origin.identity();

		c->uArm = false;
	}
	else if (c->lArm == true) {

		a = lowerArm.get(14);
		b = lowerArm.get(24);
		d = lowerArm.get(34);
	
		e = -a;
		f = -b;
		g = -d;

		//z = lowerArm.get(34);
		//z1 = hand.get(34);

		GsMat origin;
		GsMat temp;

		origin.setl1(1, 0, 0, e);
		origin.setl2(0, 1, 0, f);
		origin.setl3(0, 0, 1, g);

		upperLeft = float(cos(theta));	//cos()
		lowerLeft = float(sin(theta));	//sin()
		upperRight = -1 * lowerLeft;	//-sin()
		lowerRight = upperLeft;			//cos()

		GsMat rotateMatrix;

		rotateMatrix.setl2(0, upperLeft, upperRight, 0);
		rotateMatrix.setl3(0, lowerLeft, lowerRight, 0);

		lowerArm = rotateMatrix * lowerArm * origin;
		lowArm->set(lowerArm);

		a = hand.get(14);
		b = hand.get(24);
		d = hand.get(34);

		e = -a;
		f = -b;
		g = -d;

		origin.setl1(1, 0, 0, e);
		origin.setl2(0, 1, 0, f);
		origin.setl3(0, 0, 1, g);

		hand = rotateMatrix * hand * origin;
		han->set(hand);

		c->lArm = false;
	}
	else if (c->hands == true) {
		upperLeft = float(cos(theta));	//cos()
		lowerLeft = float(sin(theta));	//sin()
		upperRight = -1 * lowerLeft;	//-sin()
		lowerRight = upperLeft;			//cos()

		GsMat rotateMatrix;

		rotateMatrix.setl2(0, upperLeft, upperRight, 0);
		rotateMatrix.setl3(0, lowerLeft, lowerRight, 0);

		//hand = rotateMatrix * hand;
		hand.rotx(theta);
		hand.rcombtrans(GsVec(0, 0, 27));
		han->set(hand);

		c->hands = false;
	}

	render();
	ws_check();

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
		case 'q': {
			c->uArm = true;
			moveArm(gs2pi/60);
			return 1; }
		case 'a': {
			c->uArm = true;
			moveArm(-gs2pi / 60);
			return 1; }
		case 'w': {
			c->lArm = true;
			moveArm(gs2pi / 60);
			return 1; }
		case 's': {
			c->lArm = true;
			moveArm(-gs2pi / 60);
			return 1; }
		case 'e': {
			c->hands = true;
			moveArm(gs2pi / 360);
			return 1; }
		case 'd': {
			c->hands = true;
			moveArm(-gs2pi / 360);
			return 1; }
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
