#include "plugin.hpp"


struct G : Module {
	enum ParamIds {
		ATK,
		DCY,
		THR,
		RTO,
		CUT,
		Q,
		MIX,
		ENV,
		NUM_PARAMS
	};
	enum InputIds {
		FRQ,
		SCH,
		IN,
		NUM_INPUTS
	};
	enum OutputIds {
		OFRQ,
		OENV,
		OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	G() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ATK, -27.f, -1.f, -9.f, "Attack Time", " dBs");
		configParam(DCY, -27.f, -1.f, -6.f, "Decay Time", " dBs");
		configParam(THR, -24.f, 6.f, -6.f, "Threshold", " dB");
		configParam(RTO, 16.f, -16.f, 4.f, "Ratio", ":1");
		configParam(FRQ, -8.f, 4.f, 0.f, "Frequency", " Oct");
		configParam(Q, -6.f, 12.f, -6.f, "Resonance", " dBQ");
		configParam(MIX, 0.f, 100.f, 0.f, "Mix Gain", " %");
		configParam(ENV, 0.f, 1.f, 0.f, "Envelope Amount", " Oct/dB");
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

struct GWidget : ModuleWidget {
	GWidget(G* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/G.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(loc(1, 1), module, G::ATK));
		addParam(createParamCentered<RoundBlackKnob>(loc(2, 1), module, G::DCY));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 2), module, G::THR));
		addParam(createParamCentered<RoundBlackKnob>(loc(2, 2), module, G::RTO));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 3), module, G::CUT));
		addParam(createParamCentered<RoundBlackKnob>(loc(2, 3), module, G::Q));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 4), module, G::MIX));
		addParam(createParamCentered<RoundBlackKnob>(loc(2, 4), module, G::ENV));

		addInput(createInputCentered<PJ301MPort>(loc(1, 5), module, G::FRQ));
		addInput(createInputCentered<PJ301MPort>(loc(1, 6), module, G::SCH));
		addInput(createInputCentered<PJ301MPort>(loc(1, 7), module, G::IN));

		addOutput(createOutputCentered<PJ301MPort>(loc(2, 5), module, G::OFRQ));
		addOutput(createOutputCentered<PJ301MPort>(loc(2, 6), module, G::OENV));
		addOutput(createOutputCentered<PJ301MPort>(loc(2, 7), module, G::OUT));
	}
};


Model* modelG = createModel<G, GWidget>("G");