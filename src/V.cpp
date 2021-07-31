#include "plugin.hpp"


struct V : Module {
	enum ParamIds {
		HZ,
		ATK,
		DCY,
		NUM_PARAMS
	};
	enum InputIds {
		CVHZ,
		CVDB,
		CVS,
		IN1,
		T51,
		T71,
		IN2,
		T52,
		T72,
		IN3,
		T53,
		T73,
		NUM_INPUTS
	};
	enum OutputIds {
		CV1,
		OUT1,
		CV2,
		OUT2,
		CV3,
		OUT3,
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

	V() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(HZ, -4.f, 4.f, 0.f, "Frequency", " Oct");
		//perhaps exponetials ...
		configParam(ATK, 0.01f, 10.f, 1.f, "Attack Time Constant", " s");
		configParam(DCY, 0.01f, 10.f, 1.f, "Decay Time Constant", " s");
	}

	//obtain mapped control value
    float log(float val, float centre) {
        return powf(2.f, val) * centre;
    }

	void process(const ProcessArgs& args) override {
		// For inputs intended to be used solely for audio, sum the voltages of all channels
		// (e.g. with Port::getVoltageSum())
		// For inputs intended to be used for CV or hybrid audio/CV, use the first channel’s
		// voltage (e.g. with Port::getVoltage())
		// POLY: Port::getPolyVoltage(c)
		float fs = args.sampleRate;
		int maxPort = maxPoly();

		float hz = params[HZ].getValue();
		float atk = params[ATK].getValue();
		float dcy = params[DCY].getValue();

		// PARAMETERS (AND IMPLICIT INS)
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {
			float cvdb = inputs[CVDB].getPolyVoltage(p);
			float cvhz = inputs[CVHZ].getPolyVoltage(p);
			float cvs = inputs[CVS].getPolyVoltage(p);

			// OUTS
			outputs[OUT1].setVoltage(0.f, p);
			outputs[OUT2].setVoltage(0.f, p);
			outputs[OUT3].setVoltage(0.f, p);

			outputs[CV1].setVoltage(0.f, p);
			outputs[CV2].setVoltage(0.f, p);
			outputs[CV3].setVoltage(0.f, p);
		}
	}
};

//geometry edit
#define HP 9
#define LANES 3
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

struct VWidget : ModuleWidget {
	VWidget(V* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/V.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(loc(1, 1), module, V::HZ));
		addParam(createParamCentered<RoundBlackKnob>(loc(2, 1), module, V::ATK));
		addParam(createParamCentered<RoundBlackKnob>(loc(3, 1), module, V::DCY));	

		addInput(createInputCentered<PJ301MPort>(loc(1, 2), module, V::CVHZ));
		addInput(createInputCentered<PJ301MPort>(loc(2, 2), module, V::CVDB));
		addInput(createInputCentered<PJ301MPort>(loc(3, 2), module, V::CVS));

		addInput(createInputCentered<PJ301MPort>(loc(1, 3), module, V::IN1));
		addInput(createInputCentered<PJ301MPort>(loc(2, 3), module, V::IN2));
		addInput(createInputCentered<PJ301MPort>(loc(3, 3), module, V::IN3));
		
		addInput(createInputCentered<PJ301MPort>(loc(1, 4), module, V::T51));
		addInput(createInputCentered<PJ301MPort>(loc(2, 4), module, V::T52));
		addInput(createInputCentered<PJ301MPort>(loc(3, 4), module, V::T53));

		addInput(createInputCentered<PJ301MPort>(loc(1, 5), module, V::T71));
		addInput(createInputCentered<PJ301MPort>(loc(2, 5), module, V::T72));
		addInput(createInputCentered<PJ301MPort>(loc(3, 5), module, V::T73));

		addOutput(createOutputCentered<PJ301MPort>(loc(1, 6), module, V::CV1));
		addOutput(createOutputCentered<PJ301MPort>(loc(2, 6), module, V::CV2));
		addOutput(createOutputCentered<PJ301MPort>(loc(3, 6), module, V::CV3));

		addOutput(createOutputCentered<PJ301MPort>(loc(1, 7), module, V::OUT1));
		addOutput(createOutputCentered<PJ301MPort>(loc(2, 7), module, V::OUT2));
		addOutput(createOutputCentered<PJ301MPort>(loc(3, 7), module, V::OUT3));
	}
};


Model* modelV = createModel<V, VWidget>("V");