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

	const char *instring[NUM_INPUTS] = {
		"Frequency CV",
		"Side-chain",
		"Audio",
	};

	const char *outstring[NUM_OUTPUTS] = {
		"Frequency CV",
		"Envelope CV",
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

	G() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ATK, -36.f, -1.f, -9.f, "Attack time", " dBs");
		configParam(DCY, -27.f, 1.f, -6.f, "Decay time", " dBs");
		configParam(THR, -24.f, 6.f, -6.f, "Threshold", " dB");
		configParam(RTO, 8.f, -4.f, 2.f, "Ratio", " 2^N:1");//256 soft-limiter
		//default 4:1 compressing
		configParam(CUT, -8.f, 2.f, 0.f, "Frequency", " Oct");
		//more for base
		configParam(Q, -6.f, 12.f, -6.f, "Resonance", " dBQ");
		configParam(MIX, 0.f, 100.f, 0.f, "Mix gain", " %");
		configParam(ENV, -2.f, 2.f, 0.f, "Envelope amount", " Oct/6dB");
		iol(false);
		for(int i = 0; i < PORT_MAX_CHANNELS; i++) {
			bl[i] = bb[i] = b[i] = last[i] = 0;
		}
	}

	//obtain mapped control value
    float log(float val, float centre) {
        return powf(2.f, val) * centre;
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
		return high;//hpf default
	}

	/* 1P H(s) = 1 / (s + fb) */
    //ONE POLE FILTER
	float f1, f2, b[PORT_MAX_CHANNELS], last[PORT_MAX_CHANNELS];

	void setFK1(float fc, float fs) {//fb feedback not k*s denominator
		//f1   = tanf(M_PI * fc / fs);
		f1	 = tanpif(fc / fs);
		f2   = 1 / (1 + f1);
	}

	float process1(float in, int p) {
		float out = (f1 * in + b[p]) * f2;
		b[p] = f1 * (in - out) + out;
		last[p] = out;
		return out;//lpf default
	}

	void process(const ProcessArgs& args) override {
		float fs = args.sampleRate;

		float atk = log(params[ATK].getValue()/3.f, 1.f);
		float dcy = log(params[DCY].getValue()/3.f, 1.f);
		int maxPort = maxPoly(this, NUM_INPUTS, NUM_OUTPUTS);
		float thr = log(params[THR].getValue()/6.f, 5.f);
		float rto = log(-params[RTO].getValue(), 1.f);
		float cut = params[CUT].getValue();
		float rez = params[Q].getValue()/6.f;
		float mix = params[MIX].getValue() * 0.01f;//%
		float env = params[ENV].getValue();//oct/6dB
		float makeUp = 10.f / ((10.f - thr) * rto + thr);

#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {
			float frq = inputs[FRQ].getPolyVoltage(p) + cut;
			float in = inputs[IN].getPolyVoltage(p);
			float sch = inputs[SCH].isConnected() ?
				inputs[SCH].getPolyVoltage(p) : in;

			sch = abs(sch);//absolute value
			float tau;
			//f = 1/2pi-tau
			if(sch > last[p]) {
				//attack
				tau = atk;
			} else {
				//decay
				tau = dcy;
			}
			float tasf = 0.5f / (M_PI * tau);//time as frequency
			tasf = clamp(tasf, 0.f, fs * 0.5f);
			setFK1(tasf, fs);
			sch = process1(sch, p);//envelope follow
			float use = 1.f;
			if(sch > thr) {
				//alter use ratio
				float over = sch - thr;
				over *= rto;//compress
				use = (over + thr) / sch;
			}
			in *= use;//apply ratio
			float cv = frq + env * -log2f(use);
			frq = log(cv, dsp::FREQ_C4);//get corner
			frq = clamp(frq, 0.f, fs * 0.5f);
			setFK2(frq, rez, fs);
			in = process2(in, p);//cut and rez
			//make up gain
			in = (1.f - mix) * in + mix * in * makeUp;
			// OUTS
			outputs[OFRQ].setVoltage(cv, p);
			outputs[OENV].setVoltage(sch, p);
			outputs[OUT].setVoltage(in, p);
		}
	}
};

//geometry edit
#define HP 5
#define LANES 2
#define RUNGS 7

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
