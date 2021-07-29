#include "plugin.hpp"


struct Om : Module {
	enum ParamIds {

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

	Om() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(SEED, 1.f, 27.f, 14.f, "Reset Seed", " Symbol");
		configParam(VAR, 0.f, 100.f, 50.f, "Random", " %");
	}

	void process(const ProcessArgs& args) override {
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

char onDisplay1[] = "KRTOMEGA";
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

		addParam(createParamCentered<RoundBlackKnob>(loc(1, 2), module, Om::VAR));
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