#include "plugin.hpp"


struct T : Module {
	enum ParamIds {
		NOTE,
		FINE,
		NUM_PARAMS
	};
	enum InputIds {
		IN,
		TRIG,
		NUM_INPUTS
	};
	enum OutputIds {
		HI,
		OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	T() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
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

struct TWidget : ModuleWidget {
	TWidget(T* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/T.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(loc(1, 1), module, T::NOTE));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 2), module, T::FINE));

		addInput(createInputCentered<PJ301MPort>(loc(1, 3), module, T::IN));
		addInput(createInputCentered<PJ301MPort>(loc(1, 4), module, T::TRIG));

		addOutput(createOutputCentered<PJ301MPort>(loc(1, 5), module, T::HI));
		addOutput(createOutputCentered<PJ301MPort>(loc(1, 6), module, T::OUT));
	}
};


Model* modelT = createModel<T, TWidget>("T");