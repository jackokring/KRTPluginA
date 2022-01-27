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

	const char *instring[NUM_INPUTS] = {
		"Audio",
		"Trigger",
	};

	const char *outstring[NUM_OUTPUTS] = {
		"Modulation bi-polar",
		"Audio",
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

	E() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ATK, -27.f, 9.f, -9.f, "Attack time", " dBs");
		configParam(REL, -27.f, 9.f, -9.f, "Release time", " dBs");
		configParam(MOD, -6.f, 6.f, 0.f, "Modulation level", " Center dB (rel 6)");
		iol(false);
		for(int j = 0; j < PORT_MAX_CHANNELS; j++) {
			env[j] = 0.f;//silence
			envA[j] = false;//not triggered
		}
	}

	float dBMid(float val) {
		return powf(2.f, val)-powf(2.f, -val);
	}

	//obtain mapped control value
    float log(float val, float centre) {
        return powf(2.f, val) * centre;
    }

	dsp::SchmittTrigger trig[PORT_MAX_CHANNELS];
	float env[PORT_MAX_CHANNELS];
	bool envA[PORT_MAX_CHANNELS];

	void process(const ProcessArgs& args) override {
		float fs = args.sampleRate;
		int maxPort = maxPoly();

		float atk = params[ATK].getValue()/3.f;
		float rel = params[REL].getValue()/3.f;
		float mod = dBMid(params[REL].getValue()/6.f);

		// PARAMETERS (AND IMPLICIT INS)
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {
			float a = 1.f / log(atk, fs);
			float r = 1.f / log(rel, fs);

			// OUTS
			float trigIn = inputs[TRIG].getPolyVoltage(p);
			float inOsc = inputs[IN].getPolyVoltage(p);
			bool trigger = trig[p].process(rescale(trigIn, 0.1f, 2.f, 0.f, 1.f));

			float expEnv;
			if(trigger) {
				envA[p] = true;//attack
				env[p] = 1.f;
			}
			if(envA[p]) {
				env[p] -= a * env[p];
				expEnv = (1.f - env[p]);
				if(expEnv > 0.95f) {//almost
					envA[p] = false;//decay
					env[p] = 1.f;
				}
			} else {
				env[p] -= r * env[p];
				expEnv = env[p];
			}

			outputs[OUT].setVoltage(inOsc * expEnv, p);
			outputs[OMOD].setVoltage(5.f * mod * expEnv, p);//add in normalized
		}
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
