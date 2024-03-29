#include "plugin.hpp"

struct A : Module {
	enum ParamIds {
		FRQ,
		MOD,
		REZ,
		OFF,
		MAM,
		RING,
		NUM_PARAMS
	};
	enum InputIds {
		IN1,
		CV,
		ILP1,
		IN2,
		IMOD,
		IHP1,
		NUM_INPUTS
	};
	enum OutputIds {
		LP1,
		LP12,
		OCV,
		HP1,
		XP12,
		AM1,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	const char *instring[NUM_INPUTS] = {
		"-18 dB",
		"Frequency CV",
		"-18 dB predictive",
		"-12 dB (no plate)",
		"Modulation",
		"-6 dB HP",
	};

	const char *outstring[NUM_OUTPUTS] = {
		"-6 dB LP",
		"-12/-18 dB",
		"Tracking CV",
		"-6 dB HP",
		"-12/-18 dB inverse",
		"Plate",
	};

	const char *lightstring[NUM_LIGHTS] = {

	};

	void iol(bool lights) {
		for(int i = 0; i < NUM_INPUTS; i++) configInput(i, instring[i]);
		for(int i = 0; i < NUM_OUTPUTS; i++) configOutput(i, outstring[i]);
		if(!lights) return;
		for(int i = 0; i < NUM_LIGHTS; i++) configLight(i, lightstring[i]);
	}

	A() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(FRQ, -4.f, 4.f, 0.f, "Frequency", " Oct");
		configParam(REZ, -6.f, 30.f, -6.f, "Resonance", " dBQ");
		configParam(MOD, -2.f, 2.f, 0.f, "Modulation of frequency");
		configParam(OFF, -4.f, 4.f, 0.f, "Offset", " Oct");
		configParam(RING, 0.f, 6.f, 0.f, "Ring modulation", " dB (rel 6)");
		configParam(MAM, -2.f, 2.f, 0.f, "Mod offset");
		iol(false);
		for(int i = 0; i < PORT_MAX_CHANNELS; i++) {
			bl[i] = bb[i] = b[i] = b2[i] = 0;
		}
	}

    //obtain mapped control value
    float log(float val, float centre) {
        return powf(2.f, val) * centre;
    }

	float dBMid(float val) {
		return powf(2.f, val)-powf(2.f, -val);
	}

    float qk(float val) {
        float v = log(val, 1.f);
        return 1 / v;//return k from Q
    }

	float f, t, u, k, tf, bl[PORT_MAX_CHANNELS], bb[PORT_MAX_CHANNELS];

    /* 2P
           Ghigh * s^2 + Gband * s + Glow
    H(s) = ------------------------------
                 s^2 + k * s + 1
     */
    //TWO POLE FILTER
	void setFK2(float fc, float q, float fs) {
		//f   = tanf(M_PI * fc / fs);
		f	= tanpif(fc / fs);
		k   = qk(q);
		t   = 1 / (1 + k * f);
		u   = 1 / (1 + t * f * f);
		tf  = t * f;
	}

	float process2(float in, int p) {
		float low = (bl[p] + tf * (bb[p] + f * in)) * u;
		float band = (bb[p] + f * (in - low)) * t;
		float high = in - low - k * band;
		bb[p] = band + f * high;
		bl[p] = low  + f * band;
		return low;//lpf default
	}

    /* 1P H(s) = 1 / (s + fb) */
    //ONE POLE FILTER
	float f1, f2, b[PORT_MAX_CHANNELS], b2[PORT_MAX_CHANNELS];

	void setFK1(float fc, float fs) {//fb feedback not k*s denominator
		//f1   = tanf(M_PI * fc / fs);
		f1	 = tanpif(fc / fs);
		f2   = 1 / (1 + f1);
	}

	float process1(float in, int p) {
		float out = (f1 * in + b[p]) * f2;
		b[p] = f1 * (in - out) + out;
		return out;//lpf default
	}

	float process3(float in, int p) {
		float out = (f1 * in + b2[p]) * f2;
		b2[p] = f1 * (in - out) + out;
		return in - out;//hpf default (WORKS FINE so DC must cancel)
	}

	float future(float* input) {
		float co1[] = {-1.f, 4.f, -6.f, 4.f };//0th differential in the future
		return sum(co1, input, idx + 1);
	}

	float pre[PORT_MAX_CHANNELS][4];// pre buffer
	int idx = 0;// buffer current

	float sum(float* c, float* input, int begin = 0, int cycle = 4) {
		float add = 0.f;
#pragma GCC ivdep
		for(int co = 0; co < cycle; co ++) {//right is most recent
			int idx = (begin + co) & 3;
			add += c[co] * input[idx];
		}
		return add;
	}

	void process(const ProcessArgs& args) override {
		// For inputs intended to be used solely for audio, sum the voltages of all channels
		// (e.g. with Port::getVoltageSum())
		// For inputs intended to be used for CV or hybrid audio/CV, use the first channel’s
		// voltage (e.g. with Port::getVoltage())
		// POLY: Port::getPolyVoltage(c)
		float fs = args.sampleRate;

		float res = params[REZ].getValue()/6.f;
		float plate = dBMid(params[RING].getValue()/6.f);
		int maxPort = maxPoly(this, NUM_INPUTS, NUM_OUTPUTS);
		float modf = params[MOD].getValue();
		float modo = params[MAM].getValue();

		// PARAMETERS (AND IMPLICIT INS)
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {
			float cv = inputs[CV].getPolyVoltage(p);

			float fo = log(params[OFF].getValue() + cv
				+ inputs[IMOD].getPolyVoltage(p) * modo,	//offset
				dsp::FREQ_C4);

			float ff = log(params[FRQ].getValue() + cv
				+ inputs[IMOD].getPolyVoltage(p) * modf,	//freq
				dsp::FREQ_C4);

			ff = clamp(ff, 0.f, fs * 0.5f);
			fo = clamp(fo, 0.f, fs * 0.5f);

			// INS (NOT IMPLICIT)
			float in18 = inputs[IN1].getPolyVoltage(p);
			float fut = 0;
			if(inputs[ILP1].isConnected()) {
				pre[p][idx] = inputs[ILP1].getPolyVoltage(p);
				fut = future(pre[p]);
			}
			float in12 = inputs[IN2].getPolyVoltage(p);
			float inbp = inputs[IHP1].getPolyVoltage(p);

			// Process filters 1, 2, 3 with common freq 1 and 3
			setFK1(fo, fs);
			float postL = process1(in18 + fut, p);//add future for low pass

			inbp += in18;
			float postH = process3(inbp, p);//fixed to allow inverse filter
			float qPlate = postL * postH * plate;//plate signal
			setFK2(ff, res, fs);
			float mainOut = process2(postL + in12, p) + qPlate;//6% with indirect postL

			// OUTS
			outputs[LP1].setVoltage(postL, p);
			outputs[OCV].setVoltage(ff, p);
			outputs[LP12].setVoltage(mainOut, p);

			outputs[HP1].setVoltage(postH, p);
			outputs[AM1].setVoltage(qPlate, p);
			outputs[XP12].setVoltage(in18 - mainOut, p);//inverse HP?
		}
		idx = (idx + 1) & 3;//buffer modulo
	}
};

//geometry edit
#define HP 7
#define LANES 3
#define RUNGS 6

struct AWidget : ModuleWidget {
	AWidget(A* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/A.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(loc(1, 1), module, A::FRQ));
		addParam(createParamCentered<RoundBlackKnob>(loc(3, 1), module, A::MOD));
		addParam(createParamCentered<RoundBlackKnob>(loc(2, 1), module, A::REZ));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 2), module, A::OFF));
		addParam(createParamCentered<RoundBlackKnob>(loc(3, 2), module, A::MAM));
		addParam(createParamCentered<RoundBlackKnob>(loc(2, 2), module, A::RING));

		addInput(createInputCentered<PJ301MPort>(loc(1, 3), module, A::IN1));
		addInput(createInputCentered<PJ301MPort>(loc(3, 3), module, A::CV));
		addInput(createInputCentered<PJ301MPort>(loc(2, 3), module, A::ILP1));
		addInput(createInputCentered<PJ301MPort>(loc(1, 4), module, A::IN2));
		addInput(createInputCentered<PJ301MPort>(loc(3, 4), module, A::IMOD));
		addInput(createInputCentered<PJ301MPort>(loc(2, 4), module, A::IHP1));

		addOutput(createOutputCentered<PJ301MPort>(loc(1, 5), module, A::LP1));
		addOutput(createOutputCentered<PJ301MPort>(loc(3, 5), module, A::LP12));
		addOutput(createOutputCentered<PJ301MPort>(loc(2, 5), module, A::OCV));
		addOutput(createOutputCentered<PJ301MPort>(loc(1, 6), module, A::HP1));
		addOutput(createOutputCentered<PJ301MPort>(loc(3, 6), module, A::XP12));
		addOutput(createOutputCentered<PJ301MPort>(loc(2, 6), module, A::AM1));
	}
};


Model* modelA = createModel<A, AWidget>("A");
