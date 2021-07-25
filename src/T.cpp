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
			head[p] = tail[p] = 0.f;
			hi[p] = false;
			buff.resize(MAX_BUFFER);
			len[p] = 4096.f;//an initial test
		}
	}

	const float rate = 44100.f;
	const float chans = 16.f;
	const float time = 32.f;

	//buffer length
	const float MAX_BUFFER = rate * chans * time;
	const float initPos = 0.f;
	float head[PORT_MAX_CHANNELS];
	float tail[PORT_MAX_CHANNELS];
	float maxLen;
	float buffStart[PORT_MAX_CHANNELS];

	//state control
	bool hi[PORT_MAX_CHANNELS];
	bool wait[PORT_MAX_CHANNELS];//wait for state to trigger
	float lenL[PORT_MAX_CHANNELS];
	float len[PORT_MAX_CHANNELS];

	//obtain mapped control value
    float log(float val) {
        return powf(2.f, val);
    }

	dsp::SchmittTrigger st[PORT_MAX_CHANNELS];
	std::vector<float> buff;

	float modulo(float x, float m) {
		float div = x / m;
		long d = (long) div;
		float rem = x - d * m;
		return rem;
	}

	bool putBuffer(float in, int chan) {
		float where = head[chan];
		head[chan] += 1.f;
		where = modulo(where, maxLen);//modulo
		where += buffStart[chan];
		long w = (long) where;//get an integer index
		float where2 = tail[chan] + maxLen - 2.f;//trailing tail overview
		where2 = modulo(where2, maxLen);//modulo
		where2 += buffStart[chan];
		long w2 = (long) where2;//get an integer index
		//ERROR!! --fixed by -2 on buffer collide slot 
		if(w == w2) return true;//overflow
		buff[w] = in;
		return false;// no reset
	}

	float interpolate(float a, float b, float ta, float tab) {
		float lena = 1.f - ta;
		if(tab > 1.f || lena <= tab) {
			float lenb = tab - lena;
			return (a * lena + b * lenb) / tab;//normalized
		} else {
			return a;
		}
	}

	float getBuffer(float stepRelative, int chan) {
		float where = tail[chan];
		tail[chan] += stepRelative;
		where = modulo(where, maxLen);//modulo
		where += buffStart[chan];
		long w = (long) where;//get an integer index
		float fp = where - (float) w;//fractional part
		float out = buff[w];
		float where2 = tail[chan];
		where2 = modulo(where2, maxLen);//modulo
		where2 += buffStart[chan];
		w = (long) where;//get an integer index
		float out2 = buff[w];
		return interpolate(out, out2, fp, stepRelative);
	}

	void resetBuffer(float first, int chan) {
		head[chan] = 0.f;
		tail[chan] = 0.f;
		putBuffer(first, chan);//easier implemetation later
	}

	void process(const ProcessArgs& args) override {
				// For inputs intended to be used solely for audio, sum the voltages of all channels
		// (e.g. with Port::getVoltageSum())
		// For inputs intended to be used for CV or hybrid audio/CV, use the first channel’s
		// voltage (e.g. with Port::getVoltage())
		// POLY: Port::getPolyVoltage(c)
		//float fs = args.sampleRate;
		int maxPort = inputs[TRIG].getChannels();
		if(maxPort == 0) maxPort = 1;
		maxLen = MAX_BUFFER / maxPort;//share buffer 
		long max = (long) maxLen;
		maxLen = (float) max;//round down for sample guard

		float note = params[NOTE].getValue()/12.f;
		float fine = params[FINE].getValue()/1200.f;
		note += fine;
		note = log(note);//relative hi note rate
		float low = note * 0.5f;//an octave down
		float n1 = note + 1.f;

		// PARAMETERS (AND IMPLICIT INS)
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {
			buffStart[p] = maxLen * p;//fit 
			float in = inputs[IN].getPolyVoltage(p);
			float trig = inputs[TRIG].getPolyVoltage(p);
			bool trigger = st[p].process(rescale(trig, 0.1f, 2.f, 0.f, 1.f));

			if(trigger || putBuffer(in, p)) { 
				len[p] = head[p];//get written length since trigger
				resetBuffer(in, p);
				hi[p] = false;
			}
			lenL[p] = (note - 1.f) * len[p] / (note - low);//ratio

			float out;
			if(hi[p]) {
				if(tail[p] > len[p]) { 
					out = getBuffer(1.f, p);//pass
					hi[p] = false;
				} else {
					if(tail[p] + n1 > head[p]) tail[p] = head[p] - n1;
					out = getBuffer(note, p);//high
				}
			} else {
				if(tail[p] > lenL[p]) {
					if(tail[p] > len[p]) {
						out = getBuffer(1.f, p);//pass
					} else {
						if(tail[p] + n1 > head[p]) tail[p] = head[p] - n1;
						out = getBuffer(note, p);//pass edge case
						//must not overtake input to buffer
					}
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