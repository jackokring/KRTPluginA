#include "plugin.hpp"


struct X : Module {
	enum ParamIds {
		FOLD,
		KIND,
		MASH,
		WET,
		NUM_PARAMS
	};
	enum InputIds {
		IN,
		M_FOLD,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	int maxPoly() {
		int poly = 1;
		for(int i = 0; i < NUM_INPUTS; i++) {
			int chan = inputs[i].getChannels();
			if(chan > poly) poly = chan;
		}
		for(int o = 0; o < NUM_OUTPUTS; o++) {
			outputs[o].setChannels(poly);
		}
		return poly;
	}

	//{ 0.46338f, 0.947669f },//L4/2
	//{ 1.09948f, 0.430787f },//L4/2

	float f, t, u, k, tf, bl[PORT_MAX_CHANNELS], bb[PORT_MAX_CHANNELS];

    /* 2P
           Ghigh * s^2 + Gband * s + Glow
    H(s) = ------------------------------
                 s^2 + k * s + 1
     */
    //TWO POLE FILTER
	void setFK2(float fc, float kd, float fs) {
		//f   = tanf(M_PI * fc / fs);
		f	= tanpif(fc / fs);
		k   = kd;
		t   = 1 / (1 + k * f);
		u   = 1 / (1 + t * f * f);
		tf  = t * f;
	}

	float process2(float in, int p) {
		float low = (bl[p] + tf * (bb[p] + f * in)) * u;
		float band = (bb[p] + f * (in - low)) * t;
		float high = in - low - k * band;
		bb[p] = band + f * high;
		bl[p] = low  + f * band;
		return low;//lpf default
	}

	X() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(FOLD, 0.f, 100.f, 50.f, "Fold Depth", " %");
		configParam(KIND, -1.f, 1.f, 0.f, "Fold Kind");
		configParam(MASH, 23.f, 1.f, 23.f, "Mash Bit Depth", " bits");
		configParam(WET, 0.f, 100.f, 50.f, "Wet Mix", " %");
	}

	float future(float* input) {
		float co1[] = {-1.f, 4.f, -6.f, 4.f };//0th differential in the future
		return sum(co1, input, idx + 1);
	}

	float pre[PORT_MAX_CHANNELS][4];// pre buffer
	int idx = 0;// buffer current

	float sum(float* c, float* input, int begin = 0, int cycle = 4) {
		float add = 0.f;
#pragma GCC ivdep		
		for(int co = 0; co < cycle; co ++) {//right is most recent
			int idx = (begin + co) & 3;
			add += c[co] * input[idx];
		}
		return add;
	}

	void process(const ProcessArgs& args) override {
		float fs = args.sampleRate;
		int maxPort = maxPoly();

		// PARAMETERS (AND IMPLICIT INS)
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {

		}
		idx = (idx + 1) & 3;//buffer modulo
	}
};

//geometry edit
#define HP 3
#define LANES 1
#define RUNGS 7

//ok
#define HP_UNIT 5.08
#define WIDTH (HP*HP_UNIT)
#define X_SPLIT (WIDTH / 2.f / LANES)

#define HEIGHT 128.5
#define Y_MARGIN 0.05f
#define R_HEIGHT (HEIGHT*(1-2*Y_MARGIN))
#define Y_SPLIT (R_HEIGHT / 2.f / RUNGS)

//placement macro
#define loc(x,y) mm2px(Vec(X_SPLIT*(1+2*(x-1)), (HEIGHT*Y_MARGIN)+Y_SPLIT*(1+2*(y-1))))

struct XWidget : ModuleWidget {
	XWidget(X* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/X.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(loc(1, 1), module, X::FOLD));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 2), module, X::KIND));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 3), module, X::MASH));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 4), module, X::WET));

		addInput(createInputCentered<PJ301MPort>(loc(1, 5), module, X::IN));
		addInput(createInputCentered<PJ301MPort>(loc(1, 6), module, X::M_FOLD));

		addOutput(createOutputCentered<PJ301MPort>(loc(1, 7), module, X::OUT));
	}
};


Model* modelX = createModel<X, XWidget>("X");