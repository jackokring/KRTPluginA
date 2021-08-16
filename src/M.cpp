#include "plugin.hpp"


struct M : Module {
	enum ParamIds {
		LOW,
		HIGH,
		CNTR,
		TOP,
		NUM_PARAMS
	};
	enum InputIds {
		ICTR,
		ITOP,
		IN,
		RTN,
		NUM_INPUTS
	};
	enum OutputIds {
		SEND,
		OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	M() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(LOW, -2.f, 2.f, 0.f, "Low Frequency", " Oct (rel 50 Hz)");
		configParam(HIGH, -2.f, 2.f, 0.f, "High Frequency", " Oct (rel 500 Hz");
		configParam(CNTR, -2.f, 2.f, 0.f, "Center Master Frequency", " Oct (rel 1 kHz)");
		configParam(TOP, -2.f, 2.f, 0.f, "Top Frequency", " Oct (rel 2.122 kHz");
		//21kHz break on high boost
	}

	void process(const ProcessArgs& args) override {
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

struct MWidget : ModuleWidget {
	MWidget(M* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/M.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(loc(1, 1), module, M::LOW));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 2), module, M::HIGH));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 3), module, M::CNTR));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 4), module, M::TOP));
	
		addInput(createInputCentered<PJ301MPort>(loc(0.75f, 5), module, M::ICTR));
		addInput(createInputCentered<PJ301MPort>(loc(1.25f, 5), module, M::ITOP));
		addInput(createInputCentered<PJ301MPort>(loc(0.75f, 6), module, M::IN));
		addInput(createInputCentered<PJ301MPort>(loc(1.25f, 6), module, M::RTN));

		addOutput(createOutputCentered<PJ301MPort>(loc(0.75f, 7), module, M::SEND));
		addOutput(createOutputCentered<PJ301MPort>(loc(1.25f, 7), module, M::OUT));
	}
};


Model* modelM = createModel<M, MWidget>("M");