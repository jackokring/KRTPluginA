#include "plugin.hpp"

struct M : Module {
	enum ParamIds {
		LOW,
		LGAIN,
		HIGH,
		HGAIN,
		NUM_PARAMS
	};
	enum InputIds {
		ILOW,
		IHIGH,
		IN,
		RTN,
		NUM_INPUTS
	};
	enum OutputIds {
		SEND,
		OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	const char *instring[NUM_INPUTS] = {
		"Low frequency corner",
		"High frequency corner",
		"Audio",
		"FX return",
	};

	const char *outstring[NUM_OUTPUTS] = {
		"FX send",
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

    /* 1P H(s) = 1 / (s + fb) */
    //ONE POLE FILTER
	float f1, f2, b[PORT_MAX_CHANNELS][4];

	void setFK1(float fc, float fs) {//fb feedback not k*s denominator
		//f1   = tanf(M_PI * fc / fs);
		f1	 = tanpif(fc / fs);
		f2   = 1 / (1 + f1);
	}

	float process1(float in, int p, int i) {
		float out = (f1 * in + b[p][i]) * f2;
		b[p][i] = f1 * (in - out) + out;
		return out;//lpf default
	}

	M() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(LOW, -3.f, 4.f, 0.f, "Low frequency", " Oct (rel 50 Hz)");//800
		configParam(LGAIN, 0.f, 20.f, 20.f, "Low gain", " dB");
		configParam(HIGH, -4.f, 0.f, 0.f, "High frequency", " Oct (rel 21.22 kHz");
		configParam(HGAIN, -20.f, 0.f, -20.f, "High gain", " dB");
		iol(false);
		//21kHz break on high boost
		for(int i = 0; i < PORT_MAX_CHANNELS; i++) {
			for(int j = 0; j < 4; j++) {
				b[i][j] = 0;
			}
		}
	}

	//obtain mapped control value
    float log(float val, float centre) {
        return powf(2.f, val) * centre;
    }

	float dB(float val) {
        return powf(10.f, val * 0.05f);//dB scaling V??
    }

	const float PI_2 = M_PI * 2.f;

	float mul(float f1, float f2) {
		//time constants
		return 1.f / (PI_2 * PI_2 * f1 * f2);
	}

	float sum(float f1, float f2) {
		//time constants
		return 1.f / (PI_2 * f1) + 1.f / (PI_2 * f2);
	}

	void process(const ProcessArgs& args) override {
		float fs = args.sampleRate;

		float low = params[LOW].getValue();
		float lgain = dB(params[LGAIN].getValue());
		float high = params[HIGH].getValue();
		float hgain = dB(params[HGAIN].getValue());

		int maxPort = maxPoly(this, NUM_INPUTS, NUM_OUTPUTS);
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {
			float in = inputs[IN].getPolyVoltage(p);
			float rtn = inputs[RTN].getPolyVoltage(p);

			low = log(low + inputs[ILOW].getPolyVoltage(p), 50.f);//gain 10 = 1 decade
			low = clamp(low, 0.f, fs * 0.5f);
			high = log(high + inputs[IHIGH].getPolyVoltage(p), 21220.f);
			high = clamp(high, 0.f, fs * 0.5f);

			//extra constructed poles/zeros for actual
			float lmid = low * lgain;//decade
			float hmid = high * hgain;//decade

			//forward "play" curve
			float send = in * hgain;//minor end bit but tracking??
			setFK1(hmid, fs);
			send += (1.f - hgain) * process1(in, p, 0);
			setFK1(low, fs);
			send += (lgain - 1.f) * process1(in, p, 1);
			//reverse "record" curve
			float out = rtn / lgain;
			setFK1(lmid, fs);
			float hp = rtn - process1(rtn, p, 2);
			out += (1.f - 1.f / lgain) * hp;
			setFK1(high, fs);//oversampling makes worse
			//but it's the tracking of high and high pass
			//so close to Nyquist
			hp = rtn - process1(rtn, p, 3);
			out += (1.f / hgain - 1.f) * hp;

			// OUTS
			outputs[SEND].setVoltage(send, p);
			outputs[OUT].setVoltage(out, p);//DC gain
		}
	}
};

//geometry edit
#define HP 4
#define LANES 1
#define RUNGS 7

struct MWidget : ModuleWidget {
	MWidget(M* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/M.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(loc(1, 1), module, M::LOW));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 2), module, M::LGAIN));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 3), module, M::HIGH));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 4), module, M::HGAIN));

		addInput(createInputCentered<PJ301MPort>(loc(0.75f, 5), module, M::ILOW));
		addInput(createInputCentered<PJ301MPort>(loc(1.25f, 5), module, M::IHIGH));
		addInput(createInputCentered<PJ301MPort>(loc(0.75f, 6), module, M::IN));
		addInput(createInputCentered<PJ301MPort>(loc(1.25f, 6), module, M::RTN));

		addOutput(createOutputCentered<PJ301MPort>(loc(0.75f, 7), module, M::SEND));
		addOutput(createOutputCentered<PJ301MPort>(loc(1.25f, 7), module, M::OUT));
	}
};


Model* modelM = createModel<M, MWidget>("M");
