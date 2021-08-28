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

#define MOD_PASS_G 0
#define MOD_SELECT_R 1
#define MOD_FUNC_B 2

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

	const char *names[3][6] = {
		{ "1A/1", "2A/1", "3A/1", "4A/1", "5A/3", "6A/3" },
		{ "1B/1", "2B/1", "3B/1", "4B/1", "5B/3", "6B/3" },
		{ "1C/1", "2C/1", "3C/1", "4C/1", "5C/3", "6C/3" }
	};

	static const int patches = 6 * 3;
	static const int outs = 3;
	static const int ins = 6;
	char func[patches][outs][ins];
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
		json_object_set(rootJ, "save", json_stringn(saves, sized));
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
	}

	B() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		for(int i = 0; i < 6; i++) {
			for(int j = 0; j < 3; j++) {
				configParam(BUTTON + i + 6 * j, 0.f, 1.f, 0.f, names[j][i]);
			}
		}
		configParam(MODE, 0.f, 1.f, 0.f, "Memory/Pass/Function");
		configParam(I_MODE, 0.f, 2.f, 0.f);//internal mode
		configParam(PATTERN, 0.f, 18.f, 0.f);//default pattern
		for(int f = 0; f < patches; f++) {
			for(int i = 0; i < ins; i++) {
				for(int j = 0; j < outs; j++) {
					func[f][j][i] = 'A';//function set
					use[f][j][i] = false;
				}
			}
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
		int maxPort = maxPoly();
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

						}
					}
					if(use[pattern][i][j]) {
						float in = inputs[IN + j].getPolyVoltage(p);
						//process
						out += in;
					}
					RGBLed(SELECT, idx, idx == pattern, use[pattern][i][j], false);//no blue yet!
				}
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