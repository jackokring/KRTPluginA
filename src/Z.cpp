#include "plugin.hpp"


struct Z : Module {
	enum ParamIds {
		FRQ,
		NUM_PARAMS
	};
	enum InputIds {
		IN,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	const char *instring[NUM_INPUTS] = {
		"Frequency CV",
	};

	const char *outstring[NUM_OUTPUTS] = {
		"Semitones added",
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

	Z() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(FRQ, -4.f, 4.f, 0.f, "Frequency", " Oct");
		iol(false);
	}

	void process(const ProcessArgs& args) override {
		int maxPort = maxPoly();

		// PARAMETERS (AND IMPLICIT INS)
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {

		}
	}
};

//geometry edit
#define HP 13
#define LANES 6
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
	LabelWidget *display;

	WWidget(Z* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Z.svg")));

		addChild(createWidget<KScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<KScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<KScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<KScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		const char *lbl[] = {
			"CV", "TEST", "TEST", "TEST", "TEST", "TEST",
			"TEST", "TEST", "TEST", "TEST", "TEST", "TEST",
			// shape and filter modulation
			"A1M", "A3M", "RESM", "A2M", "A4M", "A6M",
			// shape and filter
			"P", "Q", "P0", "Q0", "P1", "Q1",
			"HP", "FRQ", "RES", "A2", "A4", "A6",
			"GATE", "A", "R", "EMOD", "A1", "A3",
			"PA", "QA", "PB", "QB", "PC", "QC"
		};

		const int kind[] = {
			// -1 = sink, +1 = source
			-1, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0,
			-1, -1, -1, -1, -1, -1,			
			-1, -1, 0, 0, 0, 0,
			-1, 0, 0, 0, 0, 0,
			-1, 0, 0, 0, 0, 0,
			1, 1, 1, 1, 1, 1
		};

		for(int x = 1; x <= 6; x++) {
			for(int y = 1; y <= 7; y++) {
				if(x == 1 && y == 1) {
					addInput(createInputCentered<KPJ301MPort>(loc(x, y), module, Z::IN));
				} else {
					addParam(createParamCentered<KRoundBlackKnob>(loc(x, y), module, Z::FRQ));
				}
				const int idx = (x - 1) + 6 * (y - 1);
				display = new LabelWidget(lbl[idx], kind[idx]);
				display->fixCentre(loc(x, y + 0.5f), strlen(lbl[idx]));//chars
				addChild(display);
			}
		}
	}
};


Model* modelZ = createModel<Z, WWidget>("Z");
