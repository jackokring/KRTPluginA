#include "plugin.hpp"


struct O : Module {
	enum ParamIds {
		ENUMS(FRQ, 3),
		ENUMS(FBK, 3),
		NUM_PARAMS
	};
	enum InputIds {
		CV,
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

	O() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(FRQ, -4.f, 4.f, 0.f, "Frequency", " Oct");
		for(int i = 0; i < 3; i++) {
			if(i > 0) configParam(FRQ + i, -1.f, 1.f, 0.f, "Relative Frequency", " Oct");
			configParam(FBK + i, 0.f, 100.f, 50.f, "Feedback", " %");
		}
	}

	//obtain mapped control value
    float log(float val) {
        return powf(2.f, val);
    }

	float modulo(float x, float m) {
		float div = x / m;
		long d = (long) div;
		float rem = x - d * m;
		return rem;
	}

	float wave[PORT_MAX_CHANNELS][3];
	float feed[PORT_MAX_CHANNELS];

	void process(const ProcessArgs& args) override {
		float fs = args.sampleRate;
		int maxPort = maxPoly();

		float frq[3], fbk[3];
#pragma GCC ivdep
		for(int i = 0; i < 3; i++) {
			frq[i] = params[FRQ + i].getValue();
			fbk[i] = params[FBK + i].getValue() * 0.01f;
		}

		// PARAMETERS (AND IMPLICIT INS)
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {
			float cv = inputs[CV].getPolyVoltage(p);
			float out = 0.f;
			float loop = 0.f;
			for(int i = 0; i < 3; i++) {
				float xtra = i > 0 ? frq[i] : 0.f;
				float freq = log(xtra + cv + frq[1]) * dsp::FREQ_C4;
				float step = freq * 2.f / fs;
				wave[p][i] += step;
				wave[p][i] = modulo(wave[p][i], 2.f);
				float wf = wave[p][i] + feed[p];
				float tmp3 = modulo(wf, 2.f);
				float tmp = modulo(wf, 1.f);
				float tmp2 = tmp * (tmp - 1.f) * (tmp3 - 1.f);
				out += 7.f * tmp2;
				loop += fbk[i] * tmp2 * 0.3f;
			}
			feed[p] = loop;//feedback
			// OUTS
			outputs[OUT].setVoltage(out, p);
		}
	}
};

//geometry edit
#define HP 4
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

struct OWidget : ModuleWidget {
	OWidget(O* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/O.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		for(int j = 0; j < 3; j++) {
			float x = 1.f;
			float y = j * 2.f + 1.f;
			addParam(createParamCentered<RoundBlackKnob>(loc(x, y), module, O::FRQ + j));
			addParam(createParamCentered<RoundBlackKnob>(loc(x, y + 1), module, O::FBK + j));
		}

		addInput(createInputCentered<PJ301MPort>(loc(0.75f, 7), module, O::CV));
		addOutput(createOutputCentered<PJ301MPort>(loc(1.25f, 7), module, O::OUT));
	}
};


Model* modelO = createModel<O, OWidget>("O");