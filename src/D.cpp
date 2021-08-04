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
		f1   = tanf(M_PI * fc / fs);
		f2   = 1 / (1 + f1);
	}

	float process1(float in, int p) {
		float out = (f1 * in + b[p]) * f2;
		b[p] = f1 * (in - out) + out;
		return out;//lpf default
	}

	D() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(DB, -24.f, 6.f, 0.f, "Exponential Gain", " dB");
		configParam(CVDB, -6.f, 6.f, 0.f, "Modulation Level", " Center dB (rel 6)");
		configParam(FRQ, -4.f, 4.f, 0.f, "Frequency", " Oct");
	}

	//obtain mapped control value
    float log(float val, float centre) {
        return powf(2.f, val) * centre;
    }

	float dBMid(float val) {
		return powf(2.f, val)-powf(2.f, -val);
	}

	float future2(float* input) {
		float co1[] = {
			2.0620112372482157e+4f,
			-1.832898878564529e+5f,
			+7.217039339343297e+5f,
			-1.6496089933227736e+6f,
			+2.4056797847951367e+6f,
			-2.3094525972362604e+6f,
			+1.443407876801168e+6f,
			-5.498696694805404e+5f,
			+1.0310056373750902e+5f
		};//0th differential in the future
		return sum(co1, input, idx + 1) / 2.2911237200293967e+3;
	}

	const int mod9[18] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8 };

	float pre[PORT_MAX_CHANNELS][9];// pre buffer
	int idx = 0;// buffer current

	float sum(float* c, float* input, int begin = 0, int cycle = 9) {
		float add = 0.f;
#pragma GCC ivdep		
		for(int co = 0; co < cycle; co ++) {//right is most recent
			int idx = mod9[begin + co];
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
		//float fs = args.sampleRate;
		int maxPort = maxPoly();
		float fs = args.sampleRate;

		//dBMid(params[G1].getValue()/6.f);
		float db = params[DB].getValue()/6.f;
		float cvdb = dBMid(params[CVDB].getValue()/6.f);
		float hz = log(params[FRQ].getValue(), dsp::FREQ_C4);

		// PARAMETERS (AND IMPLICIT INS)
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {
			float cv = inputs[ICVDB].getPolyVoltage(p) * 0.1f;
			cv *= cvdb;
			cv = log(db + cv, inputs[IN].getPolyVoltage(p));

			pre[p][idx] = cv;
			float out = future2(pre[p]);
			hz = clamp(hz, 0.f, fs * 0.5f);
			setFK1(hz, fs);
			out = process1(out, p);//add future for low pass
			// OUTS
			outputs[OUT].setVoltage(out, p);
		}
		idx = mod9[idx + 1];//buffer modulo
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