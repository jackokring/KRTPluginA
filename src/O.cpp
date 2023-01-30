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

	const char *instring[NUM_INPUTS] = {
		"Frequency CV",
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

	O() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(FRQ, -4.f, 4.f, 0.f, "Frequency", " Oct");
		configParam(FBK, 0.f, 100.f, 50.f, "Feedback", " %");
		for(int i = 1; i < 3; i++) {
			configParam(FRQ + i, -2.f, 2.f, 0.f, "Relative frequency", " Oct");
			configParam(FBK + i, 0.f, 100.f, 50.f, "Feedback", " %");
		}
		iol(false);
		for(int i = 0; i < PORT_MAX_CHANNELS; i++) {
			for(int j = 0; j < 3; j++) {
				wave[i][j] = 0;
			}
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
		int maxPort = maxPoly(this, NUM_INPUTS, NUM_OUTPUTS);

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
				float freq = log(xtra + cv + frq[0]) * dsp::FREQ_C4;
				float step = freq * 2.f / fs;
				wave[p][i] += step;
				wave[p][i] = modulo(wave[p][i], 2.f);
				float wf = wave[p][i] + feed[p];
				float tmp3 = modulo(wf, 2.f);
				float tmp = modulo(wf, 1.f);
				float tmp2 = tmp * (tmp - 1.f) * (tmp3 - 1.f);
				out += 10.f * (1.0f - fbk[i]) * tmp2;
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
