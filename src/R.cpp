#include "plugin.hpp"

struct R : Module {
	enum ParamIds {
		ON,
		AS,
		NUM_PARAMS
	};
	enum InputIds {
		IN,
		ION,
		NUM_INPUTS
	};
	enum OutputIds {
		OAS,
		OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	const char *instring[NUM_INPUTS] = {
		"Audio not gained",
		"Audio gained by on",
	};

	const char *outstring[NUM_OUTPUTS] = {
		"Audio gained by out",
		"Audio not gained",
	};

	const char *lightstring[NUM_LIGHTS] = {
		//no use ...
	};

	void iol(bool lights) {
		for(int i = 0; i < NUM_INPUTS; i++) configInput(i, instring[i]);
		for(int i = 0; i < NUM_OUTPUTS; i++) configOutput(i, outstring[i]);
		if(!lights) return;
		for(int i = 0; i < NUM_LIGHTS; i++) configLight(i, lightstring[i]);
	}

	R() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ON, -6.f, 6.f, 0.f, "On level", " Center dB (rel 6)");
		configParam(AS, -6.f, 6.f, 0.f, "As level", " Center dB (rel 6)");
		iol(false);
	}

	float dBMid(float val) {
		return powf(2.f, val)-powf(2.f, -val);
	}

	void process(const ProcessArgs& args) override {
		// For inputs intended to be used solely for audio, sum the voltages of all channels
		// (e.g. with Port::getVoltageSum())
		// For inputs intended to be used for CV or hybrid audio/CV, use the first channel’s
		// voltage (e.g. with Port::getVoltage())
		// POLY: Port::getPolyVoltage(c)
		//float fs = args.sampleRate;
		int maxPort = maxPoly(this, NUM_INPUTS, NUM_OUTPUTS);

		//dBMid(params[G1].getValue()/6.f);
		float on = dBMid(params[ON].getValue()/6.f);
		float as = dBMid(params[AS].getValue()/6.f);

		// PARAMETERS (AND IMPLICIT INS)
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {
			float out = inputs[IN].getPolyVoltage(p);
			out += inputs[ION].getPolyVoltage(p) * on;
			as *= out;
			// OUTS
			outputs[OAS].setVoltage(as, p);
			outputs[OUT].setVoltage(out, p);
		}
	}
};

//geometry edit
#define HP 3
#define LANES 1
#define RUNGS 6

struct RWidget : ModuleWidget {
	RWidget(R* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/R.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(loc(1, 1), module, R::ON));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 2), module, R::AS));

		addInput(createInputCentered<PJ301MPort>(loc(1, 3), module, R::IN));
		addInput(createInputCentered<PJ301MPort>(loc(1, 4), module, R::ION));

		addOutput(createOutputCentered<PJ301MPort>(loc(1, 5), module, R::OAS));
		addOutput(createOutputCentered<PJ301MPort>(loc(1, 6), module, R::OUT));
	}
};


Model* modelR = createModel<R, RWidget>("R");
