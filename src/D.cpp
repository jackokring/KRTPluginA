#include "plugin.hpp"


struct D : Module {
	enum ParamIds {
		DB,
		CVDB,
		FRQ,
		NUM_PARAMS
	};
	enum InputIds {
		IN,
		ICVDB,
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
		"Audio",
		"Gain modulation",
	};

	const char *outstring[NUM_OUTPUTS] = {
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

	/* 1P H(s) = 1 / (s + fb) */
    //ONE POLE FILTER
	float f1, f2, b[PORT_MAX_CHANNELS];

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

	D() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(DB, -24.f, 6.f, 0.f, "Exponential gain", " dB");
		configParam(CVDB, -6.f, 6.f, 0.f, "Modulation level", " Center dB (rel 6)");
		configParam(FRQ, -4.f, 4.f, 0.f, "Frequency", " Oct");
		iol(false);
		for(int i = 0; i < PORT_MAX_CHANNELS; i++) {
			b[i] = 0;
		}
	}

	//obtain mapped control value
    float log(float val, float centre) {
        return powf(2.f, val) * centre;
    }

	float dBMid(float val) {
		return powf(2.f, val)-powf(2.f, -val);
	}

	void process(const ProcessArgs& args) override {
		int maxPort = maxPoly();
		float fs = args.sampleRate;

		//dBMid(params[G1].getValue()/6.f);
		float db = params[DB].getValue()/6.f;
		float cvdb = dBMid(params[CVDB].getValue()/6.f);
		float hz = log(params[FRQ].getValue(), dsp::FREQ_C4);
		hz = clamp(hz, 0.f, fs * 2.f);
		setFK1(hz, fs * 4.f);

		// PARAMETERS (AND IMPLICIT INS)
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {
			float cv = inputs[ICVDB].getPolyVoltage(p) * 0.1f;
			cv *= cvdb;
			cv = log(db + cv, inputs[IN].getPolyVoltage(p));

			//f_ = (-120*f[i-11]+396*f[i-10]-440*f[i-9]+165*f[i-8])/(1*1.0*h**0)
			float out = -120.f * process1(4.f * cv, p);//add future for low pass
			out += 396.f * process1(0.f, p);
			out -= 440.f * process1(0.f, p);
			out += 165.f * process1(0.f, p);
			// OUTS
			outputs[OUT].setVoltage(out, p);
		}
	}
};

//geometry edit
#define HP 3
#define LANES 1
#define RUNGS 6

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

struct DWidget : ModuleWidget {
	DWidget(D* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/D.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(loc(1, 1), module, D::DB));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 2), module, D::CVDB));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 3), module, D::FRQ));

		addInput(createInputCentered<PJ301MPort>(loc(1, 4), module, D::IN));
		addInput(createInputCentered<PJ301MPort>(loc(1, 5), module, D::ICVDB));

		addOutput(createOutputCentered<PJ301MPort>(loc(1, 6), module, D::OUT));
	}
};


Model* modelD = createModel<D, DWidget>("D");
