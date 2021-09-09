#include "plugin.hpp"


struct I : Module {
	enum ParamIds {
		ENUMS(DIV, 3),//divider
		ENUMS(PHA, 3),//phase of division
		NUM_PARAMS
	};
	enum InputIds {
		CLK.
		RST,
		NUM_INPUTS
	};
	enum OutputIds {
		DWN,//downbeat clock
		SANS,//rest of the clocks
		ENUMS(OUT, 3),//divided outs
		XOR,//of outs
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	I() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs& args) override {
	}
};


struct IWidget : ModuleWidget {
	IWidget(I* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/I.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	}
};


Model* modelI = createModel<I, IWidget>("I");