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

	int maxPoly() {
		int poly = 1;
		for(int i = 0; i < NUM_INPUTS; i++) {
			int chan = inputs[i].getChannels();
			if(chan > poly) poly = chan;
		}
		for(int o = 0; o < NUM_OUTPUTS; o++) {
			outputs[o].setChannels(poly);
		}
		return poly;
	}

	//obtain mapped control value
    float log(float val, float centre) {
        return powf(2.f, val) * centre;
    }

	float modulo(float x, float m) {
		float div = x / m;
		long d = (long) div;
		float rem = x - d * m;
		return rem;
	}

	Q() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(OMEGA, -5.f, 5.f, 0.f, "Omega Mass Frequency", " Oct (rel C4)");
		configParam(SINGULAR_HYSTERISIS, -24.f, -6.f, -24.f, "Sigularity Hysterisis", " dB");
		configParam(PLANK, -1.f, 1.f, 1.f, "Uncertainty Plank Magnifier");
		configParam(NEWTON, -1.f, 1.f, 1.f, "Gravity Newton Magnifier");
		configParam(MASS_RATIO, -2.f, 2.f, 0.f, "Mass Magnifier");
		configParam(ANGLE, 0.f, 90.f, 0.f, "Angle of Dark Energy");
		for(int i = 0; i < PORT_MAX_CHANNELS; i++) {
			x[i] = 0.f;//initial follower
			//v[i] = 0.f;
		}
	}

	float wave[PORT_MAX_CHANNELS];
	float x[PORT_MAX_CHANNELS];

	void process(const ProcessArgs& args) override {
		float fs = args.sampleRate;
		int maxPort = maxPoly();

		float omega = params[OMEGA].getValue();
		float hyst = params[SINGULAR_HYSTERISIS].getValue() / 6.f;
		float plank = params[PLANK].getValue();
		float newton = params[NEWTON].getValue();
		float mass = params[MASS_RATIO].getValue();
		float angle = params[ANGLE].getValue() / 90.f;

#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {
			float ihyst = inputs[ISINGULAR_HYSTERISIS].getPolyVoltage(p) * 0.1f;
			ihyst = log(hyst + ihyst, 0.5f);//1 Volt normal
			float iomega = inputs[IOMEGA].getPolyVoltage(p);
			//a simple oscillator, doesn't have to be as is
			float freq = log(iomega + omega, dsp::FREQ_C4);
			float step = freq * 2.f / fs;
			wave[p] += step;
			wave[p] = modulo(wave[p], 2.f);
			float tmp = modulo(wave[p], 1.f);
			float massOsc = tmp * (tmp - 1.f) * (wave[p] - 1.f);
			if(abs(massOsc) >= ihyst) massOsc = ihyst * ihyst / massOsc;//turn to multiplicand stable 
			float iplank = inputs[IPLANK].getPolyVoltage(p) * 0.1f;
			massOsc *= (iplank + plank);
			//basic dr.dv = hbar * massOsc UNCERTAIN GEOMETRY
			//dv = k.F.dt and F = G.M.m/r^2 + k2.r ... for strong running effect too
			//as k2 = 1 and k = 1 can then set G as last constant
			//r can be altered some by angle of radial or hypotenuse tangent
			float in = inputs[IN].getPolyVoltage(p);
			float r = in * 0.1f - x[p];//distance
			float iangle = abs(angle + inputs[IANGLE].getPolyVoltage(p));
			r = sqrtf(r * r + x[p] * x[p] * iangle);//not right ...
			//should do tangential uncertainty increase radius "dark energy" gain
			float inewton = inputs[INEWTON].getPolyVoltage(p) * 0.1f;
			float r2 = r * r;
			massOsc *= r2;//multiply out radial uncertainty "dark matter" gain
			float imass = inputs[IMASS_RATIO].getPolyVoltage(p) * 0.1f;
			imass += mass;
			imass = log(imass, 1.f);
			float f = imass * (r * r2 - (inewton + newton));//implicit r^2 cross multiply
			x[p] += f;
			//x[p] and this is why the crash happened ... ;D
			
			// OUTS
			outputs[OUT].setVoltage(10.f * x[p], p);
		}
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

		addParam(createParamCentered<RoundBlackKnob>(loc(1, 1), module, Q::OMEGA));
		addParam(createParamCentered<RoundBlackKnob>(loc(2, 1), module, Q::SINGULAR_HYSTERISIS));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 2), module, Q::PLANK));
		addParam(createParamCentered<RoundBlackKnob>(loc(2, 2), module, Q::NEWTON));
		addParam(createParamCentered<RoundBlackKnob>(loc(1, 3), module, Q::MASS_RATIO));
		addParam(createParamCentered<RoundBlackKnob>(loc(2, 3), module, Q::ANGLE));

		addInput(createInputCentered<PJ301MPort>(loc(1, 4), module, Q::IOMEGA));
		addInput(createInputCentered<PJ301MPort>(loc(2, 4), module, Q::ISINGULAR_HYSTERISIS));
		addInput(createInputCentered<PJ301MPort>(loc(1, 5), module, Q::IPLANK));
		addInput(createInputCentered<PJ301MPort>(loc(2, 5), module, Q::INEWTON));
		addInput(createInputCentered<PJ301MPort>(loc(1, 6), module, Q::IMASS_RATIO));
		addInput(createInputCentered<PJ301MPort>(loc(2, 6), module, Q::IANGLE));

		addInput(createInputCentered<PJ301MPort>(loc(1, 7), module, Q::IN));
		addOutput(createOutputCentered<PJ301MPort>(loc(2, 7), module, Q::OUT));

	}
};


Model* modelQ = createModel<Q, QWidget>("Q");