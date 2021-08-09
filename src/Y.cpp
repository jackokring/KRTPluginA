#include "plugin.hpp"

struct Y : Module {
	enum ParamIds {
		ENUMS(QUADS, 16),
		ENUMS(TRIPS, 12),
		RUN,
		RST,
		TEMPO,
		ENUMS(MODES, 4),
		LEN,
		IS_RUN,
		MODE,
		ENUMS(PAT, 16),
		CPY,
		PST,
		CHAN,
		ENUMS(MUTES, 16),
		NUM_PARAMS
	};
	enum InputIds {
		ICV_BUT,
		IGATE_BUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(OUTS, 16),
		ORUN,
		ORST,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(LQUADS, 16),
		ENUMS(LTRIPS, 12),
		LRUN,
		LRST,
		ENUMS(LMODE, 4),
		LCPY,
		LPST,
		NUM_LIGHTS
	};

	const static int patNum = 16;
	const static int stepsNum = 16 + 12;
	const static int chanNum = 16;

	bool patterns[patNum][stepsNum][chanNum];

	const static int sized = chanNum * stepsNum * patNum;
	char saves[sized + 1];//space

	json_t* dataToJson() override {
		json_t *rootJ = json_object();
		for(int p = 0; p < patNum; p++) {
			for(int s = 0; s < stepsNum; s++) {
				for(int c = 0; c < chanNum; c++) {
					int i = c + chanNum * (s + stepsNum * p);
					saves[i] = patterns[p][s][c] ? 'T' : 'F';
				}
			}
		}
		saves[sized] = '\0';
		json_object_set_new(rootJ, "save", json_string(saves));
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {  
		json_t* textJ = json_object_get(rootJ, "save");
  		if (textJ) {
			const char *str = json_string_value(textJ);
			if(str) {
				for(int p = 0; p < patNum; p++) {
					for(int s = 0; s < stepsNum; s++) {
						for(int c = 0; c < chanNum; c++) {
							int i = c + chanNum * (s + stepsNum * p);
							patterns[p][s][c] = (str[i] == 'T') ? true : false;
						}
					}
				}
			}
		}
	}

	void maxPoly() {
		for(int o = 0; o < NUM_OUTPUTS; o++) {
			outputs[o].setChannels(1);
		}
	}

	const char qNames[16][4] = {
		"1/1", "2/1", "3/1", "4/1",
		"1/2", "2/2", "3/2", "4/2",
		"1/3", "2/3", "3/3", "4/3",
		"1/4", "2/4", "3/4", "4/4"
	};

	const char tNames[12][5] = {
		"1T/1", "2T/1", "3T/1",
		"1T/2", "2T/2", "3T/2",
		"1T/3", "2T/3", "3T/3",
		"1T/4", "2T/4", "3T/4"
	};

	const char *mNames[4] = {
		"Pattern", "Sequence", "Mute", "Now"
	};

	Y() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(TEMPO, 0.f, 240.f, 120.f, "Tempo", " bpm");
		configParam(LEN, 0.f, 100.f, 50.f, "Gate Length", " %");
		for(int i = 0; i < 16; i++) {
			configParam(QUADS + i, 0.f, 1.f, 0.f, qNames[i]);
		}
		for(int i = 0; i < 12; i++) {
			configParam(TRIPS + i, 0.f, 1.f, 0.f, tNames[i]);
		}
		configParam(RUN, 0.f, 1.f, 0.f, "Run and Stop");
		configParam(RST, 0.f, 1.f, 0.f, "Reset");
		for(int i = 0; i < 4; i++) {
			configParam(MODES + i, 0.f, 1.f, 0.f, mNames[i]);
		}
		configParam(IS_RUN, 0.f, 1.f, 0.f);
		configParam(MODE, 0.f, 3.f, 0.f);
		for(int i = 0; i < 16; i++) {
			configParam(PAT + i, 0.f, patNum - 1.f, 0.f);//default pattern
			configParam(MUTES + i, 0.f, 1.f, 1.f);
		}
		configParam(CPY, 0.f, 1.f, 0.f, "Copy");
		configParam(PST, 0.f, 1.f, 0.f, "Paste");
		configParam(CHAN, 0.f, chanNum - 1.f, 0.f);
		for(int p = 0; p < patNum; p++) {
			for(int s = 0; s < stepsNum; s++) {
				for(int c = 0; c < chanNum; c++) {
					patterns[p][s][c] = false;//blank
				}
			}
		}
	}

	int sampleCounter = 0;
	dsp::SchmittTrigger sRun;
	dsp::SchmittTrigger sRst;
	dsp::SchmittTrigger sCpy;
	dsp::SchmittTrigger sPst;
	dsp::SchmittTrigger mode[4];
	dsp::SchmittTrigger quads[16];
	dsp::SchmittTrigger trips[12];

#define MODE_PAT 0
#define MODE_SEQ 1
#define MODE_MUT 2
#define MODE_NOW 3

	float light4(float beat, int light, int mode) {
		if(mode == MODE_PAT) {
			int p = getPat(beat);
			int chan = params[CHAN].getValue();
			bool on = (patterns[p][light][chan]) == true;
			bool in = ((int)beat & 15) == light;
			return (on ? 0.75f : 0.f) + (in ? 0.25f : 0.f);
		} else {//rest
			int p = getPat(beat);
			bool on = (patterns[p][(int)beat & 15][light]) == true;
			bool in = (p == light) && (mode == MODE_SEQ);
			in |= (params[MUTES + light].getValue() < 0.5f) && (mode == MODE_MUT);
			in |= ((int)params[CHAN].getValue() == light) && (mode == MODE_NOW);
			return (on ? 0.75f : 0.f) + (in ? 0.25f : 0.f);
		}
	}

	void button4(float beat, int button, int mode) {
		if(mode > 1) {
			if(mode > 2) {//MODE_NOW
				//directly done
				params[CHAN].setValue(button);//set channel
			} else {//MODE_MUT
				float x = params[MUTES + button].getValue();
				params[MUTES + button].setValue(1.f - x);//set mute
			}
		} else {
			if(mode < 1) {//MODE_PAT
				int p = getPat(beat);
				int chan = params[CHAN].getValue();
				(patterns[p][button][chan]) ^= true;//flip
			} else {//MODE_SEQ
				//12 pattern quad for 64 mux
				int pi = (int)beat >> 4;
				int p =	params[PAT + 12 + pi].getValue();//indirect on selected
				p = mod3[p];//for next choice
				params[PAT + p + 3 * pi].setValue(button);
			}
		}
	}

	char mod12[48] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
	};

	char mod3[16] = {
		0, 1, 2, 0, 1, 2, 0, 1, 2,
		0, 1, 2, 0, 1, 2, 0
	};

	char div3[12] = {
		0, 0, 0, 1, 1, 1, 2, 2, 2,
		3, 3, 3
	};

	int getPat(float beat) {
		//12 pattern quad for 64 mux
		int pi = (int)beat >> 4;
		int p =	params[PAT + 12 + pi].getValue();//indirect on selected
		p = mod3[p];//for next choice
		return params[PAT + p + 3 * pi].getValue();//indirect from choices
	}

	float out43(float beat, float tBeat, int out, int mode) {
		int p = getPat(beat);
		float l = params[LEN].getValue();
		bool q = patterns[p][(int)beat & 15][out];
		q &= onLen(beat, l);
		bool t = patterns[p][mod12[(int)tBeat] + 16][out];
		t &= onLen(tBeat, l);
		t |= q;//is on?
		if(mode == MODE_NOW) {
			t |= params[QUADS + out].getValue() > 0.5f;//play now
		}
		return (t ? 10.f : 0.f) * params[MUTES + out].getValue();//gate
	}

	float light3(float beat, float tBeat, int light, int mode) {
		if(mode == MODE_PAT) {
			int p = getPat(beat);
			int chan = params[CHAN].getValue();
			bool on = (patterns[p][light + 16][chan]) == true;
			bool in = mod12[(int)tBeat] == light;
			return (on ? 0.75f : 0.f) + (in ? 0.25f : 0.f);
		} else {//rest
			int r = mod3[light];//button in set
			int q = div3[light];//div 3
			bool on = ((int)params[PAT + 12 + q].getValue() == r);//0 - 2
			bool in = q == (int)beat >> 4;
			return (on ? 0.75f : 0.f) + (in ? 0.25f : 0.f);
		}
	}

	void button3(float beat, int button, int mode) {
		if(mode == MODE_PAT) {
			int p = getPat(beat);
			int chan = params[CHAN].getValue();
			(patterns[p][button + 16][chan]) ^= true;//flip
		} else {//rest
			int r = mod3[button];//button in set
			int q = div3[button];//div 3
			params[PAT + 12 + q].setValue(r);//0 - 2
		}
	}

	bool onLen(float beats, float len) {
		return beats - (int)beats < len;
	}

	void process(const ProcessArgs& args) override {
		float fs = args.sampleRate;
		float len = params[LEN].getValue() / 100.f;
		maxPoly();//1
		float bps = params[TEMPO].getValue() / 15.f;//beat per bar
		float beatSamp = bps / fs;//beats per sample
		float beats = beatSamp * (float)sampleCounter;
		float tBeats = beats * 0.75f;//triples
		float rst = params[RST].getValue();
		bool trigRst = sRst.process(rst);
		float run = params[RUN].getValue();
		bool trigRun = sRun.process(run);
		float cpy = params[CPY].getValue();
		bool trigCpy = sCpy.process(cpy);
		float pst = params[PST].getValue();
		bool trigPst = sPst.process(pst);
		int newMode = params[MODE].getValue();//old
#pragma GCC ivdep
		for(int i = 0; i < 4; i++) {
			float m = params[MODES + i].getValue();//buttons
			bool trigM = mode[i].process(m);
			if(trigM) {
				newMode = i;//set new
			}
			lights[LMODE + i].setBrightness((newMode == i) ? 1.f : 0.f);//radios
		}
		params[MODE].setValue(newMode);//change
		if(beats >= 64 || trigRst) {//sanity range before use
			sampleCounter = 0;//beats long
			beats = 0.f;//faster and sample accurate
			tBeats = 0.f;
		}
#pragma GCC ivdep
		for(int i = 0; i < 16; i++) {
			float but = params[QUADS + i].getValue();
			bool trig = quads[i].process(but);
			if(trig) {
				button4(beats, i, newMode);
			}
			lights[LQUADS + i].setBrightness(light4(beats, i, newMode));
			outputs[OUTS + i].setVoltage(out43(beats, tBeats, i, newMode));
		}
#pragma GCC ivdep
		for(int i = 0; i < 12; i++) {
			float but = params[TRIPS + i].getValue();
			bool trig = trips[i].process(but);
			if(trig) {
				button3(beats, i, newMode);
			}
			lights[LTRIPS + i].setBrightness(light3(beats, tBeats, i, newMode));
		}		
		if(trigRun) {
			params[IS_RUN].setValue(1.f - params[IS_RUN].getValue());//ok?
		}
		if(trigCpy) {

		}
		if(trigPst) {

		}
		if(onLen(beats, len)) {
			outputs[ORUN].setVoltage(10.f);
			lights[LRUN].setBrightness(1.f);
		} else {
			outputs[ORUN].setVoltage(0.f);
			lights[LRUN].setBrightness(0.f);
		}
		if(beats < len) {
			outputs[ORST].setVoltage(10.f);
			lights[LRST].setBrightness(1.f);
		} else {
			outputs[ORST].setVoltage(0.f);
			lights[LRST].setBrightness(0.f);
		}
		if(params[IS_RUN].getValue() > 0.5f) sampleCounter++;
	}
};

//geometry edit
#define HP 17
#define LANES 8
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

struct YWidget : ModuleWidget {
	YWidget(Y* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Y.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		for(int i = 0; i < 8; i++) {
			addParam(createParamCentered<LEDBezel>(loc(i + 1, 6), module, Y::QUADS + i));
			addParam(createParamCentered<LEDBezel>(loc(i + 1, 7), module, Y::QUADS + i + 8));

			addChild(createLightCentered<LEDBezelLight<GreenLight>>(loc(i + 1, 6), module, Y::LQUADS + i));
			addChild(createLightCentered<LEDBezelLight<GreenLight>>(loc(i + 1, 7), module, Y::LQUADS + i + 8));

			addOutput(createOutputCentered<PJ301MPort>(loc(i + 1, 1), module, Y::OUTS + i));
			addOutput(createOutputCentered<PJ301MPort>(loc(i + 1, 1.75f), module, Y::OUTS + i + 8));
		}

		for(int i = 0; i < 3; i++) {
			for(int j = 0; j < 4; j++) {
				float x = i + 1.5f + (j % 2) * 4;
				float y = 6 - 0.5f + (j / 2);
				addParam(createParamCentered<LEDBezel>(loc(x, y), module, Y::TRIPS + i + j * 3));
				addChild(createLightCentered<LEDBezelLight<GreenLight>>(loc(x, y), module, Y::LTRIPS + i + j * 3));
			}
		}

		addParam(createParamCentered<LEDBezel>(loc(8, 4.75f), module, Y::RUN));
		addChild(createLightCentered<LEDBezelLight<GreenLight>>(loc(8, 4.75f), module, Y::LRUN));
		addOutput(createOutputCentered<PJ301MPort>(loc(8, 2.5f), module, Y::ORUN));

		addParam(createParamCentered<LEDBezel>(loc(7, 4.75f), module, Y::RST));
		addChild(createLightCentered<LEDBezelLight<GreenLight>>(loc(7, 4.75f), module, Y::LRST));
		addOutput(createOutputCentered<PJ301MPort>(loc(7, 2.5f), module, Y::ORST));

		addParam(createParamCentered<RoundBlackKnob>(loc(7.5f, 3.5f), module, Y::TEMPO));
		addParam(createParamCentered<RoundBlackKnob>(loc(1.5f, 3.5f), module, Y::LEN));

		addInput(createInputCentered<PJ301MPort>(loc(1, 2.5f), module, Y::ICV_BUT));
		addInput(createInputCentered<PJ301MPort>(loc(2, 2.5f), module, Y::IGATE_BUT));

		for(int j = 0; j < 4; j++) {
			float x = 3 + j;
			float y = 3.f;
			addParam(createParamCentered<LEDBezel>(loc(x, y), module, Y::MODES + j));
			addChild(createLightCentered<LEDBezelLight<GreenLight>>(loc(x, y), module, Y::LMODE + j));
		}

		addParam(createParamCentered<LEDBezel>(loc(1, 4.75f), module, Y::CPY));
		addChild(createLightCentered<LEDBezelLight<GreenLight>>(loc(8, 4.75f), module, Y::LCPY));

		addParam(createParamCentered<LEDBezel>(loc(2, 4.75f), module, Y::PST));
		addChild(createLightCentered<LEDBezelLight<GreenLight>>(loc(8, 4.75f), module, Y::LPST));	
	}
};


Model* modelY = createModel<Y, YWidget>("Y");