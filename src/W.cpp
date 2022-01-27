#include "plugin.hpp"


struct W : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		IN,
		I1, I2, I3, I4,
		I5, I6, I7, I8,
		I9, I10, I11,
		NUM_INPUTS
	};
	enum OutputIds {
		PLUS,
		MINUS,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	const char *instring[NUM_INPUTS] = {
		"Frequency CV",
		"Semitone 1",
		"Semitone 2",
		"Semitone 3",
		"Semitone 4",
		"Semitone 5",
		"Semitone 6",
		"Semitone 7",
		"Semitone 8",
		"Semitone 9",
		"Semitone 10",
		"Semitone 11",
	};

	const char *outstring[NUM_OUTPUTS] = {
		"Semitones added",
		"Semitones subtracted",
	};

	const char *lightstring[NUM_LIGHTS] = {

	};

	void iol(bool lights) {
		for(int i = 0; i < NUM_INPUTS; i++) configInput(i, instring[i]);
		for(int i = 0; i < NUM_OUTPUTS; i++) configOutput(i, outstring[i]);
		if(!lights) return;
		for(int i = 0; i < NUM_LIGHTS; i++) configLight(i, lightstring[i]);
	}

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

	W() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		iol(false);
	}

	int order[11] = {
		I1, I2, I3, I4,
		I5, I6, I7, I8,
		I9, I10, I11
	};

	const float semi = 1.f / 12.f;

	void process(const ProcessArgs& args) override {
		int maxPort = maxPoly();

		// PARAMETERS (AND IMPLICIT INS)
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {
			float in = inputs[IN].getPolyVoltage(p);
			float add = 0.f;
#pragma GCC ivdep
			for(int i = 0; i < 10; i++) {
				float actual = semi * (i + 1);
				add += inputs[order[i]].getPolyVoltage(p) * 0.1f * actual;//1V effective
			}
			// OUTS
			outputs[PLUS].setVoltage(in + add, p);
			outputs[MINUS].setVoltage(in - add, p);
		}
	}
};

//geometry edit
#define HP 4
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

struct WWidget : ModuleWidget {
	WWidget(W* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/W.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(loc(1, 1), module, W::IN));
		addInput(createInputCentered<PJ301MPort>(loc(2, 1), module, W::I1));
		addInput(createInputCentered<PJ301MPort>(loc(1, 2), module, W::I2));
		addInput(createInputCentered<PJ301MPort>(loc(2, 2), module, W::I3));
		addInput(createInputCentered<PJ301MPort>(loc(1, 3), module, W::I4));
		addInput(createInputCentered<PJ301MPort>(loc(2, 3), module, W::I5));
		addInput(createInputCentered<PJ301MPort>(loc(1, 4), module, W::I6));
		addInput(createInputCentered<PJ301MPort>(loc(2, 4), module, W::I7));
		addInput(createInputCentered<PJ301MPort>(loc(1, 5), module, W::I8));
		addInput(createInputCentered<PJ301MPort>(loc(2, 5), module, W::I9));
		addInput(createInputCentered<PJ301MPort>(loc(1, 6), module, W::I10));
		addInput(createInputCentered<PJ301MPort>(loc(2, 6), module, W::I11));

		addOutput(createOutputCentered<PJ301MPort>(loc(1, 7), module, W::PLUS));
		addOutput(createOutputCentered<PJ301MPort>(loc(2, 7), module, W::MINUS));
	}
};


Model* modelW = createModel<W, WWidget>("W");
