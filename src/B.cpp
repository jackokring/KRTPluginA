#include "plugin.hpp"


struct B : Module {
	enum ParamIds {
		ENUMS(BUTTON, 6 * 3),
		MODE,
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(IN, 6),
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(OUT, 3),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(SELECT, 6 * 3 * 3), //RGB
		ENUMS(MODES, 3), //RGB
		NUM_LIGHTS
	};

	B() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs& args) override {
	}
};

//geometry edit
#define HP 9
#define LANES 4
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

struct BWidget : ModuleWidget {
	BWidget(B* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/B.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		for(int i = 0; i < 3; i++) {
			addOutput(createOutputCentered<PJ301MPort>(loc(i + 2, 7), module, B::OUT + i));
		}
		for(int i = 0; i < 6; i++) {
			addInput(createInputCentered<PJ301MPort>(loc(1, i + 1), module, B::IN + i));
		}
		for(int i = 0; i < 6; i++) {
			for(int j = 0; j < 3; j++) {
				int idx = i + 6 * j;
				addParam(createParamCentered<LEDBezel>(loc(j + 2, i + 1), module, B::BUTTON + idx));
				addChild(createLightCentered<LEDBezelLight<RedGreenBlueLight>>(loc(j + 2, i + 1), module, B::SELECT + 3 * idx));
			}
		}

		addParam(createParamCentered<LEDBezel>(loc(4, 7), module, B::MODE));
		addChild(createLightCentered<LEDBezelLight<RedGreenBlueLight>>(loc(4, 7), module, B::MODES));
	}
};


Model* modelB = createModel<B, BWidget>("B");