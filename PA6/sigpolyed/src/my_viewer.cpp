
# include "my_viewer.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>

# include <sigogl/ws_run.h>

bool D3 = false;

static void my_polyed_callback ( SnPolyEditor* pe, enum SnPolyEditor::Event e, int pid )
{
	MyViewer* v = (MyViewer*)pe->userdata();
	if ( e==SnPolyEditor::PostMovement || e==SnPolyEditor::PostEdition || e==SnPolyEditor::PostInsertion )
	{	v->update_scene ();
	}
}

MyViewer::MyViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{
	//_curve = new SnModel;
	//_curve->color(GsColor::green);
	rootg()->add ( _polyed = new SnPolyEditor );
	//rootg()->add(_curve);
	rootg()->add ( _curveA = new SnLines2 );
	rootg()->add(_curveB = new SnLines2);
	rootg()->add(_curveC = new SnLines2);
	rootg()->add(_curveD = new SnLines2);
	

	_curveA->color ( GsColor(20,200,25) );
	_curveA->line_width ( 2.0f );

	_curveB->color(GsColor::darkred);
	_curveB->line_width(2.0f);

	_curveC->color(GsColor::magenta);
	_curveC->line_width(2.0f);

	_curveD->color(GsColor::cyan);
	_curveD->line_width(2.0f);


	// set initial control polygon:
	_polyed->callback ( my_polyed_callback, this );
	_polyed->max_polygons (1);	/*! Limits the maximum allowed number of polygons to be created, if -1 is given no limit is used. */
	_polyed->solid_drawing (0);	/*! Determines polygon draw mode: 0:draw edges, 1:draw interior, 2:draw both */
	GsPolygon& P = _polyed->polygons()->push();
	P.setpoly ( "-2 -2  -1 1  1 0  2 -2" );	/*! Defines the polygon from a string of coordinates */
	P.open(true);	/*! An open polygon is equivalent to a polygonal line */

	// start:
	build_ui ();
	update_scene ();
	message() = "Click on polygon to edit, use Esc to switch edition mode, Del deletes selected points. Enjoy!";
}

void MyViewer::build_ui ()
{
	UiPanel *p;
	p = uim()->add_panel ( "", UiPanel::HorizLeft );

	p->add ( _viewA=new UiCheckButton ( "CurveA", EvViewCurveA, true ) );
	p->add ( _viewB=new UiCheckButton ( "CurveB", EvViewCurveB, true ) );
	p->add(_viewC = new UiCheckButton("CurveC", EvViewCurveC, true));
	p->add(_viewD = new UiCheckButton("CurveD", EvViewCurveD, true));

	p->add ( _slider=new UiSlider ( " dt:", EvDeltaT, 0,0,150 ) );
	_slider->separate();
	_slider->range ( 0.01f, 0.25f );

	p->add ( new UiButton ( "Exit", EvExit ) ); p->top()->separate();
}

int fact(int n) {
	if (n == 0) {					//since 0! = 1
		return 1;
	}
	else {
		return n * fact(n - 1);		//recurssive call to find factorial of n
	}
}

// Use one function for each curve type. Feel free to update/adapt parameter list etc.
static GsPnt2 eval_bezier ( float t, const GsArray<GsPnt2>& P )
{
	if (D3 == true) {
		//GsModel* m = curve->model();
		//m->init();

		GsPnt2 point;
		int n = P.size() - 1;
		for (int i = 0; i < P.size(); i++) {
			float c = float(fact(n) / (fact(i) * fact(n - i)));
			point += P[i] * c * powf(t, float(i)) * powf((1 - t), float(n - i));
		}


		D3 = false;
		return point;
	}
	else {
		GsPnt2 point;
		int n = P.size() - 1;
		for (int i = 0; i < P.size(); i++) {
			float c = float(fact(n) / (fact(i) * fact(n - i)));		//ways of picking i unordered outcomes from n
			point += P[i] * c * powf(t, float(i)) * powf((1 - t), (float(n - i)));	//calculation for bezier
		}

		return point;
	}
}

float helper(int i, int k, float u){
	float ui = float(i);
	if (k == 1)
		return ui <= u && u < ui + 1 ? 1.0f : 0;
	else 
		return((u - ui) / (k - 1)) * helper(i, k - 1, u) + ((ui + k - u) / (k - 1)) * helper(i + 1, k - 1, u);
}

static GsPnt2 eval_bspline( float t, const GsArray<GsPnt2>& P )
{
	GsPnt2 point;
	for(int i = 0; i < P.size(); i++){
		point += P[i] * helper(i, 3, t);		//for order k = 3, (degree 2, quadratic case)
	}

	return point;
}

static GsPnt2 eval_crspline(float t, const GsArray<GsPnt2>& P) {
	int i = int(floor(t+1));		//ensures that the first point will start on the second CP
	float t2 = t - float(floor(t));	//to send the updated t value to bezier
	//need 4 points, 2 I's, and somewhere to store result
	GsPnt2 pointA, pointB, pointC, pointD, pointI, pointI2, result;
	//create an array to send our four points to bezier
	GsArray<GsPnt2> arr(4);
	pointA = P[i];
	pointD = P[i + 1];
	pointI = (P[i + 1] - P[i - 1]) / 2;
	i += 1;
	pointI2 = (P[i + 1] - P[i-1]) / 2;

	pointB = pointA + (pointI / 3);
	pointC = pointD - (pointI2 / 3);

	arr.set(0, pointA);
	arr.set(1, pointB);
	arr.set(2, pointC);
	arr.set(3, pointD);

	/*pointA = arr.push();
	pointB = arr.push();
	pointC = arr.push();
	pointD = arr.push();*/

	result = eval_bezier(t2, arr);
	//arr.remove();
	return result;
}

static GsPnt2 eval_bospline(float t, const GsArray<GsPnt2>& P) {
	GsPnt2 point;

	return point;
}

void MyViewer::update_scene ()
{
	// Initializations:
	_curveA->init();
	_curveB->init();
	_curveC->init();
	_curveD->init();

	// Access the control polygon:
	GsPolygon& P = _polyed->polygon(0);
	float deltat = _slider->value();
	if ( _viewA->value() ) // show curve
	{
		// Add your curves below and select the correct one(s) to be displayed.
		// As an example, below we have a linear interpolation between endpoints:
		_curveA->begin_polyline();
		for ( float t=0; t<1.0f; t+=deltat ) // note: the t range may change according to the curve
		{	_curveA->push ( eval_bezier( t, P ) );
		}
		_curveA->push ( P.top() ); // ensure final point is there
		_curveA->end_polyline();
	}

	if (_viewB->value())
	{
		_curveB->begin_polyline();
		for (float t = 2; t < P.size(); t += deltat)
		{
			_curveB->push(eval_bspline(t, P));
		}
		_curveB->push(P.top()); 
		_curveB->end_polyline();
	}
	if (_viewC->value())
	{
		_curveC->begin_polyline();
		for (float t = 0; t < P.size()-3; t += deltat)
		{
			_curveC->push(eval_crspline(t, P));
		}
		_curveC->push(P.top());
		_curveC->end_polyline();
	}
	if (_viewD->value())
	{
		_curveD->begin_polyline();
		for (float t = 0; t < P.size()-3; t += deltat)
		{
			_curveD->push(eval_bospline(t, P));
		}
		_curveD->push(P.top());
		_curveD->end_polyline();
	}
}

int MyViewer::handle_keyboard(const GsEvent& e)
{
	int ret = WsViewer::handle_keyboard(e);
	if (ret) return ret;
	GsMat m1, m2, m3;
	GsVec currentposition;
	switch (e.key)
	{
	case GsEvent::KeyEsc: gs_exit(); return 1;

	case ' ':  break;
	default: gsout << "Key pressed: " << e.key << gsnl;
	}
	render();
	return 0;
}

int MyViewer::uievent ( int e )
{
	switch ( e )
	{
		case EvViewCurveA:
		case EvViewCurveB:
		case EvViewCurveC:
		case EvViewCurveD:	update_scene(); return 1;
		case EvDeltaT:		update_scene(); return 1;
		case EvExit: gs_exit();
	}
	return WsViewer::uievent(e);
}
