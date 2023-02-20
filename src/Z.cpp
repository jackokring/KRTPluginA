#include "plugin.hpp"
#include <csignal>

float normalised(float x, float (*fn)(float)) {
	float low = fn(-5.f);
	float high = fn(5.f);
	float actual = fn(x);
	high -= low;
	actual -= low;
	return -5.f + 10.f * actual / high;
}

void fpx_handler(int signal) {
	// do nothing
}

struct Z : Module {
	enum ParamIds {
		// shape and filter
		// LFO as class genera invariat makes 5 parameters enough
		// y^2 + a1.x.y + a3.y = x^3 + a2.x^2 + a4.x + a6
		// p + i.q -> y, solve for x 
		P_PLFO, P_QLFO, P_FLFO, P_WLFO,
		P_FRQ, P_RES, P_A2, P_A4, P_A6,
		P_A, P_R, P_EMOD, P_A1, P_A3,
		// external envelope modulators
		P_SUS, P_EV1, P_EV2,
		NUM_PARAMS
	};

	// names in constructor

	enum InputIds {
		IN_CV,
		// shape and filter modulation
		IN_A1M, IN_A3M, IN_RESM, IN_A2M, IN_A4M, IN_A6M,
		// shape and filter
		// LFO as class genera invariat makes 5 parameters enough
		// y^2 + a1.x.y + a3.y = x^3 + a2.x^2 + a4.x + a6
		// p + i.q -> y, solve for x 
		IN_P, IN_Q,
		IN_HP,
		IN_GATE,
		NUM_INPUTS
	};

	const char *instring[NUM_INPUTS] = {
		"Frequency CV",
		"Modulation x.y", "Modulation y", "Modulation resonance", "Modulation x^2", "Modulation x", "Modulation constant",
		"Py", "Qy",
		"HPF Frequency",
		"Gate"
	};

	enum OutputIds {
		// shape and filter
		// LFO as class genera invariat makes 5 parameters enough
		// y^2 + a1.x.y + a3.y = x^3 + a2.x^2 + a4.x + a6
		// p + i.q -> y, solve for x 
		OUT_PA, OUT_QA, OUT_PB, OUT_QB, OUT_PC, OUT_QC,
		// external envelope modulators
		OUT_EV1, OUT_EV2,
		NUM_OUTPUTS
	};

	const char *outstring[NUM_OUTPUTS] = {
		"Px root 1", "Qx root 1", "Px root 2", "Qx root 2", "Px root 3", "Qx root 3",
		"Envelope out 1", "Envelope out 2"
	};

	enum LightIds {
		NUM_LIGHTS
	};

	const char *lightstring[NUM_LIGHTS] = {

	};

	void iol(bool lights) {
		for(int i = 0; i < NUM_INPUTS; i++) configInput(i, instring[i]);
		for(int i = 0; i < NUM_OUTPUTS; i++) configOutput(i, outstring[i]);
		if(!lights) return;
		for(int i = 0; i < NUM_LIGHTS; i++) configLight(i, lightstring[i]);
	}

	Z() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configParam(P_PLFO, -10.f, 10.f, 0.f, "LFO -> Py");
		configParam(P_QLFO, -10.f, 10.f, 0.f, "LFO -> Qy");
		configParam(P_FLFO, -4.f, 4.f, 0.f, "LFO Frequency", " Oct");
		configParam(P_WLFO, -1.f, 1.f, 0.f, "LFO Wave");

		configParam(P_FRQ, -4.f, 4.f, 0.f, "HPF Frequency", " Oct");
		configParam(P_RES, -6.f, 30.f, -6.f, "Resonance", " dBQ");
		
		configParam(P_A2, -10.f, 10.f, 0.f, "Elliptic x^2 term");
		configParam(P_A4, -10.f, 10.f, 0.f, "Elliptic x term");
		configParam(P_A6, -10.f, 10.f, 0.f, "Elliptic constant term");
		
		// E
		configParam(P_A, -27.f, 9.f, -9.f, "Attack time", " dBs");
		configParam(P_R, -27.f, 9.f, -9.f, "Release time", " dBs");
		configParam(P_EMOD, -6.f, 6.f, 0.f, "LPF modulation level", " Center dB (rel 6)");

		configParam(P_A1, -10.f, 10.f, 0.f, "Elliptic x.y term");
		configParam(P_A3, -10.f, 10.f, 0.f, "Elliptic y term");

		configParam(P_SUS, 0.f, 10.f, 0.f, "Sustain level", " Volts");
		configParam(P_EV1, -6.f, 6.f, 0.f, "Envelope out 1 modulation level", " Center dB (rel 6)");
		configParam(P_EV2, -6.f, 6.f, 0.f, "Envelope out 2 modulation level", " Center dB (rel 6)");

		iol(false);
		// Register floating-point exception handler.
    	signal(SIGFPE, fpx_handler);
	}

	void process(const ProcessArgs& args) override {
		int maxPort = maxPoly(this, NUM_INPUTS, NUM_OUTPUTS);

		// PARAMETERS (AND IMPLICIT INS)
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {

		}
	}
};

//geometry edit
#define HP 13
#define LANES 6
#define RUNGS 7

struct ZWidget : ModuleWidget {

	ZWidget(Z* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Z.svg")));

		const int ctl[] = {
			// -1 is no control and no label
			Z::IN_A1M, Z::IN_A3M, Z::IN_RESM, Z::IN_A2M, Z::IN_A4M, Z::IN_A6M,
			-1, -1, -1, -1, -1, -1,
			Z::IN_P, Z::IN_Q, Z::P_PLFO, Z::P_QLFO, Z::P_FLFO, Z::P_WLFO,
			Z::IN_HP, Z::P_FRQ, Z::P_RES, Z::P_A2, Z::P_A4, Z::P_A6,
			Z::IN_GATE, Z::P_A, Z::P_R, Z::P_EMOD, Z::P_A1, Z::P_A3,
			Z::IN_CV, Z::P_SUS, Z::P_EV1, Z::P_EV2, Z::OUT_EV1, Z::OUT_EV2,
			Z::OUT_PA, Z::OUT_QA, Z::OUT_PB, Z::OUT_QB, Z::OUT_PC, Z::OUT_QC
		};

		const char *lbl[] = {
			// shape and filter modulation
			"A1M", "A3M", "RESM", "A2M", "A4M", "A6M",
			// a nice VCO
			"TEST", "TEST", "TEST", "TEST", "TEST", "TEST",
			// shape and filter
			// LFO as class genera invariat makes 5 parameters enough
			// y^2 + a1.x.y + a3.y = x^3 + a2.x^2 + a4.x + a6
			// p + i.q -> y, solve for x 
			"P", "Q", "PLFO", "QLFO", "FLFO", "WLFO",
			"HP", "FRQ", "RES", "A2", "A4", "A6",
			"GATE", "A", "R", "EMOD", "A1", "A3",
			"CV", "SUS", "ENV1", "ENV2", "ENV1", "ENV2",
			"PA", "QA", "PB", "QB", "PC", "QC"
		};

		const int kind[] = {
			// -1 = sink, +1 = source
			-1, -1, -1, -1, -1, -1,		
			0, 0, 0, 0, 0, 0,	
			-1, -1, 0, 0, 0, 0,
			-1, 0, 0, 0, 0, 0,
			-1, 0, 0, 0, 0, 0,
			-1, 0, 0, 0, 1, 1,
			1, 1, 1, 1, 1, 1
		};

		populate(this, HP, LANES, RUNGS, ctl, lbl, kind);
	}
};

Model* modelZ = createModel<Z, ZWidget>("Z");
