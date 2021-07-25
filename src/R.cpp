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
		AS,
		OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	R() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ON, -6.f, 6.f, 0.f, "On Level", " Center dB (rel 6)");
		configParam(AS, -6.f, 6.f, 0.f, "As Level", " Center dB (rel 6)");
	}

	void process(const ProcessArgs& args) override {
	}
};

//geometry edit
#define HP 3
#define LANES 1
#define RUNGS 6

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

		addOutput(createOutputCentered<PJ301MPort>(loc(1, 5), module, R::AS));
		addOutput(createOutputCentered<PJ301MPort>(loc(1, 6), module, R::OUT));
	}
};


Model* modelR = createModel<R, RWidget>("R");