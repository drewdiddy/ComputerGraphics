
# include "my_viewer.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>

# include <sigogl/ws_run.h>


SnGroup *torso = new SnGroup;
int count = 0;
bool leftL = false; bool rightL = false; bool leftA = false; bool rightA = false;
bool front = false; bool back = false; bool left = false; bool right = false;
bool step = true; bool side = false; bool mode = false;
float finc = 0.0f;
float sinc = 0.0f;


MyViewer::MyViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{
	_torus_n = 30;
	_torus_R = 14.0f;
	_torus_r = 2.5f;
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
	
	SnTransform* transform = new SnTransform;
	SnGroup* g = new SnGroup;
	SnLines* l = new SnLines;
	GsMat matrix;

	matrix.translation(p);
	if (count == 3) {
		matrix.rotx(0, 1);
		matrix.setrans(-20, 15, 20);
	}
	if (count == 6) {
		matrix.rotx(1, 0);
		matrix.setrans(-30, 25, -30);
	}
	if (count > 6) {
		torso->add(transform);
		//torso->add(s);
	}
	transform->set(matrix);
	g->add(transform);
	g->separator(true);
	g->add(s);
	g->add(l);

	// manip->visible(false); // call this to turn off mouse interaction
	count++;
	rootg()->add(g);
}

void MyViewer::build_scene ()
{
	SnPrimitive* p;

	//scene background objects
	p = new SnPrimitive(GsPrimitive::Box, 50, .25, 50);
	p->prim().material.diffuse = GsColor::darkred;
	add_model(p, GsVec(0, -4, 0));

	p = new SnPrimitive(GsPrimitive::Sphere, 2);
	p->prim().material.diffuse = GsColor::white;
	add_model(p, GsVec(25, 25, 25)); 

	p = new SnPrimitive(GsPrimitive::Sphere, 15);
	p->prim().material.diffuse = GsColor::yellow;
	add_model(p, GsVec(50, 50, -50));

	p = new SnPrimitive(GsPrimitive::Ellipsoid, 7.0, 2.25);
	p->prim().material.diffuse = GsColor::darkgray;
	add_model(p, GsVec(-20, 0, 30));

	p = new SnPrimitive(GsPrimitive::Sphere, 5);
	p->prim().material.diffuse = GsColor::darkblue;
	add_model(p, GsVec(-20, 20, 20));

	p = new SnPrimitive(GsPrimitive::Sphere, 9);
	p->prim().material.diffuse = GsColor::cyan;
	add_model(p, GsVec(-30, 25, -30));

	//textured obj
	_torus = new SnModel;
	_torus->color(GsColor::red);
	add_model(_torus, GsVec(-30, 25, -30));
	make_torus();

	// alien body construction
	//rootg #7
	p = new SnPrimitive(GsPrimitive::Ellipsoid, 2.0, 0.5);
	p->prim().material.diffuse = GsColor::darkgreen;
	add_model(p, GsVec(0, -1, 0));

	p = new SnPrimitive(GsPrimitive::Cylinder, 0.25, 0.25, 0.5);
	p->prim().material.diffuse = GsColor::blue;
	add_model(p, GsVec(0, 1, 0));

	p = new SnPrimitive(GsPrimitive::Sphere, 1);
	p->prim().material.diffuse = GsColor::darkgreen;
	add_model(p, GsVec(0, 2, 0));

	//legs
	p = new SnPrimitive(GsPrimitive::Capsule, .25, .25, 1);
	p->prim().material.diffuse = GsColor::blue;
	add_model(p, GsVec(-0.5f, -3, 0));

	p = new SnPrimitive(GsPrimitive::Capsule, .25, .25, 1);
	p->prim().material.diffuse = GsColor::blue;
	add_model(p, GsVec(0.5f, -3, 0));

	//arms
	p = new SnPrimitive(GsPrimitive::Capsule, .25, .25, 1);
	p->prim().material.diffuse = GsColor::blue;
	add_model(p, GsVec(-1, -1, 0));

	p = new SnPrimitive(GsPrimitive::Capsule, .25, .25, 1);
	p->prim().material.diffuse = GsColor::blue;
	add_model(p, GsVec(1, -1, 0));
}

inline GsVec evaluate_torus_point(double R, double r, double theta, double phi)
{
	return GsVec((R + r * cos(theta)) * cos(phi), (R + r * cos(theta)) * sin(phi), r * sin(theta));
}

inline GsVec torus_axis_point(double R, double r, double theta, double phi)
{
	return GsVec(R * cos(phi), R * sin(phi), 0.0);
}

void MyViewer::texturize(GsModel* m) {
	//Texture init
	GsModel::Group& g = *m->G.push();
	g.fi = 0;
	g.fn = m->F.size();
	g.dmap = new GsModel::Texture;
	g.dmap->fname.set("../saturn.jpg");

	m->M.push().init();

	double inc = GS_2PI / _torus_n;
	double theta = 0; double phi = 0;

	for (theta = 0; theta <= GS_2PI; theta += inc) {
		m->T.push(GsPnt2(theta / GS_2PI, 0.0));
		m->T.push(GsPnt2(double(theta + inc) / GS_2PI, 0.0));
		for (phi = 0; phi <= GS_2PI; phi += inc) {
			m->T.push(GsPnt2(theta / GS_2PI, double(phi + inc) / GS_2PI));
			m->T.push(GsPnt2(double(theta + inc) / GS_2PI, double(phi + inc) / GS_2PI));
		}
	}
}

void MyViewer::make_torus()
{
	GsModel* m = _torus->model();  // access model definition class in the node
	m->init();

	double inc = GS_2PI / _torus_n;
	GsVec axisp;
	double theta = 0; double phi = 0;

	// (Note: this implementation can still be improved to remove all duplicate vertices!)
	for (theta = 0; theta <= GS_2PI; theta = theta + inc)
	{
		m->V.push() = evaluate_torus_point(_torus_R, _torus_r, theta, 0);
		m->V.push() = evaluate_torus_point(_torus_R, _torus_r, theta + inc, 0);
		
		axisp = torus_axis_point(_torus_R, _torus_r, theta, 0);
		m->N.push() = normalize(m->V.top(1) - axisp);
		m->N.push() = normalize(m->V.top() - axisp);

		for (phi = 0; phi <= GS_2PI; phi = phi + inc)
		{
			m->V.push() = evaluate_torus_point(_torus_R, _torus_r, theta, phi + inc);
			m->V.push() = evaluate_torus_point(_torus_R, _torus_r, theta + inc, phi + inc);

			int i = m->V.size() - 4;
			m->F.push() = GsModel::Face(i, i + 2, i + 3);
			m->F.push() = GsModel::Face(i + 3, i + 1, i);

			axisp = torus_axis_point(_torus_R, _torus_r, theta, phi + inc);
			m->N.push() = normalize(m->V.top(1) - axisp);
			m->N.push() = normalize(m->V.top() - axisp);

		}
	}

	texturize(m);

	m->M.push().init();

	m->set_mode(GsModel::Smooth, GsModel::PerGroupMtl);
	m->textured = true;
}

void MyViewer::move2() {
	SnTransform* t = torso->get<SnTransform>(0);
	SnTransform* t1 = torso->get<SnTransform>(1);
	SnTransform* t2 = torso->get<SnTransform>(2);
	SnTransform* t3 = torso->get<SnTransform>(3);
	SnTransform* t4 = torso->get<SnTransform>(4);
	SnTransform* t5 = torso->get<SnTransform>(5);
	SnTransform* t6 = torso->get<SnTransform>(6);

	GsMat m = t->get();
	GsMat m1 = t1->get();
	GsMat m2 = t2->get();
	GsMat m3 = t3->get();
	GsMat m4 = t4->get();
	GsMat m5 = t5->get();
	GsMat m6 = t6->get();

	if (front == true || back == true) {
		m.e34 = finc;
		t->set(m);
		m1.e34 = finc;
		t1->set(m1);
		m2.e34 = finc;
		t2->set(m2);
		m3.e34 = finc;
		t3->set(m3);
		m4.e34 = finc;
		t4->set(m4);
		m5.e34 = finc;
		t5->set(m5);
		m6.e34 = finc;
		t6->set(m6);
		render();
		if (step == true) {
			for (int i = 0; i < 4; i++) {
				leftL = true;
				run_animation(1);
				rightL = true;
				run_animation(-1);
			}
			step = false;
		}
		else {
			for (int i = 0; i < 4; i++) {
				leftL = true;
				run_animation(-1);
				rightL = true;
				run_animation(1);
			}
			step = true;
		}
		if (front == true)
			front = false;
		else
			back = false;
	}
	else if (left == true || right == true) {
		m.e14 = sinc;
		t->set(m);
		m1.e14 = sinc;
		t1->set(m1);
		m2.e14 = sinc;
		t2->set(m2);
		m3.e14 = sinc - .5f;
		t3->set(m3);
		m4.e14 = sinc + .5f;
		t4->set(m4);
		m5.e14 = sinc - 1;
		t5->set(m5);
		m6.e14 = sinc + 1;
		t6->set(m6);
		render();
		if (step == true) {
			for (int i = 0; i < 3; i++) {
				leftL = true; rightL = true;
				run_animation(-1);
			}
			step = false;
		}
		else {
			for (int i = 0; i < 3; i++) {
				leftL = true;  rightL = true;
				run_animation(1);
			}
			step = true;
		}
		if (left == true)
			left = false;
		else
			right = false;
	}

}

void MyViewer::move(SnGroup* limb, SnTransform* t, GsMat m, float n){

	GsMat f;
	GsMat tr;
	GsMat rotate;
	GsMat origin;

	float theta = n * float(GS_2PI / 90);

	float e14 = m.e14;
	float e24 = m.e24;
	float e34 = m.e34;
	tr.translation(0, 0.5f, 0);
	e14 = -1 * e14;
	e24 = -1 * e24;
	e34 = -1 * e34;
	//origin.set(14, e14);
	//origin.set(24, e24);
	//origin.set(34, e34);

	origin.translation(0, -.5f, 0);
	//if (leftA == true || rightA == true) {
	if (side == true)
		rotate.rotz(theta);
	else
		rotate.rotx(theta);

	f = m * tr * rotate * origin;

	t->set(f);
}

// Below is an example of how to control the main loop of an animation:
void MyViewer::run_animation (float n)
{
	if ( _animating ) return; // avoid recursive calls
	_animating = true;

	if (leftL == true && rightL == true) {
		side = true;
		SnGroup* leftLeg = rootg()->get<SnGroup>(10);
		SnTransform* lLeg = leftLeg->get<SnTransform>(0);
		GsMat m = lLeg->get();
		move(leftLeg, lLeg, m, n);
		leftL = false;

		SnGroup* rightLeg = rootg()->get<SnGroup>(11);
		SnTransform* rLeg = rightLeg->get<SnTransform>(0);
		m = rLeg->get();
		move(rightLeg, rLeg, m, -n);
		rightL = false;
		side = false;
	}
	if (leftL == true) {
		SnGroup* leftLeg = rootg()->get<SnGroup>(10);
		SnTransform* lLeg = leftLeg->get<SnTransform>(0);
		GsMat m = lLeg->get();
		move(leftLeg, lLeg, m, n);
		leftL = false;
	}
	else if (rightL == true) {
		SnGroup* rightLeg = rootg()->get<SnGroup>(11);
		SnTransform* rLeg = rightLeg->get<SnTransform>(0);
		GsMat m = rLeg->get();
		move(rightLeg, rLeg, m, n);
		rightL = false;
	}
	else if (leftA == true) {
		SnGroup* leftArm = rootg()->get<SnGroup>(12);
		SnTransform* lArm = leftArm->get<SnTransform>(0);
		GsMat m = lArm->get();
		move(leftArm, lArm, m, n);
		leftA = false;
	}
	else if (rightA == true) {
		SnGroup* rightArm = rootg()->get<SnGroup>(13);
		SnTransform* rArm = rightArm->get<SnTransform>(0);
		GsMat m = rArm->get();
		move(rightArm, rArm, m, n);
		rightA = false;
	}
	render();
	//ws_check();
	/*
	int ind = gs_random ( 0, rootg()->size()-1 ); // pick one child
	SnManipulator* manip = rootg()->get<SnManipulator>(ind); // access one of the manipulators
	GsMat m = manip->mat();

	double frdt = 1.0/30.0; // delta time to reach given number of frames per second
	double v = 4; // target velocity is 1 unit per second
	double t=0, lt=0, t0=gs_time();
	do // run for a while:
	{	while ( t-lt<frdt ) { ws_check(); t=gs_time()-t0; } // wait until it is time for next frame
		double yinc = (t-lt)*v;
		if ( t>2 ) yinc=-yinc; // after 2 secs: go down
		lt = t;
		m.e24 += (float)yinc;
		if ( m.e24<0 ) m.e24=0; // make sure it does not go below 0
		manip->initial_mat ( m );
		render(); // notify it needs redraw
		ws_check(); // redraw now
	}	while ( m.e24>0 );
	*/
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

		case 'q': leftL = true; run_animation(-1); break;
		case 'a': leftL = true; run_animation(1); break;
		case 'w': rightL = true; run_animation(-1); break;
		case 's': rightL = true; run_animation(1); break;
		case 'e': leftA = true; run_animation(-1); break;
		case 'd': leftA = true; run_animation(1); break;
		case 'r': rightA = true; run_animation(-1); break;
		case 'f': rightA = true; run_animation(1); break;
		case 't': leftL = true; rightL = true; run_animation(-1); break;
		case 'g': leftL = true; rightL = true; run_animation(1); break;
		case 65361: sinc -= 0.1f; left = true; move2(); break;
		case 65362: finc += 0.1f; front = true; move2(); break;
		case 65364: finc -= 0.1f; back = true; move2(); break;
		case 65363: sinc += 0.1f; right = true; move2(); break;
		case ' ': {
			double lt, t0 = gs_time();
			if (mode == false) {
				do
				{
					lt = gs_time() - t0;
					camera().eye.x += 0.05f;
					camera().center.x += 0.05f;
					camera().eye.y += 0.05f;
					camera().center.y += 0.05f;
					camera().eye.z += 0.05f;
					camera().center.z += 0.05f;
					//camera().up.x += 0.001f;
					render();
					ws_check();
					//message().setf(“localtime = % f”, lt);
				} while
					(lt < 10.0f);
				mode = true;
			}
			else {
				camera().eye.x = 26.1714f;
				camera().center.x = 4.95669f;
				camera().up.x = 0.548562f;
				camera().eye.y = 60.6475f;
				camera().center.y = 19.0075f;
				camera().up.y = 0.775398f;
				camera().eye.z = 146.247f;
				camera().center.z = 5.86001f;
				camera().up.z = -0.312884f;
				//camera().fovy = 46.8147f;

				render();
				mode = false;
			}
			break;
		}
		//case 'n' : { bool b=!_nbut->value(); _nbut->value(b); show_normals(b); return 1; }
		default: gsout<<"Key pressed: "<<e.key<<gsnl;
	}

	return 0;
}

int MyViewer::uievent ( int e )
{
	switch ( e )
	{	case EvNormals: show_normals(_nbut->value()); return 1;
		case EvAnimate: run_animation(1); return 1;
		case EvExit: gs_exit();
	}
	return WsViewer::uievent(e);
}
