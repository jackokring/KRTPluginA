#include "plugin.hpp"

char onDisplayD[9] = "KRTOMEGA";
char *showNow = onDisplayD;//use pointer for changing display based on context

struct Om : Module {
	enum ParamIds {
		BIRD,
		SEED,
		VAR,
		NUM_PARAMS
	};
	enum InputIds {
		CLK,
		RST,
		NUM_INPUTS
	};
	enum OutputIds {
		AN,

		BO,
		CP,
		DQ,

		ER,
		FS,
		GT,

		HU,
		IV,
		JW,

		KX,
		LY,
		MZ,
		NUM_OUTPUTS
	};
	enum LightIds {
		LRAND,
		LBIRD,
		LCLK,
		NUM_LIGHTS
	};

	const char *instring[NUM_INPUTS] = {
		"Clock",
		"Reset",
	};

	const char *outstring[NUM_OUTPUTS] = {
		"Letter pair hi/lo trigger",
		"Letter pair hi/lo trigger",
		"Letter pair hi/lo trigger",
		"Letter pair hi/lo trigger",

		"Letter pair hi/lo trigger",
		"Letter pair hi/lo trigger",
		"Letter pair hi/lo trigger",
		"Letter pair hi/lo trigger",

		"Letter pair hi/lo trigger",
		"Letter pair hi/lo trigger",
		"Letter pair hi/lo trigger",
		"Letter pair hi/lo trigger",

		"Letter pair hi/lo trigger",
	};

	const char *lightstring[NUM_LIGHTS] = {
		"Randomized",
		"Bird applied",
		"Clock active",
	};

	void iol(bool lights) {
		for(int i = 0; i < NUM_INPUTS; i++) configInput(i, instring[i]);
		for(int i = 0; i < NUM_OUTPUTS; i++) configOutput(i, outstring[i]);
		if(!lights) return;
		for(int i = 0; i < NUM_LIGHTS; i++) configLight(i, lightstring[i]);
	}

	char onDisplay1[9] = "        ";
	char onDisplay2[9] = "        ";//double
	bool flip = false;
	char *showNowM = onDisplay1;

	int maxPoly() {
		int poly = inputs[CLK].getChannels();
		for(int o = 0; o < NUM_OUTPUTS; o++) {
			outputs[o].setChannels(poly);
		}
		return poly;
	}

	//char rang @A-Z (with @ as composition of next 2 with potential nest)
	const char *birdTo[26] = {
		"A@B@CD",//A - Becard - for @@x -> @x@
		"A@BC",//B
		"ACB",//C
		"AB@CD",//D
		"AB@@CDE",//E
		"CBA",//F
		"AD@BC",//G
		"ABCB",//H
		"",//I -- efficiency
		"AB@@ADC",//J
		"A",//K
		"A@BB",//L
		"AA",//M
		"AB@AB",//N - Double Mockingbird
		"B@AB",//O
		"YO",//P - Theta bird YO special by YO > E
		"B@AC",//Q
		"BCA",//R
		"AC@BC",//S
		"BA",//T
		"B@@AAB",//U
		"CAB",//V
		"ABB",//W
		"LL",//X - Double Lark - special by LL > E
		"SLL",//Y - special by SLL > E
		"B" //Z - Kite
	};

	int birdConsume[26] = {
		4,//A
		3,//B
		3,//C
		4,//D
		5,//E
		3,//F
		4,//G
		3,//H
		0,//I
		4,//J
		2,//K
		2,//L
		1,//M
		2,//N
		2,//O
		0,//P
		3,//Q
		3,//R
		3,//S
		2,//T
		2,//U
		3,//V
		2,//W
		0,//X
		0,//Y
		2 //Z
	};

	int birdLen[26];

	unsigned char randomz[65];
	unsigned char offsets[64];
	unsigned char store[5][65];//data temp store A-E
	int storeLen[5];

	unsigned int ptrOffsets = 0;

	unsigned char getDigit(unsigned int ptrO, float seed) {
		seed += ptrO;
		unsigned int s1 = ((int) seed) & 63;//pos
		float rem = modulo(seed, 1.f);//remainder and positive for
		unsigned int s2 = s1 + 1;
		s1 = randomz[s1];
		s2 = randomz[s2];
		rem = s1 * (1.f - rem) + s2 * rem;
		s1 = (int) rem;
		s1 += offsets[ptrO & 63];//eveentual char
		return (unsigned char) ((s1 % 27) + '@');//@ plus
	}

	int alterOffPtr(int off) {
		int tmp = ptrOffsets;//post increment
		ptrOffsets += off;
		ptrOffsets &= 63;//modulo
		return tmp;
	}

	void putDigit(unsigned int ptrO, float seed, unsigned char digit) {
		offsets[ptrO & 63] = 0;//blank zero
		unsigned char x = getDigit(ptrO, seed);//producing
		offsets[ptrO & 63] = digit - x;//calc offset to make digit
	}

	void readNest(int num, unsigned int ptrMax, float seed) {
		unsigned char x = getDigit(alterOffPtr(1), seed);//keep modulo
		store[num][storeLen[num]++] = x;
		if(x == '@' && (ptrOffsets != ptrMax)) {
			//bracket
			readNest(num, ptrMax, seed);
			if(ptrOffsets != ptrMax) {//avoid overflow
				readNest(num, ptrMax, seed);
			}
		}
	}

	void getBird(int many, unsigned int ptrMax, float seed) {
		for(int i = 0; i < many; i++) {
			storeLen[i] = 0;
			readNest(i, ptrMax, seed);//beginning
		}
	}

	void writeNest(int num, unsigned int ptrMax, float seed) {
		for(int i = storeLen[num] - 1; i != -1; i--) {
			if(ptrOffsets != ptrMax) {
				putDigit(alterOffPtr(-1), seed, store[num][i]);//write backwards
			}
		}
	}

	void putBird(const char *how, int len, unsigned int ptrMax, float seed) {
		alterOffPtr(-1);//align placement removing last post increment
		for(int i = len - 1; i != -1; i--) {
			unsigned char x = how[i];//action
			if(x <= 'E' && x > '@') {//not literals
				writeNest(x - 'A', ptrMax, seed);
			} else {
				//literal
				if(ptrOffsets != ptrMax) {
					putDigit(alterOffPtr(-1), seed, x);
				}
			}
		}
		alterOffPtr(1);//place on first
	}

	float modulo(float x, float m) {
		float div = x / m;
		long d = (long) div;
		float rem = x - d * m;
		return rem;
	}

	char saves[64];//pesistance?

	json_t* dataToJson() override {
		json_t *rootJ = json_object();
		for(int i = 0; i < 64; i++) {
			saves[i] = offsets[i] + '@';//limit buffer size hack
		}
		json_object_set(rootJ, "save", json_stringn(saves, 64));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* textJ = json_object_get(rootJ, "save");
  		if (textJ) {
			const char *str = json_string_value(textJ);
			if(str) {
				for(int i = 0; i < 64; i++) {
					offsets[i] = str[i] - '@';//limit buffer size hack
				}
			}
		}
	}

	int mux = 0;

	void showOnDisplay(float seed) {
		if((mux & 1023) == 0) {
			for(int i = 0; i < 8; i++) {
				char x = getDigit(ptrOffsets + i, seed);
				if(flip) {
					onDisplay1[i] = x;
				} else {
					onDisplay2[i] = x;
				}
			}
			flip = !flip;
			//double buffer
			if(flip) {
				showNowM = onDisplay2;
			} else {
				showNowM = onDisplay1;
			}
		}
		mux++;//
	}

	bool percent(double per) {
		per *= (0.01f * RAND_MAX);
		return rand() < per;//should work
	}

	Om() {
		/* https://vcvrack.com/manual/Migrate2#3-New-optional-v2-API-features
		configInput(PITCH_INPUT, "1V/oct pitch");
		configOutput(SIN_OUTPUT, "Sine");
		configLight(PHASE_LIGHT, "Phase");
		//replace configParam with either below ...
		configButton(TAP_PARAM);
		configSwitch(SYNC_PARAM, 0, 1, 0, "Sync mode", {"Soft", "Hard"});
		//default bypass
		configBypass(LEFT_INPUT, LEFT_OUTPUT);
		configBypass(RIGHT_INPUT, RIGHT_OUTPUT);
		*/
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(BIRD, 0.f, 100.f, 0.f, "Apply bird rate", " %");
		//decide symbol rangs ...
		configParam(SEED, 0.f, 64.f, 0.f, "Seed random");
		configParam(VAR, 0.f, 100.f, 50.f, "Randomize rate", " %");
		iol(true);
		for(int o = 0; o < PORT_MAX_CHANNELS; o++) {
			outSym[o] = 0;
		}
		srand(666);
		for(int o = 0; o < 64; o++) {
			randomz[o] = (unsigned char) (rand() % 27);//a randomized pattern
			offsets[o] = 0;//calculation offsets
		}
		randomz[64] = randomz[0];//loop
		for(int o = 0; o < 26; o++) {
			birdLen[o] = strlen(birdTo[o]);//opt
		}
	}

	//obtain mapped control value
    float log(float val, float centre) {
        return powf(2.f, val) * centre;
    }

	const int out[13] = {
		AN, BO, CP,	DQ,	ER,	FS,	GT,	HU,	IV,	JW,	KX,	LY,	MZ
	};

	unsigned char outSym[PORT_MAX_CHANNELS];

	dsp::SchmittTrigger sClk[PORT_MAX_CHANNELS];
	dsp::SchmittTrigger sRst;

	void process(const ProcessArgs& args) override {
		//float fs = args.sampleRate;
		int maxPort = maxPoly();
		//umm polyphony?

		float bird = params[BIRD].getValue();
		float seed = params[SEED].getValue();
		float var = params[VAR].getValue();

		// PARAMETERS (AND IMPLICIT INS)
		float rst = inputs[RST].getVoltageSum();//signal OR
		sRst.process(rescale(rst, 0.1f, 2.f, 0.f, 1.f));
		for(int p = maxPort - 1; p > 0; p--) {//Assume branch into unroll number ...
			float clk = inputs[CLK].getPolyVoltage(p);
			bool trigClk = sClk[p].process(rescale(clk, 0.1f, 2.f, 0.f, 1.f));
			if(trigClk) {
				outSym[p] = outSym[p - 1];//chain arp
			}
		}
		//clock normalization
		float clk = inputs[CLK].getPolyVoltage(0);
		bool trigClk = sClk[0].process(rescale(clk, 0.1f, 2.f, 0.f, 1.f));
		clk = sClk[0].isHigh() ? 10.f : 0.f;
		float clk2 = sClk[0].isHigh() ? 0.f : 10.f;
		lights[LCLK].setBrightness(clk);//only needs to be 1.f
		if(sRst.isHigh()) {
			//on reset
			ptrOffsets = 0;//reset
			lights[LRAND].setBrightness(0.f);
			lights[LBIRD].setBrightness(0.f);
			//ignore synchronous clock trigger
			//unlike MIDI which reset does not override
			//first clock
		} else if(trigClk) {
			//on clock step forward
			unsigned char x = getDigit(ptrOffsets, seed) - '@';
			ptrOffsets++;
			//apply bird
			if(x != 0 && percent(bird)) {//on prob
				int consume = birdConsume[x - 1];//details
				const char *to = birdTo[x - 1];
				int len = birdLen[x - 1];
				getBird(consume, ptrOffsets, seed);
				putBird(to, len, ptrOffsets, seed);
				lights[LBIRD].setBrightness(1.f);
			} else {
				lights[LBIRD].setBrightness(0.f);
			}
			//apply rand
			if(percent(var)) {//on prob
				putDigit(ptrOffsets, seed, (unsigned char) (rand() % 27 + '@'));
				//modify
				lights[LRAND].setBrightness(1.f);
			} else {
				lights[LRAND].setBrightness(0.f);
			}
		}
		showOnDisplay(seed);
		outSym[0] = getDigit(ptrOffsets, seed) - '@';//set out
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {
			// OUTS
#pragma GCC ivdep
			for(int i = 0; i < 13; i++) {
				float combined = 0.f;
				if(outSym[p] == i + 1) {
					combined = clk;
				}
				if(outSym[p] == i + 14) {
					combined = clk2;
				}
				outputs[out[i]].setVoltage(combined, p);
			}
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

void prepareDisplay(NVGcontext *vg, Rect *box) {
	NVGcolor backgroundColor = nvgRGB(0x10, 0x10, 0x10);
	nvgBeginPath(vg);
	nvgRoundedRect(vg, 0.f, 0.f, box->size.x, box->size.y, 3.f);
	nvgFillColor(vg, backgroundColor);
	nvgFill(vg);
}

struct DisplayWidget : LightWidget {//TransparentWidget {
	std::string fontPath;
	char **what = NULL;

	DisplayWidget() {
		//maybe will change to default 7 seg font to reduce size??
		//std::string(asset::plugin(pluginInstance, "res/fonts/Segment14.ttf"));
		fontPath = asset::system("res/fonts/DSEG7ClassicMini-Regular.ttf");
	}

	void set(char **p) {
		what = p;
	}

	void drawLayer(const DrawArgs& args, int layer) override {
		if (layer == 1 /* || layer == 0 */) {
			drew(args);
		}
		Widget::drawLayer(args, layer);
	}

	void draw(const DrawArgs &args) override {//background
		prepareDisplay(args.vg, &box);
	}

	void drew(const DrawArgs &args) {//foreground
		std::shared_ptr<Font> font;
		if (!(font = APP->window->loadFont(fontPath))) {
			return;
		}
		NVGcolor textColor = nvgRGB(0x5d, 0xfc, 0x0a);//green
		nvgFontFaceId(args.vg, font->handle);
		nvgFontSize(args.vg, 18);

		Vec textPos = Vec(4, 22);
		nvgFillColor(args.vg, textColor);
		nvgText(args.vg, textPos.x, textPos.y, *what, NULL);
	}

	void fixCentre(Vec here, int many) {//locate control
		box.size = Vec(14.6f * many + 8, 26);
		box.pos = Vec(here.x - box.size.x / 2, here.y - box.size.y / 2);
	}
};

struct OmWidget : ModuleWidget {
	Om *pModule = NULL;
	DisplayWidget *display;

	void step() override {
		ModuleWidget::step();
		if (pModule != NULL) {
			display->set(&(pModule->showNowM));//valid?
		}
	}

	OmWidget(Om* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Om.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		display = new DisplayWidget();
		display->fixCentre(loc(2, 1), 8);//chars
		display->set(&showNow);
		pModule = module;//hook
		addChild(display);

		addParam(createParamCentered<RoundBlackKnob>(loc(1, 2), module, Om::BIRD));
		addParam(createParamCentered<RoundBlackKnob>(loc(2, 2), module, Om::SEED));
		addParam(createParamCentered<RoundBlackKnob>(loc(3, 2), module, Om::VAR));

		addChild(createLightCentered<SmallLight<GreenLight>>(loc(1, 1.5f), module, Om::LBIRD));
		addChild(createLightCentered<SmallLight<GreenLight>>(loc(2, 1.5f), module, Om::LCLK));
		addChild(createLightCentered<SmallLight<GreenLight>>(loc(3, 1.5f), module, Om::LRAND));

		addInput(createInputCentered<PJ301MPort>(loc(1, 3), module, Om::CLK));
		addInput(createInputCentered<PJ301MPort>(loc(2, 3), module, Om::RST));
		addOutput(createOutputCentered<PJ301MPort>(loc(3, 3), module, Om::AN));

		addOutput(createOutputCentered<PJ301MPort>(loc(1, 4), module, Om::BO));
		addOutput(createOutputCentered<PJ301MPort>(loc(2, 4), module, Om::CP));
		addOutput(createOutputCentered<PJ301MPort>(loc(3, 4), module, Om::DQ));

		addOutput(createOutputCentered<PJ301MPort>(loc(1, 5), module, Om::ER));
		addOutput(createOutputCentered<PJ301MPort>(loc(2, 5), module, Om::FS));
		addOutput(createOutputCentered<PJ301MPort>(loc(3, 5), module, Om::GT));

		addOutput(createOutputCentered<PJ301MPort>(loc(1, 6), module, Om::HU));
		addOutput(createOutputCentered<PJ301MPort>(loc(2, 6), module, Om::IV));
		addOutput(createOutputCentered<PJ301MPort>(loc(3, 6), module, Om::JW));

		addOutput(createOutputCentered<PJ301MPort>(loc(1, 7), module, Om::KX));
		addOutput(createOutputCentered<PJ301MPort>(loc(2, 7), module, Om::LY));
		addOutput(createOutputCentered<PJ301MPort>(loc(3, 7), module, Om::MZ));
	}
};


Model* modelOm = createModel<Om, OmWidget>("Om");
