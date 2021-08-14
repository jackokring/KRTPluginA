#include "plugin.hpp"


struct X : Module {
	enum ParamIds {
		FOLD,
		KIND,
		MASH,
		WET,
		NUM_PARAMS
	};
	enum InputIds {
		IN,
		M_FOLD,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	X() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(FOLD, 0.f, 100.f, 50.f, "Fold Depth", " %");
		configParam(KIND, -1.f, 1.f, 0.f, "Fold Kind");
		configParam(MASH, 23.f, 1.f, 23.f, "Mash Bit Depth", " bits");
		configParam(WET, 0.f, 100.f, 50.f, "Wet Mix", " %");
	}

	void process(const ProcessArgs& args) override {
	}
};

//geometry edit
#define HP 3
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

struct XWidget : ModuleWidget {
	XWidget(X* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/X.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(loc(1, 1), module, X::FOLD));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 2), module, X::KIND));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 3), module, X::MASH));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 4), module, X::WET));

		addInput(createInputCentered<PJ301MPort>(loc(1, 5), module, X::IN));
		addInput(createInputCentered<PJ301MPort>(loc(1, 6), module, X::M_FOLD));

		addOutput(createOutputCentered<PJ301MPort>(loc(1, 7), module, X::OUT));
	}
};


Model* modelX = createModel<X, XWidget>("X");