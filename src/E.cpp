#include "plugin.hpp"


struct E : Module {
	enum ParamIds {
		ATK,
		REL,
		MOD,
		NUM_PARAMS
	};
	enum InputIds {
		IN,
		TRIG,
		NUM_INPUTS
	};
	enum OutputIds {
		OMOD,
		OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	int maxPoly() {
		int poly = 1;
		for(int i = 0; i < NUM_INPUTS; i++) {
			int chan = inputs[i].getChannels();
			if(chan > poly) poly = chan;
		}
		for(int o = 0; o < NUM_OUTPUTS; o++) {
			outputs[o].setChannels(poly);
		}
		return poly;
	}

	E() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ATK, -27.f, 9.f, -9.f, "Attack Time", " dBs");
		configParam(REL, -27.f, 9.f, -9.f, "Release Time", " dBs");
		configParam(MOD, -6.f, 6.f, 0.f, "Modulation Level", " Center dB (rel 6)");
	}

	float dBMid(float val) {
		return powf(2.f, val)-powf(2.f, -val);
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

struct EWidget : ModuleWidget {
	EWidget(E* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/E.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(loc(1, 1), module, E::ATK));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 2), module, E::REL));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 3), module, E::MOD));

		addInput(createInputCentered<PJ301MPort>(loc(1, 4), module, E::IN));
		addInput(createInputCentered<PJ301MPort>(loc(1, 5), module, E::TRIG));

		addOutput(createOutputCentered<PJ301MPort>(loc(1, 6), module, E::OMOD));
		addOutput(createOutputCentered<PJ301MPort>(loc(1, 7), module, E::OUT));
	}
};


Model* modelE = createModel<E, EWidget>("E");