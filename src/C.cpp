#include "plugin.hpp"


struct C : Module {
	enum ParamIds {
		G1, F1, G2, F2, G3, F3,
		NUM_PARAMS
	};
	enum InputIds {
		IN1,
		IN2,
		IN3,
		LINK,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT1,
		OUT2,
		OUT3,
		MIX,
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

	//obtain mapped control value
    float log(float val, float centre) {
        return powf(2.f, val) * centre;
    }

	float dBMid(float val) {
		return powf(2.f, val)-powf(2.f, -val);
	}

	/* 1P H(s) = 1 / (s + fb) */
    //ONE POLE FILTER
	float f1, f2, b[PORT_MAX_CHANNELS][3];

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

	C() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(G1, -6.f, 6.f, 0.f, "Gain", " Center dB (rel 6)");
		configParam(G2, -6.f, 6.f, 0.f, "Gain", " Center dB (rel 6)");
		configParam(G3, -6.f, 6.f, 0.f, "Gain", " Center dB (rel 6)");
		configParam(F1, -6.f, 6.f, 0.f, "LPF", " Oct");
		configParam(F2, -6.f, 6.f, 0.f, "LPF", " Oct");
		configParam(F3, -6.f, 6.f, 0.f, "LPF", " Oct");
		for(int i = 0; i < PORT_MAX_CHANNELS; i++) {
			for(int j = 0; j < 3; j++) {
				b[i][j] = 0;
			}
		}
	}

	void process(const ProcessArgs& args) override {
		float fs = args.sampleRate;

		float g1 = dBMid(params[G1].getValue()/6.f);
		float g2 = dBMid(params[G2].getValue()/6.f);
		float g3 = dBMid(params[G3].getValue()/6.f);

		float f1 = log(params[F1].getValue(), dsp::FREQ_C4);
		float f2 = log(params[F2].getValue(), dsp::FREQ_C4);
		float f3 = log(params[F3].getValue(), dsp::FREQ_C4);
		
		int maxPort = maxPoly();
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {
			float in1 = inputs[IN1].getPolyVoltage(p);
			float in2 = inputs[IN2].getPolyVoltage(p);
			float in3 = inputs[IN3].getPolyVoltage(p);

			float link = inputs[LINK].getPolyVoltage(p);

			setFK1(f1, fs);
			float out1 = g1 * process1(in1, p, 0);
			setFK1(f2, fs);
			float out2 = g2 * process1(in2, p, 1);
			setFK1(f3, fs);
			float out3 = g3 * process1(in3, p, 2);

			// OUTS
			outputs[OUT1].setVoltage(out1, p);
			outputs[OUT2].setVoltage(out2, p);
			outputs[OUT3].setVoltage(out3, p);
			float mix = 0.f;
			if(!outputs[OUT1].isConnected()) mix += out1;
			if(!outputs[OUT2].isConnected()) mix += out2;
			if(!outputs[OUT3].isConnected()) mix += out3;
			//SD normed mix
			//mix /= sqrtf(3.f);
			mix += link;
			outputs[MIX].setVoltage(mix, p);
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

struct CWidget : ModuleWidget {
	CWidget(C* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/C.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(loc(1, 1), module, C::G1));
		addParam(createParamCentered<RoundBlackKnob>(loc(2, 1), module, C::F1));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 2), module, C::G2));
		addParam(createParamCentered<RoundBlackKnob>(loc(2, 2), module, C::F2));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 3), module, C::G3));
		addParam(createParamCentered<RoundBlackKnob>(loc(2, 3), module, C::F3));

		addInput(createInputCentered<PJ301MPort>(loc(1, 4), module, C::IN1));
		addOutput(createOutputCentered<PJ301MPort>(loc(2, 4), module, C::OUT1));
		addInput(createInputCentered<PJ301MPort>(loc(1, 5), module, C::IN2));
		addOutput(createOutputCentered<PJ301MPort>(loc(2, 5), module, C::OUT2));
		addInput(createInputCentered<PJ301MPort>(loc(1, 6), module, C::IN3));
		addOutput(createOutputCentered<PJ301MPort>(loc(2, 6), module, C::OUT3));

		addInput(createInputCentered<PJ301MPort>(loc(1, 7), module, C::LINK));
		addOutput(createOutputCentered<PJ301MPort>(loc(2, 7), module, C::MIX));
	}
};


Model* modelC = createModel<C, CWidget>("C");