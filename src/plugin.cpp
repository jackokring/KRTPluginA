#include "plugin.hpp"

Plugin* pluginInstance;

void init(Plugin* p) {
	pluginInstance = p;

	// Add modules here
	// p->addModel(modelMyModule);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
	p->addModel(modelA);
	p->addModel(modelMu);
	p->addModel(modelT);
	p->addModel(modelL);
	p->addModel(modelD);
	p->addModel(modelR);

	p->addModel(modelOm);
	p->addModel(modelV);
	p->addModel(modelF);
	p->addModel(modelW);
	p->addModel(modelY);
	p->addModel(modelO);

	p->addModel(modelS);
	p->addModel(modelE);
	p->addModel(modelX);

	p->addModel(modelM);
	p->addModel(modelQ);
	p->addModel(modelC);

	p->addModel(modelU);
	p->addModel(modelB);
	p->addModel(modelJ);
	p->addModel(modelK);

	p->addModel(modelH);
	p->addModel(modelI);
	p->addModel(modelG);

	p->addModel(modelZ);
}

#define M_PI_F float(M_PI)
#define M_PI_POW_2 M_PI * M_PI
#define M_PI_POW_3 M_PI_POW_2 * M_PI
#define M_PI_POW_5 M_PI_POW_3 * M_PI_POW_2
#define M_PI_POW_7 M_PI_POW_5 * M_PI_POW_2
#define M_PI_POW_9 M_PI_POW_7 * M_PI_POW_2
#define M_PI_POW_11 M_PI_POW_9 * M_PI_POW_2

float tanpif(float f) {
    // These coefficients don't need to be tweaked for the audio range.
    // Include f multiplication by PI for 0->1 frequency normal %fs
    const float a = 3.333314036e-01 * M_PI_POW_3;
    const float b = 1.333923995e-01 * M_PI_POW_5;
    const float c = 5.33740603e-02 * M_PI_POW_7;
    const float d = 2.900525e-03 * M_PI_POW_9;
    const float e = 9.5168091e-03 * M_PI_POW_11;
    float f2 = f * f;
    return f * (M_PI_F + f2 * (a + f2 * (b + f2 * (c + f2 * (d + f2 * e)))));
}

int maxPoly(Module *m, const int numIn, const int numOut) {
	int poly = 1;
	for(int i = 0; i < numIn; i++) {
		int chan = m->inputs[i].getChannels();
		if(chan > poly) poly = chan;
	}
	// perhaps an optimization on #pragma GCC ivdep is possible
	// knowing the ranged 1 .. 16
	if(poly > PORT_MAX_CHANNELS) poly = PORT_MAX_CHANNELS;
	for(int o = 0; o < numOut; o++) {
		m->outputs[o].setChannels(poly);
	}
	return poly;
}

// leave filters or close coupling of copies

// placement macro (local)
#define locl(x,y) mm2px(Vec(((hp*HP_UNIT) / 2.f / lanes)*(1+2*(x-1)), (HEIGHT*Y_MARGIN)+(R_HEIGHT / 2.f / rungs)*(1+2*(y-1))))

// control populator
void populate(ModuleWidget *m, int hp, int lanes, int rungs, const int ctl[],
							const char *lbl[], const int kind[]) {
	LabelWidget *display;

	m->addChild(createWidget<KScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	m->addChild(createWidget<KScrewSilver>(Vec(m->box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	m->addChild(createWidget<KScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	m->addChild(createWidget<KScrewSilver>(Vec(m->box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	for(int x = 1; x <= lanes; x++) {
		for(int y = 1; y <= rungs; y++) {
			// automatic layout
			const int idx = (x - 1) + lanes * (y - 1);
			if(ctl[idx] == -1) continue;
			display = new LabelWidget(lbl[idx], kind[idx]);
			display->fixCentre(locl(x, y + 0.5f), strlen(lbl[idx]));//chars
			m->addChild(display);
			switch(kind[idx]) {
				case -1:
					m->addInput(createInputCentered<KPJ301MPort>(locl(x, y), m->module, ctl[idx]));
					break;
				case 0: default:
					m->addParam(createParamCentered<KRoundBlackKnob>(locl(x, y), m->module, ctl[idx]));
					break;
				case 1:
					m->addOutput(createOutputCentered<KPJ301MPort>(locl(x, y), m->module, ctl[idx]));
					break;
			}
		}
	}
}