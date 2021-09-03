#include "plugin.hpp"


struct K : Module {
	enum ParamIds {
		FRQ,
		LPF,
		RTO1,
		RTO2,
		MOD1,
		MOD2,
		NUM_PARAMS
	};
	enum InputIds {
		IFRQ,
		ILPF,
		IRTO1,
		IRTO2,
		IMOD1,
		IMOD2,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT,
		OMOD,
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

	K() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(FRQ, -4.f, 4.f, 0.f, "Frequency", " Oct");
		configParam(LPF, -4.f, 4.f, 0.f, "Filter", " Oct");
		configParam(RTO1, -12.f, 12.f, 0.f, "Low Ratio", " Semitones");
		configParam(RTO2, -12.f, 12.f, 0.f, "High Ratio", " Semitones");
		configParam(MOD1, 0.f, 100.f, 50.f, "Low Modulation", " %");
		configParam(MOD2, 0.f, 100.f, 50.f, "High Modulation", " %");
	}

	//obtain mapped control value
    float log(float val, float centre) {
        return powf(2.f, val) * centre;
    }

	/* 1P H(s) = 1 / (s + fb) */
    //ONE POLE FILTER
	float f1, f2, b[PORT_MAX_CHANNELS][8];

	void setFK1(float fc, float fs) {//fb feedback not k*s denominator
		//f1   = tanf(M_PI * fc / fs);
		f1	 = tanpif(fc / fs);
		f2   = 1 / (1 + f1);
	}

	float process1(float in, int p, int idx) {
		float out = (f1 * in + b[p][idx]) * f2;
		b[p][idx] = f1 * (in - out) + out;
		return out;//lpf default
	}

	float modulo(float x, float m) {
		float div = x / m;
		long d = (long) div;
		float rem = x - d * m;
		return rem;
	}

	void process(const ProcessArgs& args) override {
	}
};

//geometry edit
#define HP 5
#define LANES 2
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

struct KWidget : ModuleWidget {
	KWidget(K* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/K.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(loc(1, 1), module, K::FRQ));
		addParam(createParamCentered<RoundBlackKnob>(loc(2, 1), module, K::LPF));
		addParam(createParamCentered<RoundBlackSnapKnob>(loc(1, 2), module, K::RTO1));
		addParam(createParamCentered<RoundBlackSnapKnob>(loc(2, 2), module, K::RTO2));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 3), module, K::MOD1));
		addParam(createParamCentered<RoundBlackKnob>(loc(2, 3), module, K::MOD2));

		addInput(createInputCentered<PJ301MPort>(loc(1, 4), module, K::IFRQ));
		addInput(createInputCentered<PJ301MPort>(loc(2, 4), module, K::ILPF));
		addInput(createInputCentered<PJ301MPort>(loc(1, 5), module, K::IRTO1));
		addInput(createInputCentered<PJ301MPort>(loc(2, 5), module, K::IRTO2));
		addInput(createInputCentered<PJ301MPort>(loc(1, 6), module, K::IMOD1));
		addInput(createInputCentered<PJ301MPort>(loc(2, 6), module, K::IMOD2));

		addOutput(createOutputCentered<PJ301MPort>(loc(1, 7), module, K::OMOD));
		addOutput(createOutputCentered<PJ301MPort>(loc(2, 7), module, K::OUT));
	}
};


Model* modelK = createModel<K, KWidget>("K");