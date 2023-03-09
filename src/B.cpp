#include "plugin.hpp"

struct B : Module {
	enum ParamIds {
		ENUMS(BUTTON, 6 * 3),
		MODE,
		I_MODE,
		PATTERN,
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(IN, 6),
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(OUT, 3),
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(SELECT, 6 * 3 * 3), //RGB
		ENUMS(MODES, 3), //RGB
		NUM_LIGHTS
	};

	const char *instring[NUM_INPUTS] = {
		"Crossbar 1",
		"Crossbar 2",
		"Crossbar 3",
		"Crossbar 4",
		"Crossbar 5",
		"Crossbar 6",
	};

	const char *outstring[NUM_OUTPUTS] = {
		"Crossbar A",
		"Crossbar B",
		"Crossbar C",
	};

	const char *lightstring[NUM_LIGHTS] = {
		//no use ...
		//done by buttons, but ... RGB tripple tool tip faux pas.
	};

	void iol(bool lights) {
		for(int i = 0; i < NUM_INPUTS; i++) configInput(i, instring[i]);
		for(int i = 0; i < NUM_OUTPUTS; i++) configOutput(i, outstring[i]);
		if(!lights) return;
		for(int i = 0; i < NUM_LIGHTS; i++) configLight(i, lightstring[i]);
	}

#define MOD_PASS_G 0
#define MOD_SELECT_R 1
#define MOD_FUNC_B 2

	const char *names[3][6] = {
		{ "1A", "2A", "3A", "4A", "5A", "6A" },
		{ "1B", "2B", "3B", "4B", "5B", "6B" },
		{ "1C", "2C", "3C", "4C", "5C", "6C" }
	};

	static const int patches = 6 * 3;
	static const int outs = 3;
	static const int ins = 6;
	bool func[patches][outs][ins];
	bool use[patches][outs][ins];
	static const int sized = patches * outs * ins;
	char saves[sized];

	json_t* dataToJson() override {
		json_t *rootJ = json_object();
		for(int f = 0; f < patches; f++) {
			for(int i = 0; i < ins; i++) {
				for(int j = 0; j < outs; j++) {
					int idx = i + ins * (j + outs * f);
					saves[idx] = use[f][j][i] ? 'T' : 'F';
				}
			}
		}
		json_object_set_new(rootJ, "save", json_stringn(saves, sized));
		for(int f = 0; f < patches; f++) {
			for(int i = 0; i < ins; i++) {
				for(int j = 0; j < outs; j++) {
					int idx = i + ins * (j + outs * f);
					saves[idx] = func[f][j][i] ? 'T' : 'F';
				}
			}
		}
		json_object_set_new(rootJ, "blue", json_stringn(saves, sized));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* textJ = json_object_get(rootJ, "save");
  		if (textJ) {
			const char *str = json_string_value(textJ);
			if(str) {
				for(int f = 0; f < patches; f++) {
					for(int i = 0; i < ins; i++) {
						for(int j = 0; j < outs; j++) {
							int idx = i + ins * (j + outs * f);
							use[f][j][i] = (str[idx] == 'T') ? true : false;
						}
					}
				}
			}
		}
		textJ = json_object_get(rootJ, "blue");
  		if (textJ) {
			const char *str = json_string_value(textJ);
			if(str) {
				for(int f = 0; f < patches; f++) {
					for(int i = 0; i < ins; i++) {
						for(int j = 0; j < outs; j++) {
							int idx = i + ins * (j + outs * f);
							func[f][j][i] = (str[idx] == 'T') ? true : false;
						}
					}
				}
			}
		}
	}

	//obtain mapped control value
    float log(float val) {
        return powf(2.f, val);
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
		return in - out;//hpf default
	}

	B() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		for(int i = 0; i < 6; i++) {
			for(int j = 0; j < 3; j++) {
				configButton(BUTTON + i + 6 * j, names[j][i]);
			}
		}
		configButton(MODE, "Memory (R)/Pass (G)/Function (B)");
		configSwitch(I_MODE, 0.f, 2.f, 0.f);//internal mode
		configSwitch(PATTERN, 0.f, 18.f, 0.f);//default pattern
		iol(false);
		for(int f = 0; f < patches; f++) {
			for(int i = 0; i < ins; i++) {
				for(int j = 0; j < outs; j++) {
					func[f][j][i] = false;//function set
					use[f][j][i] = false;
				}
			}
		}
		for(int i = 0; i < PORT_MAX_CHANNELS * 3; i++) {
			b[i] = 0;
		}
	}

	dsp::SchmittTrigger modeTrig;
	dsp::SchmittTrigger selTrig[outs][ins];

	void RGBLed(int base, int id, bool r, bool g, bool b) {
		lights[base + 3 * id].setBrightness(r ? 1.f : 0.f);
		lights[base + 3 * id + 1].setBrightness(g ? 1.f : 0.f);
		lights[base + 3 * id + 2].setBrightness(b ? 1.f : 0.f);
	}

	void process(const ProcessArgs& args) override {
		float fs = args.sampleRate;
		setFK1(10.f, fs);//dc stop
		int maxPort = maxPoly(this, NUM_INPUTS, NUM_OUTPUTS);
		int pattern = (int)params[PATTERN].getValue();
		int mode = (int)params[I_MODE].getValue();
		if(modeTrig.process(params[MODE].getValue())) {
			mode++;
			if(mode > 2 || mode < 0) mode = 0;//loop
		}
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {
#pragma GCC ivdep
			for(int i = 0; i < 3; i++) {//outputs
				float out = 0.f;
				bool fn[6];
#pragma GCC ivdep
				for(int j = 0; j < 6; j++) {//over inputs
					int idx = j + 6 * i;
					if(selTrig[i][j].process(params[BUTTON + idx].getValue())) {
						if(mode == MOD_PASS_G) {
							use[pattern][i][j] ^= true;//invert
						}
						if(mode == MOD_SELECT_R) {
							pattern = idx;
							params[PATTERN].setValue(pattern);//save
						}
						if(mode == MOD_FUNC_B) {
							func[pattern][i][j] ^= true;//invert
						}
					}
					fn[j] = func[pattern][i][j];
					if((j == 0) && fn[j]) {
						out = 10.f;//mul function
					}
					if(use[pattern][i][j]) {
						float in = inputs[IN + j].getPolyVoltage(p);
						//process
						if(fn[0]) {//special as set up as first
							out *= in * 0.2f;//div 5
						} else {
							out += in;
						}
					}
					RGBLed(SELECT, idx, idx == pattern, use[pattern][i][j],
						fn[j]);//blue selectors too
				}
				//blue processing per output here <---
				if(fn[1] && abs(out) >= 5.f) {
					//sing process
					out = 25.f / out;
				}
				if(fn[2]) out = -out;//phase invert
				if(fn[3]) {
					out = log(out);//map CV?
				}
				if(fn[4]) {
					out = cbrtf(out * 25.f);//cube root clip
				}
				if(fn[5]) {//dc block
					out = process1(out, p + i * PORT_MAX_CHANNELS);//individual fix
				}
				//output out
				outputs[OUT + i].setVoltage(out, p);
			}
		}
		params[I_MODE].setValue(mode);//on changed
		RGBLed(MODES, 0, mode == MOD_SELECT_R, mode == MOD_PASS_G, mode == MOD_FUNC_B);
	}
};

//geometry edit
#define HP 9
#define LANES 4
#define RUNGS 7

struct BWidget : ModuleWidget {
	BWidget(B* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/B.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		for(int i = 0; i < 3; i++) {
			addOutput(createOutputCentered<PJ301MPort>(loc(i + 2, 7), module, B::OUT + i));
		}
		for(int i = 0; i < 6; i++) {
			addInput(createInputCentered<PJ301MPort>(loc(1, i + 1), module, B::IN + i));
		}
		for(int i = 0; i < 6; i++) {
			for(int j = 0; j < 3; j++) {
				int idx = i + 6 * j;
				addParam(createParamCentered<LEDBezel>(loc(j + 2, i + 1), module, B::BUTTON + idx));
				addChild(createLightCentered<LEDBezelLight<RedGreenBlueLight>>(loc(j + 2, i + 1), module, B::SELECT + 3 * idx));
			}
		}

		addParam(createParamCentered<LEDBezel>(loc(1, 7), module, B::MODE));
		addChild(createLightCentered<LEDBezelLight<RedGreenBlueLight>>(loc(1, 7), module, B::MODES));
	}
};


Model* modelB = createModel<B, BWidget>("B");
