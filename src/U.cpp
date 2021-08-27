#include "plugin.hpp"
//#include <random> - links but then missing symbol!!!

struct U : Module {
	enum ParamIds {
		QUANTIZE,
		NOISE,
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(CLK, 3),
		ENUMS(IN, 3),
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(OUT, 3),
		ENUMS(QOUT, 3),
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

	/* 1P H(s) = 1 / (s + fb) */
    //ONE POLE FILTER
	float f1, f2, b[PORT_MAX_CHANNELS * 3];

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

	U() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(QUANTIZE, 1.f, 12.f, 1.f, "Interval", " Semitones");
		configParam(NOISE, -8.f, 8.f, 0.f, "Noise Filter", " Oct");
		for(int i = 0; i < 3; i++) {
			sampled[i] = 0.f;
		}
	}

	dsp::SchmittTrigger sh[3];
	float sampled[3];

	float moduloD(float x, float m) {
		float div = x / m;
		long d = (long) div;
		float rem = d * m;
		return rem;
	}

	//obtain mapped control value
    float log(float val, float centre) {
        return powf(2.f, val) * centre;
    }

	//static std::default_random_engine gen;

	float nextNextGaussian;
	bool haveNextNextGaussian = false;

	float nextDouble() {
		float r = rand();
		r /= RAND_MAX;//0 to 1
		return r;
	}

	float nextGaussian() {
		if (haveNextNextGaussian) {
			haveNextNextGaussian = false;
			return nextNextGaussian;
		} else {
			float v1, v2, s;
			do {
			v1 = 2 * nextDouble() - 1;   // between -1.0 and 1.0
			v2 = 2 * nextDouble() - 1;   // between -1.0 and 1.0
			s = v1 * v1 + v2 * v2;
			} while (s >= 1.f || s == 0.f);
			float multiplier = sqrtf(-2.f * logf(s) / s);
			nextNextGaussian = v2 * multiplier;
			haveNextNextGaussian = true;
			return v1 * multiplier;
		}
	}

	void process(const ProcessArgs& args) override {
		float fs = args.sampleRate;
		int maxPort = maxPoly();

		float q = params[QUANTIZE].getValue() / 12.f;//semitone volts
		float n = log(params[NOISE].getValue(), dsp::FREQ_C4);
		n = clamp(n, 0.f, fs * 0.5f);//filter limit
		//static std::normal_distribution<float> dist(0, 5);

#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {

#pragma GCC ivdep
			for(int i = 0; i < 3; i++) {
				float in = 0.f;
				if(inputs[IN + i].isConnected()) {
					in = inputs[IN + i].getPolyVoltage(p);
				} else {
					//in = dist(gen);//noise
					in = nextGaussian() * 5.f;//SD
					setFK1(n, fs);
					in = process1(in, p + i * PORT_MAX_CHANNELS);
				}
				float clk = inputs[CLK + i].getPolyVoltage(p);
				bool trig = sh[i].process(rescale(clk, 0.1f, 2.f, 0.f, 1.f));
				if(trig) sampled[i] = in;//sh
				outputs[OUT + i].setVoltage(sampled[i], p);//out
				float quant = moduloD(sampled[i] - (q * 0.5f), q);//off by
				outputs[QOUT + i].setVoltage(quant, p);
			}
		}
	}
};

//geometry edit
#define HP 5
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

struct UWidget : ModuleWidget {
	UWidget(U* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/U.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackSnapKnob>(loc(1, 1), module, U::QUANTIZE));
		addParam(createParamCentered<RoundBlackKnob>(loc(2, 1), module, U::NOISE));

		for(int i = 0; i < 3; i++) {
			int y =  2 + 2 * i;
			addInput(createInputCentered<PJ301MPort>(loc(1, y), module, U::CLK + i));
			addInput(createInputCentered<PJ301MPort>(loc(2, y), module, U::IN + i));
			addOutput(createOutputCentered<PJ301MPort>(loc(1, y + 1), module, U::OUT + i));
			addOutput(createOutputCentered<PJ301MPort>(loc(2, y + 1), module, U::QOUT + i));
		}
	}
};


Model* modelU = createModel<U, UWidget>("U");