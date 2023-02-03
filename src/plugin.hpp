#pragma once
#include <rack.hpp>

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file
// extern Model* modelMyModule;
extern Model* modelA;
extern Model *modelMu;
extern Model *modelT;
extern Model *modelL;
extern Model *modelD;
extern Model *modelR;

extern Model *modelOm;
extern Model *modelV;
extern Model *modelF;
extern Model *modelW;
extern Model *modelY;
extern Model *modelO;

extern Model *modelS;
extern Model *modelE;
extern Model *modelX;

extern Model *modelM;
extern Model *modelQ;
extern Model *modelC;

extern Model *modelU;
extern Model *modelB;
extern Model *modelJ;
extern Model *modelK;

extern Model *modelH;
extern Model *modelI;
extern Model *modelG;

extern Model *modelZ;

extern float tanpif(float f);

////////////////////
// Knobs
////////////////////

struct KRoundBlackKnob : RoundBlackKnob {
	KRoundBlackKnob() {
		setSvg(Svg::load(asset::plugin(pluginInstance, "res/RoundBlackKnob.svg")));
		bg->setSvg(Svg::load(asset::plugin(pluginInstance, "res/RoundBlackKnob_bg.svg")));
	}
};

struct KRoundBlackSnapKnob : KRoundBlackKnob {
	KRoundBlackSnapKnob() {
		snap = true;
	}
};

////////////////////
// Ports
////////////////////

struct KPJ301MPort : PJ301MPort {
	KPJ301MPort() {
		setSvg(Svg::load(asset::plugin(pluginInstance, "res/PJ301M.svg")));
		shadow->opacity = 0.0f;
	}
};

////////////////////
// Misc
////////////////////

struct KScrewSilver : ScrewSilver {
	KScrewSilver() {
		setSvg(Svg::load(asset::plugin(pluginInstance, "res/ScrewSilver.svg")));
	}
};

#define LABEL 8.0f
#define SCALE_LBL (LABEL / 18.0f)

struct LabelWidget : LightWidget {//TransparentWidget {
	const char *what;
	int kind;
	const std::string fontPath = asset::system("res/fonts/DSEG7ClassicMini-Regular.ttf");

	LabelWidget(const char *p, const int k) {
		what = p;
		kind = k;
	}

	void drawLayer(const DrawArgs& args, int layer) override {
		if (layer == 1 /* || layer == 0 */) {
			drew(args);
		}
		Widget::drawLayer(args, layer);
	}

	void drew(const DrawArgs &args) {//foreground
		std::shared_ptr<Font> font;
		if (!(font = APP->window->loadFont(fontPath))) {
			return;
		}
		NVGcolor textColor;
		switch(kind) {
			case -1:
				textColor = SCHEME_GREEN;
				break;
			case 0: default:
				textColor = SCHEME_YELLOW;
				break;
			case 1:
				textColor = SCHEME_RED;
				break;
		}
		nvgFontFaceId(args.vg, font->handle);
		nvgFontSize(args.vg, LABEL);

		Vec textPos = Vec(4 * SCALE_LBL, 22 * SCALE_LBL);
		nvgFillColor(args.vg, textColor);
		nvgText(args.vg, textPos.x, textPos.y, what, NULL);
	}

	void fixCentre(Vec here, int many) {//locate control
		box.size = Vec((14.6f * many + 4) * SCALE_LBL, 26 * SCALE_LBL);
		box.pos = Vec(here.x - box.size.x / 2, here.y - box.size.y / 2);
	}
};

struct KLightWidget : ModuleLightWidget {
	void drawLayer(const DrawArgs& args, int layer) override {
		if(layer == 1) {
			if (this->color.a > 0.0) {
				nvgBeginPath(args.vg);
				nvgRect(args.vg, 0, 0, this->box.size.x, this->box.size.y);
				nvgFillColor(args.vg, this->color);
				nvgFill(args.vg);
			}
		}
		Widget::drawLayer(args, layer);
	}
};

struct KGRLightWidget : KLightWidget {
	KGRLightWidget() {
		bgColor = SCHEME_BLACK;
		borderColor = SCHEME_BLACK_TRANSPARENT;
		addBaseColor(SCHEME_GREEN);
        addBaseColor(SCHEME_RED);
		box.size = Vec(14, 14);
	}
};

extern int maxPoly(Module *m, const int numIn, const int numOut);

#define HP_UNIT 5.08
#define WIDTH (HP*HP_UNIT)
#define X_SPLIT (WIDTH / 2.f / LANES)

#define HEIGHT 128.5
#define Y_MARGIN 0.05f
#define R_HEIGHT (HEIGHT*(1-2*Y_MARGIN))
#define Y_SPLIT (R_HEIGHT / 2.f / RUNGS)

//placement macro
#define loc(x,y) mm2px(Vec(X_SPLIT*(1+2*(x-1)), (HEIGHT*Y_MARGIN)+Y_SPLIT*(1+2*(y-1))))

extern void populate(ModuleWidget *m, int hp, int lanes, int rungs, const int ctl[],
							const char *lbl[], const int kind[]);

template<typename kind>
struct plist {
	std::atomic<plist<kind>*> next;
	kind* item;
	void insertAfter(plist<kind>* what);
	plist<kind>* removeAfter(kind* what);
	plist<kind>* removeFirstAfter();
	bool containedAfter(kind* what);
};