#include "plugin.hpp"


struct I : Module {
	enum ParamIds {
		ENUMS(DIV, 3),//divider
		ENUMS(PHA, 3),//phase of division
		NUM_PARAMS
	};
	enum InputIds {
		CLK,
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

	const int gcd = 360 * 7 * 11 * 13;//for phase

	I() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		for(int i = 0; i < 3; i++) {
			configParam(DIV + i, 1.f, 16.f, 1.f, "Divider");
			configParam(PHA + i, 0.f, 100.f, 0.f, "Phase", " %");
		}
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

struct IWidget : ModuleWidget {
	IWidget(I* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/I.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		for(int i = 0; i < 3; i++) {
			addParam(createParamCentered<RoundBlackSnapKnob>(loc(1, i + 1), module, I::DIV + i));
			addParam(createParamCentered<RoundBlackKnob>(loc(2, i + 1), module, I::PHA + i));
			addOutput(createOutputCentered<PJ301MPort>(loc(2, i + 4), module, I::OUT + i));
		}

		addInput(createInputCentered<PJ301MPort>(loc(1, 4), module, I::CLK));
		addInput(createInputCentered<PJ301MPort>(loc(1, 5), module, I::RST));
		addOutput(createOutputCentered<PJ301MPort>(loc(1, 6), module, I::DWN));
		addOutput(createOutputCentered<PJ301MPort>(loc(1, 7), module, I::SANS));
		addOutput(createOutputCentered<PJ301MPort>(loc(2, 7), module, I::XOR));
	}
};


Model* modelI = createModel<I, IWidget>("I");