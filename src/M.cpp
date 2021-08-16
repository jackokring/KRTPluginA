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

	float f, t, u, k, tf, bl[PORT_MAX_CHANNELS][2], bb[PORT_MAX_CHANNELS][2];

	float h, b, l, fm, km;
    /* 2P
           Ghigh * s^2 + Gband * s + Glow
    H(s) = ------------------------------
                 s^2 + k * s + 1
     */
    //TWO POLE FILTER
	void setFK2(float fc, float kd, float fs) {//1,1 for normal
		//f   = tanf(M_PI * fc / fs);
		f	= tanpif(fm * fc / fs);
		k   = kd * km;
		t   = 1 / (1 + k * f);
		u   = 1 / (1 + t * f * f);
		tf  = t * f;
	}

	void setHBL(float hs, float bs, float ls) {//second gains from unit filter
		//remove frequency correction for HBL equivelent G 
		h = hs / (fm * fm);
		b = bs / fm;
		l = ls;
	}

	void setPDQ(float p, float d, float q) {//1st to unit filter
		fm = sqrtf(p) / sqrtf(q);//--> `F` 
		km = 1.f / sqrtf(p * q);//so p cancels and q divides
		//unit form but correct quadratic
	}

	float process2(float in, int p, int i) {
		float low = (bl[p][i] + tf * (bb[p][i] + f * in)) * u;
		float band = (bb[p][i] + f * (in - low)) * t;
		float high = in - low - k * band;
		bb[p][i] = band + f * high;
		bl[p][i] = low  + f * band;
		return high * h + band * b + low * l;
	}	

	M() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(LOW, -3.f, 4.f, 0.f, "Low Frequency", " Oct (rel 50 Hz)");//800
		configParam(LGAIN, 0.f, 20.f, 20.f, "Low Gain", " dB");
		configParam(HIGH, -4.f, 0.f, 0.f, "High Frequency", " Oct (rel 21.22 kHz");
		configParam(HGAIN, -20.f, 0.f, -20.f, "High Gain", " dB");
		//21kHz break on high boost
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
		
		int maxPort = maxPoly();
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
			setPDQ(mul(low, hmid), sum(low, hmid), 1.f);//poles
			setHBL(mul(lmid, high), sum(lmid, high), 1.f);//zeros
			//omega 1 => f = 1 / 2*pi
			setFK2(1.f / PI_2, 1.f, fs);//unit filter moded
			float send = process2(in, p, 0);
			//reverse "record" curve
			setPDQ(mul(lmid, high), sum(lmid, high), 1.f);//poles
			setHBL(mul(low, hmid), sum(low, hmid), 1.f);//zeros
			setFK2(1.f / PI_2, 1.f, fs);//unit filter moded
			float out = process2(rtn, p, 1);

			// OUTS
			outputs[SEND].setVoltage(send * lgain, p);
			outputs[OUT].setVoltage(out * lgain, p);//DC gain
		}
	}
};

//geometry edit
#define HP 4
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