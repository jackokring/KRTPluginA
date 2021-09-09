#include "plugin.hpp"


struct G : Module {
	enum ParamIds {
		ATK,
		DCY,
		THR,
		RTO,
		CUT,
		Q,
		MIX,
		ENV,
		NUM_PARAMS
	};
	enum InputIds {
		FRQ,
		SCH,
		IN,
		NUM_INPUTS
	};
	enum OutputIds {
		OFRQ,
		OENV,
		OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	G() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs& args) override {
	}
};


struct GWidget : ModuleWidget {
	GWidget(G* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/G.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	}
};


Model* modelG = createModel<G, GWidget>("G");