#include "plugin.hpp"


struct Y : Module {
	enum ParamIds {
		ENUMS(QUADS, 16),
		ENUMS(TRIPS, 12),
		RUN,
		RST,
		TEMPO,
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
		NUM_LIGHTS
	};

	int maxPoly() {
		for(int o = 0; o < NUM_OUTPUTS; o++) {
			outputs[o].setChannels(1);
		}
		return 1;//monophonic
	}

	Y() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(TEMPO, 0.f, 240.f, 120.f, "Tempo", " bpm");
	}

	void process(const ProcessArgs& args) override {
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
		addChild(createLightCentered<LEDBezelLight<GreenLight>>(loc(8, 4), module, Y::RUN));
		addOutput(createOutputCentered<PJ301MPort>(loc(8, 2.5f), module, Y::ORUN));

		addParam(createParamCentered<LEDBezel>(loc(7, 4.75f), module, Y::RST));
		addChild(createLightCentered<LEDBezelLight<GreenLight>>(loc(7, 4), module, Y::RST));
		addOutput(createOutputCentered<PJ301MPort>(loc(7, 2.5f), module, Y::ORST));

		addParam(createParamCentered<RoundBlackKnob>(loc(7.5f, 3.5f), module, Y::TEMPO));

		addInput(createInputCentered<PJ301MPort>(loc(1, 2.5f), module, Y::ICV_BUT));
		addInput(createInputCentered<PJ301MPort>(loc(2, 2.5f), module, Y::IGATE_BUT));	
	}
};


Model* modelY = createModel<Y, YWidget>("Y");