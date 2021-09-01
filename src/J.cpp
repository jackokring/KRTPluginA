#include "plugin.hpp"


struct J : Module {
	enum ParamIds {
		FRQ,
		ODR,
		BHA,
		WET,
		NUM_PARAMS
	};
	enum InputIds {
		IFRQ,
		IODR,
		IBHA,
		IWET,
		IN,
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

	J() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(FRQ, -4.f, 4.f, 0.f, "Frequency", " Oct");
		configParam(ODR, 1.f, 8.f, 4.f, "Order");
		configParam(BHA, -1.f, 1.f, 1.f, "Bypass, High, All");
		configParam(WET, 0.f, 100.f, 100.f, "Wet Mix", " %");
		//configParam(DRV, -6.f, 6.f, 0.f, "Drive", " dB");
		//configParam(INV, -1.f, 1.f, -1.f, "Invert");
	}

	/* 1P H(s) = 1 / (s + fb) */
    //ONE POLE FILTER
	float f1, f2, b[PORT_MAX_CHANNELS][8];

	void setFK1(float fc, float fs) {//fb feedback not k*s denominator
		//f1   = tanf(M_PI * fc / fs);
		f1	 = tanpif(fc / fs);
		f2   = 1 / (1 + f1);
	}

	float process1(float in, int p, int idx) {
		float out = (f1 * in + b[p][idx]) * f2;
		b[p][idx] = f1 * (in - out) + out;
		return out;//lpf default
	}

	void process(const ProcessArgs& args) override {
		float fs = args.sampleRate;
		int maxPort = maxPoly();

		float frq = params[FRQ].getValue();
		float odr = params[ODR].getValue();
		float bha = params[BHA].getValue();
		float wet = params[WET].getValue() * 0.01f;//%
		//float drv = params[DRV].getValue()/6.f;//dB
		//float inv = params[INV].getValue();

		// PARAMETERS (AND IMPLICIT INS)
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {
			/* float mag = 0.25f;
			float ispd = inputs[ISPD].getPolyVoltage(p) * mag + spd;
			float iskw = inputs[ISKW].getPolyVoltage(p) * mag + skw;
			float ifrq = log(inputs[IFRQ].getPolyVoltage(p) + frq, dsp::FREQ_C4);
			float ilah = inputs[ILAH].getPolyVoltage(p) * mag + lah;
			flo0 = powf(flo0, ilah);//invert again at LPF
			float flo1 = freqMul(ispd, iskw, 1);//second
			flo1 = powf(flo1, ilah);
			float damp0 = findK(ispd, iskw, 0);//first
			float damp1 = findK(ispd, iskw, 1);//second

			flo0 = clamp(ifrq * flo0, 0.f, fs * 0.5f);
			flo1 = clamp(ifrq * flo1, 0.f, fs * 0.5f);
			//calm max change
			
			float idrv = inputs[IDRV].getPolyVoltage(p) * mag + drv;
			float iinv = inputs[IINV].getPolyVoltage(p) * mag + inv;
			idrv = log(idrv, 5.f);//normal magnitude
			*/
			// IN
			float in = inputs[IN].getPolyVoltage(p);

			// OUT
			outputs[OUT].setVoltage(in, p);
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

struct JWidget : ModuleWidget {
	JWidget(J* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/J.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(loc(1, 1), module, J::FRQ));
		addParam(createParamCentered<RoundBlackSnapKnob>(loc(2, 1), module, J::ODR));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 2), module, J::BHA));
		addParam(createParamCentered<RoundBlackKnob>(loc(2, 2), module, J::WET));
		//addParam(createParamCentered<RoundBlackKnob>(loc(1, 3), module, F::INV));
		//addParam(createParamCentered<RoundBlackKnob>(loc(2, 3), module, F::DRV));

		addInput(createInputCentered<PJ301MPort>(loc(1, 4), module, J::IFRQ));
		addInput(createInputCentered<PJ301MPort>(loc(2, 4), module, J::IODR));
		addInput(createInputCentered<PJ301MPort>(loc(1, 5), module, J::IBHA));
		addInput(createInputCentered<PJ301MPort>(loc(2, 5), module, J::IWET));
		//addInput(createInputCentered<PJ301MPort>(loc(1, 6), module, F::IINV));
		//addInput(createInputCentered<PJ301MPort>(loc(2, 6), module, F::IDRV));

		addInput(createInputCentered<PJ301MPort>(loc(1, 7), module, J::IN));
		addOutput(createOutputCentered<PJ301MPort>(loc(2, 7), module, J::OUT));
	}
};


Model* modelJ = createModel<J, JWidget>("J");