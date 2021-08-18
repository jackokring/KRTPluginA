#include "plugin.hpp"


struct Q : Module {
	enum ParamIds {
		OMEGA,//master uncertainty mass frequency
		SINGULAR_HYSTERISIS,//avoid div zero range
		PLANK,//quantum constant
		NEWTON,//gravity constant
		MASS_RATIO,//mass ratio of driver to driven
		ANGLE,//radial to tangential
		NUM_PARAMS
	};
	enum InputIds {
		IN,
		IOMEGA,//master uncertainty mass frequency
		ISINGULAR_HYSTERISIS,//avoid div zero range
		IPLANK,//quantum constant
		INEWTON,//gravity constant
		IMASS_RATIO,//mass ratio of driver to driven
		IANGLE,//radial to tangential
		NUM_INPUTS
	};
	enum OutputIds {
		OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Q() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs& args) override {
	}
};

//geometry edit
#define HP 5
#define LANES 2
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

struct QWidget : ModuleWidget {
	QWidget(Q* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Q.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	}
};


Model* modelQ = createModel<Q, QWidget>("Q");