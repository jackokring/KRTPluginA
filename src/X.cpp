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

	const char *instring[NUM_INPUTS] = {
		"Audio",
		"Shape order modulation",
	};

	const char *outstring[NUM_OUTPUTS] = {
		"Audio",
	};

	const char *lightstring[NUM_LIGHTS] = {

	};

	void iol(bool lights) {
		for(int i = 0; i < NUM_INPUTS; i++) configInput(i, instring[i]);
		for(int i = 0; i < NUM_OUTPUTS; i++) configOutput(i, outstring[i]);
		if(!lights) return;
		for(int i = 0; i < NUM_LIGHTS; i++) configLight(i, lightstring[i]);
	}

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

	//cheby 3 and 5
	//4x^3-3x
	//16x^5-20x^3+5x

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
		configParam(FOLD, 0.f, 100.f, 50.f, "Fold depth", " %");
		configParam(KIND, -1.f, 1.f, 0.f, "Fold kind");
		configParam(MASH, 23.f, -4.f, 23.f, "Mash bit depth", " bits");
		configParam(WET, 0.f, 100.f, 50.f, "Wet mix", " %");
		iol(false);
		for(int i = 0; i < PORT_MAX_CHANNELS; i++) {
			bl[i] = bb[i] = 0;
		}
	}

	float modulo(float x, float m) {
		float div = x / m;
		long d = (long) div;
		float rem = x - d * m;
		return rem;
	}

	float shape(float in, float fmod, float kind) {
		float folded = in * 0.1f * fmod;//folded range
		float folded2 = folded * folded;//square
		float ch3 = (4.f * folded2 - 3.f) * folded;
		float ch5 = ((16.f * folded2 - 20.f) * folded2 + 5.f) * folded;
		float ch = (ch3 * (1.f - kind) + ch5 * (1.f + kind)) * 0.25f;//mixed
		return ch;
	}

	void process(const ProcessArgs& args) override {
		float fs = args.sampleRate;
		int maxPort = maxPoly();

		float fold = params[FOLD].getValue() * 0.01f;
		float kind = params[KIND].getValue();
		float mash = params[MASH].getValue();
		mash = powf(2.f, -mash);
		float wet = params[WET].getValue() * 0.01f;

		// PARAMETERS (AND IMPLICIT INS)
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {
			float in = inputs[IN].getPolyVoltage(p);
			float mod = inputs[M_FOLD].getPolyVoltage(p) * 0.1f;
			mod += fold;
			setFK2(fs * 0.5f, sqrtf(2), fs * 4.f);
			float err = modulo(in, mash);
			in -= err;
			//implicit future
			float ret = -shape(process2(in, p), mod, kind);
			ret += 4.f * shape(process2(err, p), mod, kind);
			ret -= 6.f * shape(process2(-err, p), mod, kind);
			ret += 4.f * shape(process2(err, p), mod, kind);//four times oversampling
			float out = 80.f * ret;
			outputs[OUT].setVoltage(wet * out + (1.f - wet) * in, p);
		}
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
