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