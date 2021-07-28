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
#define LANES 4
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

const char onDisp[] = "OK";
char *show = &onDisp;

struct OmWidget : ModuleWidget {

	struct DisplayWidget : LightWidget {//TransparentWidget {
		std::shared_ptr<Font> font;
		std::string fontPath;
		char **what;
		
		DisplayWidget(char **textPtr) {
			what = textPtr;
			fontPath = std::string(asset::plugin(pluginInstance, "res/fonts/Segment14.ttf"));
		}

		void draw(const DrawArgs &args) override {
			if (!(font = APP->window->loadFont(fontPath))) {
				return;
			}
			NVGcolor textColor = prepareDisplay(args.vg, &box, 18);
			nvgFontFaceId(args.vg, font->handle);

			Vec textPos = VecPx(6, 24);
			nvgFillColor(args.vg, nvgTransRGBA(textColor, displayAlpha));
			nvgText(args.vg, textPos.x, textPos.y, "~~", NULL);
			nvgFillColor(args.vg, textColor);
			nvgText(args.vg, textPos.x, textPos.y, *textPtr, NULL);
		}
	};

	OmWidget(Om* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Om.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	DisplayWidget *display = new DisplayWidget(&show);
		display->box.pos = loc(1, 1);//try
		display->box.size = VecPx(24 * 2, 30);// 2 character
		addChild(display);	
	}
};


Model* modelOm = createModel<Om, OmWidget>("Om");