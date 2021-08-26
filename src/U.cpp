#include "plugin.hpp"


struct U : Module {
	enum ParamIds {
		QUANTIZE,
		NOISE,
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(CLK, 3),
		ENUMS(IN, 3),
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(OUT, 3),
		ENUMS(QOUT, 3),
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	U() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
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

struct UWidget : ModuleWidget {
	UWidget(U* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/U.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackSnapKnob>(loc(1, 1), module, U::QUANTIZE));
		addParam(createParamCentered<RoundBlackKnob>(loc(2, 1), module, U::NOISE));

		for(int i = 0; i < 3; i++) {
			int y =  2 + 2 * i;
			addInput(createInputCentered<PJ301MPort>(loc(1, y), module, U::CLK + i));
			addInput(createInputCentered<PJ301MPort>(loc(2, y), module, U::IN + i));
			addOutput(createOutputCentered<PJ301MPort>(loc(1, y + 1), module, U::OUT + i));
			addOutput(createOutputCentered<PJ301MPort>(loc(2, y + 1), module, U::QOUT + i));
		}
	}
};


Model* modelU = createModel<U, UWidget>("U");