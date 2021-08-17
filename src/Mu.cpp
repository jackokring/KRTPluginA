#include "plugin.hpp"

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

	Mu() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(DB, -24.f, 6.f, 0.f, "Exponential Gain", " dB");
		configParam(HZ, -4.f, 4.f, 0.f, "Slew LPF", " Oct");
		configParam(LAM, -36.f, 0.f, 36.f, "Halflife", " dBs");
		//-infinty centre
		configParam(G1, -6.f, 6.f, 0.f, "Gain", " Center dB (rel 6)");
		configParam(G2, -6.f, 6.f, 0.f, "Gain", " Center dB (rel 6)");
		configParam(G3, -6.f, 6.f, 0.f, "Gain", " Center dB (rel 6)");
	}

	//obtain mapped control value
    float log(float val, float centre) {
        return powf(2.f, val) * centre;
    }

	float dBMid(float val) {
		return powf(2.f, val)-powf(2.f, -val);
	}

	float f, t, u, k, tf, bl[PORT_MAX_CHANNELS][4], bb[PORT_MAX_CHANNELS][4];

    /* 2P
           Ghigh * s^2 + Gband * s + Glow
    H(s) = ------------------------------
                 s^2 + k * s + 1
     */
    //TWO POLE FILTER
	void setFK2(float fc, float fs) {
		//f   = tanf(M_PI * fc / fs);
		f	= tanpif(fc / fs);
		k   = sqrtf(2.f);//butterworth
		t   = 1 / (1 + k * f);
		u   = 1 / (1 + t * f * f);
		tf  = t * f;
	}

	float process2(float in, int p, int i) {
		float low = (bl[p][i] + tf * (bb[p][i] + f * in)) * u;
		float band = (bb[p][i] + f * (in - low)) * t;
		float high = in - low - k * band;
		bb[p][i] = band + f * high;
		bl[p][i] = low  + f * band;
		return low;//lpf default
	}

	float future(float* input) {
		//f_ = (-1*f[i-4]+4*f[i-3]-6*f[i-2]+4*f[i-1])/(1*1.0*h**0)
		float co1[] = {-1.f, 4.f, -6.f, 4.f };//0th differential in the future
		return sum(co1, input, idx + 1);
	}

	float dif1(float* input) {
		//f_x = (-2*f[i-3]+9*f[i-2]-18*f[i-1]+11*f[i+0])/(6*1.0*h**1)
		float co1[] = {
			-2.f, 9.f, -18.f, 11.f
		};
		return sum(co1, input, idx + 1)/6.f;
	}

	float dif2(float* input) {
		//f_xx = (-1*f[i-3]+4*f[i-2]-5*f[i-1]+2*f[i+0])/(1*1.0*h**2)
		float co1[] = {
			-1.f, 4.f, -5.f, 2.f
		};
		return sum(co1, input, idx + 1)/1.f;
	}

	float dif3(float* input) {
		//f_xxx = (-1*f[i-4]+3*f[i-3]-3*f[i-2]+1*f[i-1])/(1*1.0*h**3)
		float co1[] = {
			-1.f, 3.f, -3.f, 1.f
		};
		return sum(co1, input, idx + 1)/1.f;
	}

	float int1(float in, float a, float b, float c, float l) {
		//preprocess
		float x = l*l;
		a *= -x*0.5f;
		b *= x*l/6.f;
		c *= -x*x/24.f;
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
		//preprocess
		float x = l*l;
		a *= -l*x;
		b *= x*x;
		c *= -x*x*l;
		in *= x;
		return (accel(in, a, b)+accel(a+in, b, c))*0.5f;
	}

	float terms2(float k, float k2, float b, float c, float l, float kin) {//float predicate
		//float out = (k+1)!.in^(2+k)*b/(k+2)!;
		//out -= (k+1)!.in^(3+k)*c/(k+3)!;
		float x = l*l;
		return b*k*x*kin-c*x*l*kin*k2;
	}

	/* JUST DOCS
		And on the second better than first of the equality
		of the current avaerage? Guys "n girls?
	*/
	float int3(float in, float a, float b, float c, float l, float ll) {
		//quite complicated as the 1/X has to be integrated to a log
		//so simplification on the multiply defers complexity
		//to the integral of the division in a integral(x*f(x)/x, x)
		float out = in*ll;//the first part (bug fix)
		float secMul = (l-ll);//the second part multiplier
		//second part
		float x = accel(a, -l*b*0.5f, l*l*c/6.f);
		out -= secMul*x;
		//third part nested series
		//inner 2 significat terms
		x = terms2(1.f/2.f, 1.f/6.f, b, c, l, 1.f)*0.5f;//0
		float y = -terms2(1.f/3.f, 2.f/24.f, b, c, l, l)/6.f;//1
		float z = terms2(6.f/24.f, 24.f/120.f, b, c, l, l*l)/24.f;//2
		out += accel(x, y, z);
		return out;
	}

	float pre[5*PORT_MAX_CHANNELS][4];// pre buffer
	int idx = 0;// buffer current

	float sum(float* c, float* input, int begin = 0, int cycle = 4) {
		float add = 0.f;
#pragma GCC ivdep		
		for(int co = 0; co < cycle; co ++) {//right is most recent
			int idx = (begin + co) & 3;
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
		int maxPort = maxPoly();

		//gain in a cube root peak
		float db = params[DB].getValue()/2.f;
		float hz = params[HZ].getValue();
		float lam = params[LAM].getValue()/3.f;

		float g1 = dBMid(params[G1].getValue()/6.f);
		float g2 = dBMid(params[G2].getValue()/6.f);
		float g3 = dBMid(params[G3].getValue()/6.f);

		// added elimination of calculations not necessary for outputs
		// which are not used. Inputs not used are different as
		// controls maybe used as voltage setters for outputs.

		// PARAMETERS (AND IMPLICIT INS)
#pragma GCC ivdep
		for(int p = 0; p < maxPort; p++) {
			float cvdb = inputs[CVDB].getPolyVoltage(p) * 0.1f;
			float cvhz = inputs[CVHZ].getPolyVoltage(p);
			float cvlam = inputs[CVLAM].getPolyVoltage(p);

			float in1 = inputs[IN1].getPolyVoltage(p);
			float in2 = inputs[IN2].getPolyVoltage(p);
			float in3 = inputs[IN3].getPolyVoltage(p);

			float in = in1 * g1 + in2 * g2 + in3 * g3;
			cvdb = log(cvdb + db, 1.f);
			cvhz = log(cvhz + hz, dsp::FREQ_C4);
			float cheat = cvlam + lam;
			//low pass add
			setFK2(cvhz, fs);
			/* JUST DOCS.
			To bring upon the elimination of audiable measure of Gibbs stream Hz measure?
			Counter? Inductives? Yes, to chop top makes a sense. To "cool" a choice?
			Ands of three ands of sampling settings in the compute out of space?
			Hence this is correct for Mu.
			*/
			cheat = process2(cheat, p, 0);//LPF
			cheat = process2(cheat, p, 1);//LPF
			cvlam = log(cheat, 1.f);
			if(outputs[I1].isConnected() || outputs[I2].isConnected()
				|| outputs[I3].isConnected() || outputs[D2].isConnected()
				|| outputs[D3].isConnected()) {
				pre[p+3*PORT_MAX_CHANNELS][idx] = cvlam;
				cvlam = future(pre[p+3*PORT_MAX_CHANNELS]);
			}
			if(outputs[I3].isConnected()) {
				cheat *= 0.69314718056f;//base e log cheat
				pre[p+4*PORT_MAX_CHANNELS][idx] = cheat;
				cheat = future(pre[p+4*PORT_MAX_CHANNELS]);
			}
			
			cvhz = clamp(cvhz, 0.f, fs * 0.5f);//limit max filter

			in *= cvdb;//gain
			in = process2(in, p, 2);//LPF
			in = process2(in, p, 3);//LPF

			if(inputs[IN1].isConnected() || inputs[IN2].isConnected()
				|| inputs[IN3].isConnected()) {
				pre[p][idx] = in;//buffer
				in = future(pre[p]);
			}
			
			float h = dsp::FREQ_C4/cvhz;//inverse of central rate nyquist
			//scale? - V/sample -> normalization of sample rate change
			float si = fs/dsp::FREQ_C4;//doubling rate should double gain
			float f = h * h;
			float i1 = 0.f;
			float out1 = 0.f;
			if(outputs[I1].isConnected() || outputs[D1].isConnected()) {
				out1 = dif1(pre[p])*si;//h
			}
			if(outputs[I1].isConnected()) {
				float out2 = dif2(pre[p])*h*si;//h^2
				float out3 = dif3(pre[p])*f*si;//h^3
				i1 = int1(in, out1, out2, out3, cvlam);
			}

			float inp = 0.f;
			if(outputs[I2].isConnected() || outputs[D2].isConnected()) {
				inp = in/cvlam;
				pre[p+PORT_MAX_CHANNELS][idx] = inp;//just in case?
				inp = future(pre[p+PORT_MAX_CHANNELS]);
			}
			float i2 = 0.f;
			if(outputs[I2].isConnected()) {
				float out1p = dif1(pre[p+PORT_MAX_CHANNELS])*si;//h
				float out2p = dif2(pre[p+PORT_MAX_CHANNELS])*h*si;//h^2
				float out3p = dif3(pre[p+PORT_MAX_CHANNELS])*f*si;//h^3
				i2 = int2(inp, out1p, out2p, out3p, cvlam);
			}

			float inl = 0.f;
			if(outputs[I3].isConnected() || outputs[D3].isConnected()) {
				inl = in*cvlam;
				pre[p+2*PORT_MAX_CHANNELS][idx] = inl;//just in case?
				inl = future(pre[p+2*PORT_MAX_CHANNELS]);
			}
			float i3 = 0.f;
			if(outputs[I3].isConnected()) {
				float out1l = dif1(pre[p+2*PORT_MAX_CHANNELS])*si;//h
				float out2l = dif2(pre[p+2*PORT_MAX_CHANNELS])*h*si;//h^2
				float out3l = dif3(pre[p+2*PORT_MAX_CHANNELS])*f*si;//h^3
				i3 = int3(inl, out1l, out2l, out3l, cvlam, cheat);
			}

			// OUTS
			outputs[D1].setVoltage(cbrtf(out1), p);
			//maybe better modulation
			outputs[D2].setVoltage(cbrtf(inp), p);
			//slew LPF implicated in tweeter distruction!!!
			outputs[D3].setVoltage(cbrtf(inl), p);

			outputs[I1].setVoltage(cbrtf(i1), p);
			outputs[I2].setVoltage(cbrtf(i2), p);
			outputs[I3].setVoltage(cbrtf(i3), p);
		}
		idx = (idx + 1) & 3;//buffer modulo
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