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

	const char *instring[NUM_INPUTS] = {
		"Frequency CV",
		"Gain modulation",
		"Envelope time modulation",
		"Audio 1",
		"Up 4th",
		"Up 5th",
		"Audio 2",
		"Up 4th",
		"Up 5th",
		"Audio 3",
		"Up 4th",
		"Up 5th",
	};

	const char *outstring[NUM_OUTPUTS] = {
		"Frequency CV 1",
		"Audio 1",
		"Frequency CV 2",
		"Audio 2 normalized summing",
		"Frequency CV 3",
		"Audio 3",
	};

	const char *lightstring[NUM_LIGHTS] = {

	};

	void iol(bool lights) {
		for(int i = 0; i < NUM_INPUTS; i++) configInput(i, instring[i]);
		for(int i = 0; i < NUM_OUTPUTS; i++) configOutput(i, outstring[i]);
		if(!lights) return;
		for(int i = 0; i < NUM_LIGHTS; i++) configLight(i, lightstring[i]);
	}

	V() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(HZ, -4.f, 4.f, 0.f, "Frequency", " Oct");
		//perhaps exponetials ... inverse Oct -> Hz to seconds
		// 1/512th to 8 seconds (default 1/8th)
		configParam(ATK, -27.f, 9.f, -9.f, "Attack time", " dBs");
		configParam(DCY, -27.f, 9.f, -9.f, "Decay time", " dBs");
		iol(false);
		for(int i = 0; i < 3; i++) {
			for(int j = 0; j < PORT_MAX_CHANNELS; j++) {
				env[i][j] = 0.f;//silence
				envA[i][j] = false;//not triggered
			}
		}
	}

	//obtain mapped control value
    float log(float val, float centre) {
        return powf(2.f, val) * centre;
    }

	const int out[3] = {
		OUT1, OUT2, OUT3
	};

	const int cv[3] = {
		CV1, CV2, CV3
	};

	const int t5[3] = {
		T51, T52, T53
	};

	const int t7[3] = {
		T71, T72, T73
	};

	const int in[3] = {
		IN1, IN2, IN3
	};

	dsp::SchmittTrigger trig[3][PORT_MAX_CHANNELS];
	float env[3][PORT_MAX_CHANNELS];
	bool envA[3][PORT_MAX_CHANNELS];

	void process(const ProcessArgs& args) override {
		float fs = args.sampleRate;
		int maxPort = maxPoly(this, NUM_INPUTS, NUM_OUTPUTS);

		float hz = params[HZ].getValue();
		float atk = params[ATK].getValue()/3.f;
		float dcy = params[DCY].getValue()/3.f;

		// PARAMETERS (AND IMPLICIT INS)
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {
			float cvdb = log(inputs[CVDB].getPolyVoltage(p) * 0.1f, 1.f);
			float cvhz = inputs[CVHZ].getPolyVoltage(p) + hz;
			float cvs = inputs[CVS].getPolyVoltage(p) * 0.4f;//10v = quad/quarter
			float a = 1.f / log(atk + cvs, fs);
			float d = 1.f / log(dcy + cvs, fs);

			// OUTS
			float normal = 0.f;//normal total
#pragma GCC ivdep
			for(int i = 0; i < 3; i++) {
				float tr5 = inputs[t5[i]].getPolyVoltage(p);
				float tr7 = inputs[t7[i]].getPolyVoltage(p);
				float inOsc = inputs[in[i]].getPolyVoltage(p);
				bool trigger = trig[i][p].process(rescale(tr5 + tr7, 0.1f, 2.f, 0.f, 1.f));
				//TODO envelope on trigger
				float expEnv;
				if(trigger) {
					envA[i][p] = true;//attack
					env[i][p] = 1.f - env[i][p];
				}
				if(envA[i][p]) {
					env[i][p] -= a * env[i][p];
					expEnv = (1.f - env[i][p]);
					if(expEnv > 0.95f) {//almost
						envA[i][p] = false;//decay
						env[i][p] = expEnv;
					}
				} else {
					env[i][p] -= d * env[i][p];
					expEnv = env[i][p];
				}
				float outNorm = cvdb * inOsc * expEnv;//individual out
				if(!outputs[out[i]].isConnected()) normal += outNorm;//normalized
				outputs[out[i]].setVoltage(outNorm, p);
				float cvNorm = cvhz + (tr5 * 0.5f + tr7 * 0.7f) / 10.f;
				outputs[cv[i]].setVoltage(cvNorm, p);
			}
			outputs[OUT2].setVoltage(normal, p);//add in normalized
		}
	}
};

//geometry edit
#define HP 9
#define LANES 3
#define RUNGS 7

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
