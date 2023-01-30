#include "plugin.hpp"

struct I : Module {
	enum ParamIds {
		ENUMS(DIV, 3),//divider
		ENUMS(PHA, 3),//phase of division
		NUM_PARAMS
	};
	enum InputIds {
		CLK,
		RST,
		NUM_INPUTS
	};
	enum OutputIds {
		DWN,//downbeat clock
		SANS,//rest of the clocks
		ENUMS(OUT, 3),//divided outs
		XOR,//of outs
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	const char *instring[NUM_INPUTS] = {
		"Clock",
		"Reset",
	};

	const char *outstring[NUM_OUTPUTS] = {
		"Down beat",
		"Beats without down beat",
		"Divided 1",
		"Divided 2",
		"Divided 3",
		"Exclusive or of all 3 divided outputs",
	};

	const char *lightstring[NUM_LIGHTS] = {

	};

	void iol(bool lights) {
		for(int i = 0; i < NUM_INPUTS; i++) configInput(i, instring[i]);
		for(int i = 0; i < NUM_OUTPUTS; i++) configOutput(i, outstring[i]);
		if(!lights) return;
		for(int i = 0; i < NUM_LIGHTS; i++) configLight(i, lightstring[i]);
	}

	int maxPolySpecial() {
		int poly = inputs[CLK].getChannels();
		for(int o = 0; o < NUM_OUTPUTS; o++) {
			outputs[o].setChannels(poly);
		}
		outputs[DWN].setChannels(1);
		outputs[SANS].setChannels(1);
		return poly;
	}

	const int gcd = 360 * 7 * 11 * 13;//for phase

	bool outSym[PORT_MAX_CHANNELS][3];

	dsp::SchmittTrigger sClk[PORT_MAX_CHANNELS];
	dsp::SchmittTrigger sRst;

	int master = 0;
	bool maskIt = false;
	int divs[3];
	int phase[3];

	I() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		for(int i = 0; i < 3; i++) {
			configParam(DIV + i, 1.f, 16.f, 1.f, "Divider");
			configParam(PHA + i, 0.f, 100.f, 0.f, "Phase", " %");
			for(int j = 0; j < PORT_MAX_CHANNELS; j++) {
				outSym[j][i] = false;
			}
		}
		iol(false);
	}

	void process(const ProcessArgs& args) override {
		//float fs = args.sampleRate;
		int maxPort = maxPolySpecial();
		for(int i = 0; i < 3; i++) {
			divs[i] = (int)params[DIV + i].getValue();
			phase[i] = (int)(params[PHA + i].getValue() * 0.01f * (divs[i] - 1.f));
		}

		// PARAMETERS (AND IMPLICIT INS)
		float rst = inputs[RST].getVoltageSum();//signal OR
		sRst.process(rescale(rst, 0.1f, 2.f, 0.f, 1.f));
		for(int p = maxPort - 1; p > 0; p--) {//Assume branch into unroll number ...
			float clk = inputs[CLK].getPolyVoltage(p);
			bool trigClk = sClk[p].process(rescale(clk, 0.1f, 2.f, 0.f, 1.f));
			if(trigClk) {
				for(int i = 0; i < 3; i++) {
					outSym[p][i] = outSym[p - 1][i];
				}
			}
		}
		//clock normalization
		float clk = inputs[CLK].getPolyVoltage(0);
		bool trigClk = sClk[0].process(rescale(clk, 0.1f, 2.f, 0.f, 1.f));
		if(sRst.isHigh()) {
			master = 0;
			maskIt = true;
		} else if(trigClk) {
			maskIt = false;
			master++;
			if(master > gcd) master = 0;//modulo
		}
		bool clkMsk = sClk[0].isHigh();
		outSym[0][0] = clkMsk && ((master % divs[0]) == phase[0]);
		outSym[0][1] = clkMsk && ((master % divs[1]) == phase[1]);
		outSym[0][2] = clkMsk && ((master % divs[2]) == phase[2]);
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {
			// OUTS
			bool x = false;
#pragma GCC ivdep
			for(int i = 0; i < 3; i++) {
				outputs[OUT + i].setVoltage(outSym[p][i] ? 10.f : 0.f, p);
				x ^= outSym[p][i];
			}
			outputs[XOR].setVoltage(x ? 10.f : 0.f, p);//gate
		}
		outputs[DWN].setVoltage(clkMsk && maskIt ? 10.f : 0.f);//gate
		outputs[SANS].setVoltage(clkMsk && !maskIt ? 10.f : 0.f);//gate
	}
};

//geometry edit
#define HP 5
#define LANES 2
#define RUNGS 7

struct IWidget : ModuleWidget {
	IWidget(I* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/I.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		for(int i = 0; i < 3; i++) {
			addParam(createParamCentered<RoundBlackSnapKnob>(loc(1, i + 1), module, I::DIV + i));
			addParam(createParamCentered<RoundBlackKnob>(loc(2, i + 1), module, I::PHA + i));
			addOutput(createOutputCentered<PJ301MPort>(loc(2, i + 4), module, I::OUT + i));
		}

		addInput(createInputCentered<PJ301MPort>(loc(1, 4), module, I::CLK));
		addInput(createInputCentered<PJ301MPort>(loc(1, 5), module, I::RST));
		addOutput(createOutputCentered<PJ301MPort>(loc(1, 6), module, I::DWN));
		addOutput(createOutputCentered<PJ301MPort>(loc(1, 7), module, I::SANS));
		addOutput(createOutputCentered<PJ301MPort>(loc(2, 7), module, I::XOR));
	}
};


Model* modelI = createModel<I, IWidget>("I");
