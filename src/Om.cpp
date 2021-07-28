#include "plugin.hpp"


struct Om : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Om() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs& args) override {
	}
};

//geometry edit
#define HP 9
#define LANES 3
#define RUNGS 6

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

char onDisplay1[3] = "OK";
char *showNow = onDisplay1;//use pointer for changing display based on context

NVGcolor prepareDisplay(NVGcontext *vg, Rect *box, int fontSize) {
	NVGcolor backgroundColor = nvgRGB(0x38, 0x38, 0x38); 
	NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
	nvgBeginPath(vg);
	nvgRoundedRect(vg, 0.0, 0.0, box->size.x, box->size.y, 5.0);
	nvgFillColor(vg, backgroundColor);
	nvgFill(vg);
	nvgStrokeWidth(vg, 1.0);
	nvgStrokeColor(vg, borderColor);
	nvgStroke(vg);
	nvgFontSize(vg, fontSize);
	NVGcolor textColor = nvgRGB(0xaf, 0xd2, 0x2c);
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

		Vec textPos = Vec(6, 24);//?
		nvgFillColor(args.vg, nvgTransRGBA(textColor, 23));//alpha 23
		nvgText(args.vg, textPos.x, textPos.y, "~~", NULL);
		nvgFillColor(args.vg, textColor);
		nvgText(args.vg, textPos.x, textPos.y, *what, NULL);
	}

	void fixCentre(Vec here, int many) {
		box.size = Vec(12 * many + 12, 30);
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
		display->fixCentre(loc(1, 1), 2);//chars
		addChild(display);	
	}
};


Model* modelOm = createModel<Om, OmWidget>("Om");