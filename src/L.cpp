#include "plugin.hpp"


struct L : Module {
	enum ParamIds {
		PRE,
		FINE,
		NUM_PARAMS
	};
	enum InputIds {
		IN,
		TRIG,
		NUM_INPUTS
	};
	enum OutputIds {
		OPRE,
		OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	L() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PRE, 0.f, 16.f, 0.f, "Pre-trigger Samples", " 2^N Samples");
		configParam(FINE, 0.f, 1.f, 0.f, "Fine Pre-trigger", " 2^N+ Samples");
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

struct LWidget : ModuleWidget {
	LWidget(L* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/L.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(loc(1, 1), module, L::PRE));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 2), module, L::FINE));

		addInput(createInputCentered<PJ301MPort>(loc(1, 3), module, L::IN));
		addInput(createInputCentered<PJ301MPort>(loc(1, 4), module, L::TRIG));

		addOutput(createOutputCentered<PJ301MPort>(loc(1, 5), module, L::OPRE));
		addOutput(createOutputCentered<PJ301MPort>(loc(1, 6), module, L::OUT));
	}
};


Model* modelL = createModel<L, LWidget>("L");