#include "plugin.hpp"

struct L : Module {
	enum ParamIds {
		PRE,
		FINE,
		NUM_PARAMS
	};
	enum InputIds {
		IN,
		TRIG,
		NUM_INPUTS
	};
	enum OutputIds {
		OPRE,
		OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	const char *instring[NUM_INPUTS] = {
		"Audio",
		"Trigger sync",
	};

	const char *outstring[NUM_OUTPUTS] = {
		"Pre-trigger sync",
		"Audio",
	};

	const char *lightstring[NUM_LIGHTS] = {
		//no use ...
	};

	void iol(bool lights) {
		for(int i = 0; i < NUM_INPUTS; i++) configInput(i, instring[i]);
		for(int i = 0; i < NUM_OUTPUTS; i++) configOutput(i, outstring[i]);
		if(!lights) return;
		for(int i = 0; i < NUM_LIGHTS; i++) configLight(i, lightstring[i]);
	}

	L() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PRE, 0.f, 16.f, 0.f, "Pre-trigger samples", " 2^N Samples");
		configParam(FINE, 0.f, 1.f, 0.f, "Fine pre-trigger", " 2^N+ Samples");
		iol(false);
		for(int p = 0; p < PORT_MAX_CHANNELS; p++) {
			lastHead[p] = head[p] = tail[p] = 0.f;
			buff.resize(MAX_BUFFER);
			len[p] = 4096.f;//an initial test
		}
	}

	const float rate = 44100.f;
	const float chans = 16.f;
	const float time = 16.f;

	//buffer length
	const float MAX_BUFFER = rate * chans * time;
	const float initPos = 0.f;
	float head[PORT_MAX_CHANNELS];
	float tail[PORT_MAX_CHANNELS];
	float maxLen;
	float buffStart[PORT_MAX_CHANNELS];

	//state control
	float lastHead[PORT_MAX_CHANNELS];
	float len[PORT_MAX_CHANNELS];

	//obtain mapped control value
    float log(float val) {
        return powf(2.f, val);
    }

	dsp::SchmittTrigger st[PORT_MAX_CHANNELS];
	std::vector<float> buff;

	float modulo(float x, float m) {
		float div = x / m;
		long d = (long) div;
		float rem = x - d * m;
		return rem;
	}

	void putBuffer(float in, int chan) {
		float where = head[chan];
		//modulo fine here!!
		where = modulo(where, maxLen);//modulo
		head[chan] = modulo(head[chan] + 1.f, maxLen);
		where += buffStart[chan];
		long w = (long) where;//get an integer index
		buff[w] = in;
	}

	float getBuffer(int chan) {
		float where = tail[chan];
		where = modulo(where, maxLen);//modulo
		tail[chan] = modulo(tail[chan] + 1.f, maxLen);
		where += buffStart[chan];
		long w = (long) where;//get an integer index
		float out = buff[w];
		return out;
	}

	void process(const ProcessArgs& args) override {
		// For inputs intended to be used solely for audio, sum the voltages of all channels
		// (e.g. with Port::getVoltageSum())
		// For inputs intended to be used for CV or hybrid audio/CV, use the first channel’s
		// voltage (e.g. with Port::getVoltage())
		// POLY: Port::getPolyVoltage(c)
		//float fs = args.sampleRate;
		int maxPort = maxPoly(this, NUM_INPUTS, NUM_OUTPUTS);
		maxLen = MAX_BUFFER / maxPort;//share buffer
		long max = (long) maxLen;
		maxLen = (float) max;//round down for sample guard

		float pres = params[PRE].getValue();
		pres += params[FINE].getValue();
		pres = log(pres);//samples

		// PARAMETERS (AND IMPLICIT INS)
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {
			buffStart[p] = maxLen * p;//fit
			float in = inputs[IN].getPolyVoltage(p);
			float trig = inputs[TRIG].getPolyVoltage(p);
			bool trigger = st[p].process(rescale(trig, 0.1f, 2.f, 0.f, 1.f));

			putBuffer(in, p);
			if(trigger) {
				len[p] = modulo(head[p] - lastHead[p] + maxLen, maxLen);//get written length since trigger
				lastHead[p] = head[p];//maintain length measure
			}

			float t = head[p] + maxLen;//a suitable positive
			t -= len[p];//tail point without pre trigger
			t += pres;//and pre trigger removal of delay
			tail[p] = t;//suitable delay

			bool preb = (head[p] - lastHead[p]) > (len[p] - pres);

			float out = getBuffer(p);

			// OUTS
			outputs[OPRE].setVoltage(preb ? 10.f : 0.f, p);//trigger out sort of
			outputs[OUT].setVoltage(out, p);
		}
	}
};

//geometry edit
#define HP 3
#define LANES 1
#define RUNGS 6

struct LWidget : ModuleWidget {
	LWidget(L* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/L.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(loc(1, 1), module, L::PRE));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 2), module, L::FINE));

		addInput(createInputCentered<PJ301MPort>(loc(1, 3), module, L::IN));
		addInput(createInputCentered<PJ301MPort>(loc(1, 4), module, L::TRIG));

		addOutput(createOutputCentered<PJ301MPort>(loc(1, 5), module, L::OPRE));
		addOutput(createOutputCentered<PJ301MPort>(loc(1, 6), module, L::OUT));
	}
};


Model* modelL = createModel<L, LWidget>("L");
