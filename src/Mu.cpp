#include "plugin.hpp"

//f_x1 = (1.5265105587276737e+48*f[i-9]-1.3668387579017067e+49*f[i-8]+5.431248343252391e+49*f[i-7]-1.2560580881508727e+50*f[i-6]+1.8604974291269024e+50*f[i-5]-1.825112874436707e+50*f[i-4]+1.1774257443584765e+50*f[i-3]-4.70911457297244e+49*f[i-2]+9.245318227649387e+48*f[i-1])/(5.61659602207866e+47*1.0*h**1)
//f_x2 = (8.145411592862319e+40*f[i-9]-7.24075814611475e+41*f[i-8]+2.8505272093785156e+42*f[i-7]-6.511132716988107e+42*f[i-6]+9.479167232763678e+42*f[i-5]-9.062466169644141e+42*f[i-4]+5.604592622540067e+42*f[i-3]-2.0690575636637124e+42*f[i-2]+3.50991084293707e+41*f[i-1])/(1.3901620164136395e+40*1.0*h**2)
//f_x3 = (4.768402943555988e+41*f[i-9]-4.198496441152479e+42*f[i-8]+1.6329646901392421e+43*f[i-7]-3.6714916748050246e+43*f[i-6]+5.231749630181975e+43*f[i-5]-4.851706129693572e+43*f[i-4]+2.866756706165198e+43*f[i-3]-9.882480374551996e+42*f[i-2]+1.5214043014568113e+42*f[i-1])/(4.762449890092781e+40*1.0*h**3)

struct Mu : Module {
	enum ParamIds {
		DB,
		HZ,
		LAM,
		G1,
		G2,
		G3,
		NUM_PARAMS
	};
	enum InputIds {
		CVDB,
		CVHZ,
		CVLAM,
		IN1,
		IN2,
		IN3,
		NUM_INPUTS
	};
	enum OutputIds {
		D1,
		D2,
		D3,
		I1,
		I2,
		I3,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Mu() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs& args) override {
	}
};

//geometry edit
#define HP 7
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

struct MuWidget : ModuleWidget {
	MuWidget(Mu* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Mu.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(loc(1, 1), module, Mu::DB));
		addParam(createParamCentered<RoundBlackKnob>(loc(2, 1), module, Mu::HZ));
		addParam(createParamCentered<RoundBlackKnob>(loc(3, 1), module, Mu::DB));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 2), module, Mu::G1));
		addParam(createParamCentered<RoundBlackKnob>(loc(2, 2), module, Mu::G2));
		addParam(createParamCentered<RoundBlackKnob>(loc(3, 2), module, Mu::G3));

		addInput(createInputCentered<PJ301MPort>(loc(1, 3), module, Mu::CVDB));
		addInput(createInputCentered<PJ301MPort>(loc(2, 3), module, Mu::CVHZ));
		addInput(createInputCentered<PJ301MPort>(loc(3, 3), module, Mu::CVLAM));
		addInput(createInputCentered<PJ301MPort>(loc(1, 4), module, Mu::IN1));
		addInput(createInputCentered<PJ301MPort>(loc(2, 4), module, Mu::IN2));
		addInput(createInputCentered<PJ301MPort>(loc(3, 4), module, Mu::IN3));

		addOutput(createOutputCentered<PJ301MPort>(loc(1, 5), module, Mu::D1));
		addOutput(createOutputCentered<PJ301MPort>(loc(2, 5), module, Mu::D2));
		addOutput(createOutputCentered<PJ301MPort>(loc(3, 5), module, Mu::D3));
		addOutput(createOutputCentered<PJ301MPort>(loc(1, 6), module, Mu::I1));
		addOutput(createOutputCentered<PJ301MPort>(loc(2, 6), module, Mu::I2));
		addOutput(createOutputCentered<PJ301MPort>(loc(3, 6), module, Mu::I3));
	}
};


Model* modelMu = createModel<Mu, MuWidget>("Mu");