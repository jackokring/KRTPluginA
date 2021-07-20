#include "plugin.hpp"

//f_x1 = (1.5265105587276737e+48*f[i-9]-1.3668387579017067e+49*f[i-8]+5.431248343252391e+49*f[i-7]-1.2560580881508727e+50*f[i-6]+1.8604974291269024e+50*f[i-5]-1.825112874436707e+50*f[i-4]+1.1774257443584765e+50*f[i-3]-4.70911457297244e+49*f[i-2]+9.245318227649387e+48*f[i-1])/(5.61659602207866e+47*1.0*h**1)
//f_x2 = (8.145411592862319e+40*f[i-9]-7.24075814611475e+41*f[i-8]+2.8505272093785156e+42*f[i-7]-6.511132716988107e+42*f[i-6]+9.479167232763678e+42*f[i-5]-9.062466169644141e+42*f[i-4]+5.604592622540067e+42*f[i-3]-2.0690575636637124e+42*f[i-2]+3.50991084293707e+41*f[i-1])/(1.3901620164136395e+40*1.0*h**2)
//f_x3 = (4.768402943555988e+41*f[i-9]-4.198496441152479e+42*f[i-8]+1.6329646901392421e+43*f[i-7]-3.6714916748050246e+43*f[i-6]+5.231749630181975e+43*f[i-5]-4.851706129693572e+43*f[i-4]+2.866756706165198e+43*f[i-3]-9.882480374551996e+42*f[i-2]+1.5214043014568113e+42*f[i-1])/(4.762449890092781e+40*1.0*h**3)

struct Mu : Module {
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

	Mu() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs& args) override {
	}
};


struct MuWidget : ModuleWidget {
	MuWidget(Mu* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Mu.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	}
};


Model* modelMu = createModel<Mu, MuWidget>("Mu");