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

	/* 1P H(s) = 1 / (s + fb) */
    //ONE POLE FILTER
	float f1, f2, b[PORT_MAX_CHANNELS][8];

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
		configParam(LOW, -3.f, 4.f, 0.f, "Low Frequency", " Oct (rel 50 Hz)");//800
		configParam(LGAIN, -20.f, 20.f, 20.f, "Low Gain", " dB");
		configParam(HIGH, -4.f, 0.f, 0.f, "High Frequency", " Oct (rel 21.22 kHz");
		configParam(HGAIN, -20.f, 20.f, -20.f, "High Gain", " dB");
		//21kHz break on high boost
	}

	//obtain mapped control value
    float log(float val, float centre) {
        return powf(2.f, val) * centre;
    }

	float dB(float val) {
        return powf(10.f, val * 0.05f);//dB scaling V??
    }

	float slopeGainL(float hz, float gain) {
		//as 20dB per decade or 10 times
		if(gain >= 1.f)	return hz * gain;
		return hz / gain;
	}

	float slopeGainH(float hz, float gain) {
		//as 20dB per decade or 10 times
		if(gain >= 1.f)	return hz / gain;
		return hz * gain;
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
			float lmid = slopeGainL(low, lgain);//decade
			float hmid = slopeGainH(high, hgain);//decade

			//forward "play" curve
			setFK1(hmid, fs);//top cut
			float mid = process1(in, p, 0);
			setFK1(lmid, fs);//low cut
			mid = process1(mid, p, 1);//HPF

			setFK1(low, fs);
			mid += process1(in, p, 2) * lgain;//gained low
			setFK1(high, fs);
			mid += (in - process1(in, p, 3)) * hgain;//gained high
			float send = mid;

			//reverse "record" curve
			setFK1(hmid, fs);//top cut
			mid = process1(rtn, p, 4);
			setFK1(lmid, fs);//low cut
			mid = process1(mid, p, 5);//HPF

			setFK1(low, fs);
			mid += process1(rtn, p, 6) / lgain;//gained low
			setFK1(high, fs);
			mid += (rtn - process1(rtn, p, 7)) / hgain;//gained high

			// OUTS
			outputs[SEND].setVoltage(send, p);
			outputs[OUT].setVoltage(mid, p);//inverse HP?
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