#include "plugin.hpp"


struct M : Module {
	enum ParamIds {
		LOW,
		HIGH,
		CNTR,
		TOP,
		NUM_PARAMS
	};
	enum InputIds {
		ICTR,
		ITOP,
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
	float f1, f2, b[PORT_MAX_CHANNELS][7];

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
		configParam(LOW, -2.f, 2.f, 0.f, "Low Frequency", " Oct (rel 50 Hz)");
		configParam(HIGH, -2.f, 2.f, 0.f, "High Frequency", " Oct (rel 500 Hz");
		configParam(CNTR, -2.f, 2.f, 0.f, "Center Master Frequency", " Oct (rel 1 kHz)");
		configParam(TOP, -2.f, 2.f, 0.f, "Top Frequency", " Oct (rel 2.122 kHz");
		//21kHz break on high boost
	}

	//obtain mapped control value
    float log(float val, float centre) {
        return powf(2.f, val) * centre;
    }

	void process(const ProcessArgs& args) override {
		float fs = args.sampleRate;

		float cent = params[CNTR].getValue();//a relative measure
		float low = params[LOW].getValue() + cent;
		float high = params[HIGH].getValue() + cent;
		float top = params[TOP].getValue() + cent;
		float chop = 21000.f;//top boost chop corner
		
		int maxPort = maxPoly();
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {
			float in = inputs[IN].getPolyVoltage(p);
			float rtn = inputs[RTN].getPolyVoltage(p);

			low = log(low + inputs[ICTR].getPolyVoltage(p), 50.f);//gain 10 = 1 decade
			low = clamp(low, 0.f, fs * 0.5f);
			high = log(high + inputs[ICTR].getPolyVoltage(p), 500.f);
			high = clamp(high, 0.f, fs * 0.5f);
			top = log(top + inputs[ITOP].getPolyVoltage(p), 2122.f);
			top = clamp(top, 0.f, fs * 0.5f);

			float decade = high / low;
			float duck = chop / high;

			//forward "play" curve
			setFK1(top, fs);//top cut
			float mid = process1(in, p, 0);
			setFK1(high, fs);//low cut for add
			mid -= process1(mid, p, 1);//HPF
			setFK1(low, fs);
			mid += process1(in, p, 2) * decade;//gained low

			//reverse "record" curve
			setFK1(top, fs);//top boost => cut for add
			float mid2 = process1(rtn, p, 3);
			setFK1(high, fs);//low cut for add
			mid2 -= process1(mid2, p, 4);//HPF
			setFK1(low, fs);
			mid2 += process1(rtn, p, 5) / decade;//dropped low
			setFK1(chop, fs);//chopper high pass
			mid2 += (mid2 - process1(rtn, p, 6)) * duck;//high shelf add 

			// OUTS
			outputs[SEND].setVoltage(mid, p);
			outputs[OUT].setVoltage(mid2, p);//inverse HP?
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
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 2), module, M::HIGH));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 3), module, M::CNTR));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 4), module, M::TOP));
	
		addInput(createInputCentered<PJ301MPort>(loc(0.75f, 5), module, M::ICTR));
		addInput(createInputCentered<PJ301MPort>(loc(1.25f, 5), module, M::ITOP));
		addInput(createInputCentered<PJ301MPort>(loc(0.75f, 6), module, M::IN));
		addInput(createInputCentered<PJ301MPort>(loc(1.25f, 6), module, M::RTN));

		addOutput(createOutputCentered<PJ301MPort>(loc(0.75f, 7), module, M::SEND));
		addOutput(createOutputCentered<PJ301MPort>(loc(1.25f, 7), module, M::OUT));
	}
};


Model* modelM = createModel<M, MWidget>("M");