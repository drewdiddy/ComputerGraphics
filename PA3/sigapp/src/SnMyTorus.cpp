# include "SnMyTorus.h"

const char* SnMyTorus::class_name = "SnMyTorus"; // static
SN_SHAPE_RENDERER_DEFINITIONS(SnMyTorus);

//===== SnMyTorus =====

SnMyTorus::SnMyTorus() : SnShape(class_name)
{
	//width = 0.5f;
	//height = 1.0f;

	r = 0.5f;
	R = 5.0f;
	resolution = 0.1f;
	uArm = false;
	lArm = false;
	hands = false;

	if (!SnMyTorus::renderer_instantiator) SnMyNodeRegisterRenderer();
}

SnMyTorus::~SnMyTorus()
{
}

void SnMyTorus::get_bounding_box(GsBox& b) const
{
	// the bounding box is needed for camera view_all computations
	b.set(init, init + GsVec(r, R, 0));
}

//===== Renderer Instantiator =====

# include "glr_mynode.h"

static SnShapeRenderer* GlrMyNodeInstantiator()
{
	return new GlrMyNode;
}

void SnMyNodeRegisterRenderer()
{
	SnMyTorus::renderer_instantiator = &GlrMyNodeInstantiator;
}
