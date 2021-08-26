#include "plugin.hpp"


struct B : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	B() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs& args) override {
	}
};


struct BWidget : ModuleWidget {
	BWidget(B* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/B.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	}
};


Model* modelB = createModel<B, BWidget>("B");