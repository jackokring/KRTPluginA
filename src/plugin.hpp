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

#define M_PI_F float(M_PI)
#define M_PI_POW_2 M_PI * M_PI
#define M_PI_POW_3 M_PI_POW_2 * M_PI
#define M_PI_POW_5 M_PI_POW_3 * M_PI_POW_2
#define M_PI_POW_7 M_PI_POW_5 * M_PI_POW_2
#define M_PI_POW_9 M_PI_POW_7 * M_PI_POW_2
#define M_PI_POW_11 M_PI_POW_9 * M_PI_POW_2

static inline float tanpif(float f) {
    // These coefficients don't need to be tweaked for the audio range.
    // Include f multiplication by PI for 0->1 frequency normal %fs
    const float a = 3.333314036e-01 * M_PI_POW_3;
    const float b = 1.333923995e-01 * M_PI_POW_5;
    const float c = 5.33740603e-02 * M_PI_POW_7;
    const float d = 2.900525e-03 * M_PI_POW_9;
    const float e = 9.5168091e-03 * M_PI_POW_11;
    float f2 = f * f;
    return f * (M_PI_F + f2 * (a + f2 * (b + f2 * (c + f2 * (d + f2 * e)))));
}

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