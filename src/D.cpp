#include "plugin.hpp"


struct D : Module {
	enum ParamIds {
		DB,
		CVDB,
		NUM_PARAMS
	};
	enum InputIds {
		IN,
		ICVDB,
		NUM_INPUTS
	};
	enum OutputIds {
		ERR,
		OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	D() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(DB, -24.f, 24.f, 0.f, "Exponential Gain", " dB");
		configParam(CVDB, -6.f, 6.f, 0.f, "Modulation Level", " Center dB (rel 6)");
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

struct DWidget : ModuleWidget {
	DWidget(D* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/D.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(loc(1, 1), module, D::DB));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 2), module, D::CVDB));

		addInput(createInputCentered<PJ301MPort>(loc(1, 3), module, D::IN));
		addInput(createInputCentered<PJ301MPort>(loc(1, 4), module, D::ICVDB));

		addOutput(createOutputCentered<PJ301MPort>(loc(1, 5), module, D::ERR));
		addOutput(createOutputCentered<PJ301MPort>(loc(1, 6), module, D::OUT));
	}
};


Model* modelD = createModel<D, DWidget>("D");