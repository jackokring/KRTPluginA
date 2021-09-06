#include "plugin.hpp"


struct H : Module {
	enum ParamIds {
		ENUMS(HARM, 9),
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(IHARM, 9),
		FRQ,
		PM,
		NUM_INPUTS
	};
	enum OutputIds {
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

	const char *names[9] = {
		"16'", "5 1/3'", "8'",
		"4'", "2 2/3'", "2'",
		"1 3/5'", "1 1/3'", "1'"
	};

	H() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		for(int i = 0; i < 3; i++) {
			for(int j = 0; j < 3; j++) {
				int idx = j + 3 * i;
				configParam(HARM + idx, 0.f, 100.f, 50.f, names[idx], " %");
			}
		}
	}

	const float divider[9] = {//as multiplier
		1.f / 16.f, 3.f / 16.f, 1.f / 8.f,
		1.f / 4.f, 3.f / 8.f, 1.f / 2.f,
		5.f / 8.f, 3.f / 4.f, 1.f
	};

	const float multiplier[9] = {//as multiplier
		16.f, 16.f / 3.f, 8.f,
		4.f, 8.f / 3.f, 2.f,
		8.f / 5.f, 4.f / 3.f, 1.f
	};

	//obtain mapped control value
    float log(float val, float centre) {
        return powf(2.f, val) * centre;
    }

	float modulo(float x, float m) {
		float div = x / m;
		long d = (long) div;
		float rem = x - d * m;
		return rem;
	}

	float wave[PORT_MAX_CHANNELS];
	float para[9];

	void process(const ProcessArgs& args) override {
		float fs = args.sampleRate;
		int maxPort = maxPoly();
#pragma GCC ivdep
		for(int i = 0; i < 9; i++) {
			para[i] = params[HARM + i].getValue() * 0.01f;
		}
		// PARAMETERS (AND IMPLICIT INS)
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {
			float frq = log(inputs[FRQ].getPolyVoltage(p), dsp::FREQ_C4);
			float pm = inputs[PM].getPolyVoltage(p) * 0.1f;
			float out = 0.f;
			float step = frq * 2.f / fs;
			wave[p] += step;
			wave[p] = modulo(wave[p], 240.f * 4.f);
#pragma GCC ivdep
			for(int i = 0; i < 9; i++) {
				float x = modulo(wave[p] * divider[i] + multiplier[i] * pm + 64.f, 2.f);//phase offset plus wrap
				float amp = inputs[IHARM + i].getPolyVoltage(p) * 0.1f + para[i];
				if(x > 1.f) {
					x = 2.f - x;//triangle
				}
				x -= 0.5f;//centre
				out += x * amp;//energy density?
			}
			outputs[OUT].setVoltage(10.f / 3.f * out, p);
		}
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

struct HWidget : ModuleWidget {
	HWidget(H* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/H.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		for(int i = 0; i < 3; i++) {
			for(int j = 0; j < 3; j++) {
				int idx = j + 3 * i;
				addParam(createParamCentered<RoundBlackKnob>(loc(j + 1, i + 1), module, H::HARM + idx));
				addInput(createInputCentered<PJ301MPort>(loc(j + 1, i + 4), module, H::IHARM + idx));
			}
		}
		addInput(createInputCentered<PJ301MPort>(loc(1, 7), module, H::FRQ));
		addInput(createInputCentered<PJ301MPort>(loc(2, 7), module, H::PM));
		addOutput(createOutputCentered<PJ301MPort>(loc(3, 7), module, H::OUT));
	}
};


Model* modelH = createModel<H, HWidget>("H");