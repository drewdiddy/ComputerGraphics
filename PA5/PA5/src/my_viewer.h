# pragma once

# include <sig/sn_lines2.h>
# include <sigogl/ui_button.h>
# include <sigogl/ui_slider.h>
# include <sigogl/ws_viewer.h>
# include <sig/sn_model.h>
# include <sig/sn_lines.h>

class MyViewer : public WsViewer
{  protected :
	enum MenuEv { EvNormals, EvSlider, EvExit };
	UiSlider* _slider;
	int _torus_n;
	float _torus_R, _torus_r;
	SnModel *_torus;
	SnLines *_normals;
	bool _flat;
	int mode;
   public :
	MyViewer ( int x, int y, int w, int h, const char* l );
	void build_ui ();
	void build_scene ();
	void make_torus ( bool flat );
	void texturize(GsModel* m);
	void make_normals ( bool flat );
	virtual int handle_keyboard ( const GsEvent &e ) override;
	virtual int uievent ( int e ) override;
};

