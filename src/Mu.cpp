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
		configParam(DB, -4.f, 4.f, 0.f, "Exponential Gain", " 6dB");
		configParam(HZ, -4.f, 4.f, 0.f, "Slew LPF", " Oct");
		configParam(LAM, -4.f, 4.f, 0.f, "Halflife", " per Oct");
		configParam(G1, -2.f, 2.f, 0.f, "Gain");
		configParam(G2, -2.f, 2.f, 0.f, "Gain");
		configParam(G3, -2.f, 2.f, 0.f, "Gain");
	}

	//obtain mapped control value
    float log(float val, float centre) {
        return powf(2.f, val) * centre;
    }

    /* 1P H(s) = 1 / (s + fb) */
    //ONE POLE FILTER
	float f1, f2, b[PORT_MAX_CHANNELS];

	void setFK1(float fc, float fs) {//fb feedback not k*s denominator
		f1   = tanf(M_PI * fc / fs);
		f2   = 1 / (1 + f1);
	}

	float process1(float in, int p) {
		float out = (f1 * in + b[p]) * f2;
		b[p] = f1 * (in - out) + out;
		return out;//lpf default
	}

	float dif1(float* input) {
		float co1[] = {
			1.5265105587276737e+48f,
			-1.3668387579017067e+49f,
			+5.431248343252391e+49f,
			-1.2560580881508727e+50f,
			+1.8604974291269024e+50f,
			-1.825112874436707e+50f,
			+1.1774257443584765e+50f,
			-4.70911457297244e+49f,
			+9.245318227649387e+48f
		};
		return sum(co1, input, idx + 1)/5.61659602207866e+47f;
	}

	float dif2(float* input) {
		float co1[] = {
			8.145411592862319e+40f,
			-7.24075814611475e+41f,
			+2.8505272093785156e+42f,
			-6.511132716988107e+42f,
			+9.479167232763678e+42f,
			-9.062466169644141e+42f,
			+5.604592622540067e+42f,
			-2.0690575636637124e+42f,
			+3.50991084293707e+41f
		};
		return sum(co1, input, idx + 1)/1.3901620164136395e+40f;
	}

	float dif3(float* input) {
		float co1[] = {
			4.768402943555988e+41f,
			-4.198496441152479e+42f,
			+1.6329646901392421e+43f,
			-3.6714916748050246e+43f,
			+5.231749630181975e+43f,
			-4.851706129693572e+43f,
			+2.866756706165198e+43f,
			-9.882480374551996e+42f,
			+1.5214043014568113e+42f
		};
		return sum(co1, input, idx + 1)/4.762449890092781e+40;
	}

	float int1(float in, float a, float b, float c, float l) {
		//preprocess
		float x = in*in;
		a *= x*0.5f;
		b *= x*in/6.f;
		c *= x*x/24.f;
		in *= l;
		return (accel(in, a, b)+accel(a+in, b, c))*0.5f;
	}

	float accel(float a, float b, float c) {
		//cumulate sum
		b += a;
		c += b;
		//Accel 3 point
		float cb = c - b;
		float cba = cb - (b - a);
		return cba == 0.f ? c : c - (cb * cb) / cba;
	}

	float int2(float in, float a, float b, float c, float l) {
		return accel(a, b, c);
	}

	float int3(float in, float a, float b, float c, float l, float ll) {
		return accel(a, b, c);
	}

	const int mod9[18] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8 };

	float pre[2*PORT_MAX_CHANNELS][9];// pre buffer
	int idx = 0;// buffer current

	float sum(float* c, float* input, int begin = 0, int cycle = 9) {
		float add = 0.f;
#pragma GCC ivdep		
		for(int co = 0; co < cycle; co ++) {//right is most recent
			int idx = mod9[begin + co];
			add += c[co] * input[idx];
		}
		return add;
	}

	void process(const ProcessArgs& args) override {
		// For inputs intended to be used solely for audio, sum the voltages of all channels
		// (e.g. with Port::getVoltageSum())
		// For inputs intended to be used for CV or hybrid audio/CV, use the first channel’s
		// voltage (e.g. with Port::getVoltage())
		// POLY: Port::getPolyVoltage(c)
		float fs = args.sampleRate;
		int maxPort = inputs[HZ].getChannels();
		if(maxPort == 0) maxPort = 1;

		float db = params[DB].getValue();
		float hz = params[HZ].getValue();
		float lam = params[LAM].getValue();

		float g1 = params[G1].getValue();
		float g2 = params[G2].getValue();
		float g3 = params[G3].getValue();

		// PARAMETERS (AND IMPLICIT INS)
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {
			float cvdb = inputs[CVDB].getPolyVoltage(p);
			float cvhz = inputs[CVHZ].getPolyVoltage(p);
			float cvlam = inputs[CVLAM].getPolyVoltage(p);

			float in1 = inputs[IN1].getPolyVoltage(p);
			float in2 = inputs[IN2].getPolyVoltage(p);
			float in3 = inputs[IN3].getPolyVoltage(p);

			float in = in1 + in2 + in3;//add

			cvdb = log(cvdb + db, 1.f);
			cvhz = log(cvhz + hz, dsp::FREQ_C4);
			float cheat = cvlam + lam;
			cvlam = log(cheat, 1.f);
			cheat *= 0.69314718056f;//base e log cheat
			
			cvhz = clamp(cvhz, 0.f, fs * 0.5f);//limit max filter

			in *= cvdb;//gain
			setFK1(cvhz, fs);
			in = process1(in);//LPF

			pre[p][idx] = in;//buffer
			pre[p+PORT_MAX_CHANNELS][idx] = cvlam;//just in case?
			
			float h = dsp::FREQ_C4/cvhz;//inverse of central rate nyquist
			//scale? - V/sample -> normalization of sample rate change
			float si = fs/dsp::FREQ_C4;//doubling rate should double gain
			float f = h * h;
			float out1 = dif1(pre[p])*si;//h
			float out2 = dif2(pre[p])*h*si;//h^2
			float out3 = dif3(pre[p])*f*si;//h^3

			//process endpoint integrals @ cvlam
			float i1 = int1(in, out1, out2, out3, cvlam);
			float i2 = int1(in, out1, out2, out3, cvlam);
			float i3 = int1(in, out1, out2, out3, cvlam, cheat);

			// OUTS
			outputs[D1].setVoltage(clamp(out1, -20.f, 20.f), p);
			outputs[D2].setVoltage(clamp(out2, -20.f, 20.f), p);
			//slew LPF implicated in tweeter distruction!!!
			outputs[D3].setVoltage(clamp(out3, -20.f, 20.f), p);

			outputs[I1].setVoltage(i1, p);
			outputs[I2].setVoltage(i2, p);
			outputs[I3].setVoltage(i3, p);
		}
		idx = mod9[idx + 1];//buffer modulo
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
		addParam(createParamCentered<RoundBlackKnob>(loc(3, 1), module, Mu::LAM));
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