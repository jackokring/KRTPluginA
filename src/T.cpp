#include "plugin.hpp"


struct T : Module {
	enum ParamIds {
		NOTE,
		FINE,
		NUM_PARAMS
	};
	enum InputIds {
		IN,
		TRIG,
		NUM_INPUTS
	};
	enum OutputIds {
		HI,
		OUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	T() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(NOTE, 0.f, 11.f, 0.f, "Tune", " Semitones");
		configParam(FINE, 0.f, 100.f, 0.f, "Fine Tune", " Cents");
		for(int p = 0; p < PORT_MAX_CHANNELS; p++) {
			len[p] = lenL[p] = head[p] = tail[p] = 0.f;
			lastTrig[p] = hi[p] = false;
			wait[p] = true; 
		}
	}

	//buffer length
	const float MAX_BUFFER = 4096.f;//rounding?
	const float initPos = 0.f;
	float head[PORT_MAX_CHANNELS];
	float tail[PORT_MAX_CHANNELS];
	bool lastTrig[PORT_MAX_CHANNELS];

	//state control
	bool hi[PORT_MAX_CHANNELS];
	bool wait[PORT_MAX_CHANNELS];//wait for state to trigger
	float lenL[PORT_MAX_CHANNELS];
	float len[PORT_MAX_CHANNELS];

	//obtain mapped control value
    float log(float val, float centre) {
        return powf(2.f, val) * centre;
    }

	dsp::SchmittTrigger st[PORT_MAX_CHANNELS];

	bool putBuffer(float in, int chan) {
		return false;//reset
	}

	float getBuffer(float stepRelative, int chan) {
		return 0.f;//TODO
	}

	void resetBuffer(float first, int chan) {

		putBuffer(first, chan);//easier implemetation later
	}

	void process(const ProcessArgs& args) override {
				// For inputs intended to be used solely for audio, sum the voltages of all channels
		// (e.g. with Port::getVoltageSum())
		// For inputs intended to be used for CV or hybrid audio/CV, use the first channel’s
		// voltage (e.g. with Port::getVoltage())
		// POLY: Port::getPolyVoltage(c)
		float fs = args.sampleRate;
		int maxPort = inputs[TRIG].getChannels();
		if(maxPort == 0) maxPort = 1;

		float note = params[NOTE].getValue()/12.f;
		float fine = params[FINE].getValue()/1200.f;
		note += fine;
		note = log(note, 1.f);//relative hi note rate
		float low = note * 0.5f;//an octave down

		// PARAMETERS (AND IMPLICIT INS)
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {
			float in = inputs[IN].getPolyVoltage(p);
			float trig = inputs[TRIG].getPolyVoltage(p);
			st[p].process(rescale(trig, 0.1f, 2.f, 0.f, 1.f));

			if((!lastTrig[p] && st[p].isHigh()) || putBuffer(in, p)) { 
				len[p] = head[p];//get written length since trigger
				resetBuffer(in, p);
				lenL[p] = (2.f * low - 1.f) * len[p] / low;
				wait[p] = false;
				hi[p] = false;
			}
			lastTrig[p] = st[p].isHigh();

			float out;
			if(wait[p]) {
				out = getBuffer(1.f, p);//pass
			} else if(hi[p]) {
				if(tail[p] + note > len[p]) {
					out = getBuffer(1.f, p);//pass
					hi[p] = false;
					wait[p] = true;//reset
				} else {
					out = getBuffer(note, p);//high
				}
			} else {
				if(tail[p] + low > lenL[p]) {
					out = getBuffer(1.f, p);//pass
					hi[p] = true;
				} else {
					out = getBuffer(low, p);//low
				}
			}

			// OUTS
			outputs[HI].setVoltage(hi[p] ? 10.f : 0.f, p);//trigger out sort of
			outputs[OUT].setVoltage(out, p);
		}
	}
};

//geometry edit
#define HP 3
#define LANES 1
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

struct TWidget : ModuleWidget {
	TWidget(T* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/T.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(loc(1, 1), module, T::NOTE));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 2), module, T::FINE));

		addInput(createInputCentered<PJ301MPort>(loc(1, 3), module, T::IN));
		addInput(createInputCentered<PJ301MPort>(loc(1, 4), module, T::TRIG));

		addOutput(createOutputCentered<PJ301MPort>(loc(1, 5), module, T::HI));
		addOutput(createOutputCentered<PJ301MPort>(loc(1, 6), module, T::OUT));
	}
};


Model* modelT = createModel<T, TWidget>("T");