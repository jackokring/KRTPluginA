#include "plugin.hpp"

char onDisplay1[] = "KRTOMEGA";

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
		NUM_LIGHTS
	};

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
		"A",//I
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
		1,//I
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

	char randomz[65];
	char offsets[65];

	int ptrRandomz = 0;
	int ptrOffsets = 0;

	char getDigit(int ptrR, int ptrO, float seed) {
		seed += ptrRandomz;
		int s1 = ((int) seed) & 63;//pos
		float rem = modulo(seed, 64);//remainder
		int s2 = s1 + 1;
		s1 = randomz[s1];
		s2 = randomz[s2];
		rem = s1 * (1.f - rem) + s2 * rem;
		s1 = (int) rem;
		s1 += offsets[ptrO & 63];//eveentual char
		return (char) ((s1 % 27) + '@');//@ plus
	}

	void putDigit(int ptrR, int ptrO, float seed, char digit) {
		offsets[ptrO & 63] = 0;//blank zero 
		char x = getDigit(ptrR, ptrO, seed) - '@';//producing
		offsets[ptrO & 63] = digit - x;//calc offset to make digit
	}

	float modulo(float x, float m) {
		float div = x / m;
		long d = (long) div;
		float rem = x - d * m;
		return rem;
	}

	json_t* dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "save", json_string(offsets));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {  
		json_t* textJ = json_object_get(rootJ, "save");
  		if (textJ) {
			const char *str = json_string_value(textJ);
			for(int i = 0; i < 64; i++) {
				offsets[i] = str[i];//limit buffer size hack
			}
		}
	}

	int mux = 0;

	void showOnDisplay(float seed) {
		if((mux & 1023) == 0) {
			for(int i = 0; i < 8; i++) {
				onDisplay1[i] = getDigit(ptrRandomz + i, ptrOffsets + i, seed);
			}
		}
		mux++;//
	}

	Om() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(BIRD, 0.f, 100.f, 0.f, "Apply Bird", " %");
		//decide symbol rangs ...
		configParam(SEED, 0.f, 64.f, 0.f, "Seed");
		configParam(VAR, 0.f, 100.f, 50.f, "Random", " %");
		for(int o = 0; o < PORT_MAX_CHANNELS; o++) {
			outSym[o] = 0;
		}
		srand(666);
		for(int o = 0; o < 64; o++) {
			randomz[o] = (char) rand();//a randomized pattern
			offsets[o] = 0;//calculation offsets
		}
		randomz[64] = randomz[0];//loop
		offsets[64] = '\n';//end
	}

	//obtain mapped control value
    float log(float val, float centre) {
        return powf(2.f, val) * centre;
    }

	const int out[13] = {
		AN, BO, CP,	DQ,	ER,	FS,	GT,	HU,	IV,	JW,	KX,	LY,	MZ
	};

	int outSym[PORT_MAX_CHANNELS];

	dsp::SchmittTrigger sClk[PORT_MAX_CHANNELS];
	dsp::SchmittTrigger sRst;

	void process(const ProcessArgs& args) override {
		//float fs = args.sampleRate;
		int maxPort = maxPoly();
		//umm polyphony?

		float bird = params[BIRD].getValue()/100.f;
		float seed = params[SEED].getValue();
		if(seed >= 27.f) seed = 1.f;//wrap
		float var = params[VAR].getValue()/100.f;

		// PARAMETERS (AND IMPLICIT INS)
		float rst = inputs[RST].getVoltageSum();//signal OR
		bool trigRst = sRst.process(rescale(rst, 0.1f, 2.f, 0.f, 1.f));
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
		clk = sClk[0].state ? 10.f : 0.f;
		float clk2 = sClk[0].state ? 0.f : 10.f;
		if(trigRst) {
			//on reset
			ptrRandomz = ptrOffsets = 0;//reset
		} else if(trigClk) {
			//on clock step forward
			ptrRandomz++;
			ptrOffsets++;
			//apply bird

			//apply rand

		}
		showOnDisplay(seed);
		outSym[0] = getDigit(ptrRandomz, ptrOffsets, seed);//set out
#pragma GCC ivdep
		for(int p = 1; p < maxPort; p++) {
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

char *showNow = onDisplay1;//use pointer for changing display based on context

NVGcolor prepareDisplay(NVGcontext *vg, Rect *box, int fontSize) {
	NVGcolor backgroundColor = nvgRGB(0x10, 0x10, 0x10); 
	nvgBeginPath(vg);
	nvgRoundedRect(vg, 0.f, 0.f, box->size.x, box->size.y, 3.f);
	nvgFillColor(vg, backgroundColor);
	nvgFill(vg);
	nvgFontSize(vg, fontSize);
	NVGcolor textColor = nvgRGB(0x5d, 0xfc, 0x0a);
	return textColor;
}

struct DisplayWidget : LightWidget {//TransparentWidget {
	std::shared_ptr<Font> font;
	std::string fontPath;
	char **what;
	
	DisplayWidget(char **p) {
		fontPath = std::string(asset::plugin(pluginInstance, "res/fonts/Segment14.ttf"));
		what = p;
	}

	void draw(const DrawArgs &args) override {
		if (!(font = APP->window->loadFont(fontPath))) {
			return;
		}
		NVGcolor textColor = prepareDisplay(args.vg, &box, 18);
		nvgFontFaceId(args.vg, font->handle);

		Vec textPos = Vec(4, 22);
		nvgFillColor(args.vg, textColor);
		nvgText(args.vg, textPos.x, textPos.y, *what, NULL);
	}

	void fixCentre(Vec here, int many) {
		box.size = Vec(14.6f * many + 8, 26);
		box.pos = Vec(here.x - box.size.x / 2, here.y - box.size.y / 2);
	}
};

struct OmWidget : ModuleWidget {

	OmWidget(Om* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Om.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		DisplayWidget *display = new DisplayWidget(&showNow);
		display->fixCentre(loc(2, 1), 8);//chars
		addChild(display);

		addParam(createParamCentered<RoundBlackKnob>(loc(1, 2), module, Om::BIRD));
		addParam(createParamCentered<RoundBlackKnob>(loc(2, 2), module, Om::SEED));
		addParam(createParamCentered<RoundBlackKnob>(loc(3, 2), module, Om::VAR));	

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