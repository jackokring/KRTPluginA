#include "plugin.hpp"


struct V : Module {
	enum ParamIds {
		HZ,
		ATK,
		DCY,
		NUM_PARAMS
	};
	enum InputIds {
		CVHZ,
		CVDB,
		CVS,
		IN1,
		T51,
		T71,
		IN2,
		T52,
		T72,
		IN3,
		T53,
		T73,
		NUM_INPUTS
	};
	enum OutputIds {
		CV1,
		OUT1,
		CV2,
		OUT2,
		CV3,
		OUT3,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	V() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(HZ, -4.f, 4.f, 0.f, "Frequency", " Oct");
		configParam(ATK, 0.f, 10.f, 1.f, "Attack", " s");
		configParam(DCY, 0.f, 10.f, 1.f, "Decay", " s");
	}

	void process(const ProcessArgs& args) override {
	}
};

//geometry edit
#define HP 9
#define LANES 3
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

struct VWidget : ModuleWidget {
	VWidget(V* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/V.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(loc(1, 1), module, V::HZ));
		addParam(createParamCentered<RoundBlackKnob>(loc(2, 1), module, V::ATK));
		addParam(createParamCentered<RoundBlackKnob>(loc(3, 1), module, V::DCY));	

		addInput(createInputCentered<PJ301MPort>(loc(1, 2), module, V::CVHZ));
		addInput(createInputCentered<PJ301MPort>(loc(2, 2), module, V::CVDB));
		addInput(createInputCentered<PJ301MPort>(loc(3, 2), module, V::CVS));

		addInput(createInputCentered<PJ301MPort>(loc(1, 3), module, V::IN1));
		addInput(createInputCentered<PJ301MPort>(loc(2, 3), module, V::IN2));
		addInput(createInputCentered<PJ301MPort>(loc(3, 3), module, V::IN3));
		
		addInput(createInputCentered<PJ301MPort>(loc(1, 4), module, V::T51));
		addInput(createInputCentered<PJ301MPort>(loc(2, 4), module, V::T52));
		addInput(createInputCentered<PJ301MPort>(loc(3, 4), module, V::T53));

		addInput(createInputCentered<PJ301MPort>(loc(1, 5), module, V::T71));
		addInput(createInputCentered<PJ301MPort>(loc(2, 5), module, V::T72));
		addInput(createInputCentered<PJ301MPort>(loc(3, 5), module, V::T73));

		addOutput(createOutputCentered<PJ301MPort>(loc(1, 6), module, V::CV1));
		addOutput(createOutputCentered<PJ301MPort>(loc(2, 6), module, V::CV2));
		addOutput(createOutputCentered<PJ301MPort>(loc(3, 6), module, V::CV3));

		addOutput(createOutputCentered<PJ301MPort>(loc(1, 7), module, V::OUT1));
		addOutput(createOutputCentered<PJ301MPort>(loc(2, 7), module, V::OUT2));
		addOutput(createOutputCentered<PJ301MPort>(loc(3, 7), module, V::OUT3));
	}
};


Model* modelV = createModel<V, VWidget>("V");