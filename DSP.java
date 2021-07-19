//EDIT and strip!!


package uk.co.kring.android.dcs.statics;

import static java.lang.Math.PI;
import static java.lang.Math.expm1;

public class DSPStatic {

    //======================== PUBLIC INTERFACE
    //obtain mapped control values
    public static float lin(int val, float min, float max) {
        float v = ((float)val) / (float)Integer.MAX_VALUE;
        return min + (max - min) * v;
    }

    public static float log(int val, float centre, float octaves) {
        float v = lin(val, -octaves, octaves);
        return (float)Math.pow(2F, v) * centre;
    }

    public static float qk(int val, float octaves) {
        float v = log(val, 1F, -octaves);
        return 1 / v;//return k from Q
    }

    public interface DSP {
        void process(float[] samples);
        void setParams(int[] maxScaled);
        void setRate(int rate);
    }

    /* 2P
           Ghigh * s^2 + Gband * s + Glow
    H(s) = ------------------------------
                 s^2 + k * s + 1
     */
    //TWO POLE PENTA-VARIANT FILTER
    public static class TwoPole implements DSP {
        float fs, f, t, u, k, tf, bl, bb, shelfCentre, lowHi, i;

        public void setRate(int rate) {
            fs = (float)rate;
        }

        public void setFK(float fc, float ks, float inv, float sc, float lh) {
            f   = (float)Math.tan(PI * fc / fs);
            k = ks;
            t   = 1 / (1 + k * f);
            u   = 1 / (1 + t * f * f);
            tf  = t * f;
            shelfCentre = sc;
            i = inv;
            lowHi = lh;
        }

        public void setParams(int[] maxScaled) {
            setFK(log(maxScaled[0], 500F, 4F),
                    qk(maxScaled[1], 4F),
                    lin(maxScaled[2], 0F, 1F),
                    lin(maxScaled[3], 0F, 1F),
                    lin(maxScaled[4], 0F, 1F));
        }

        public void process(float[] samples) {
            for(int i = 0; i < samples.length; ++i) {
                float low = (bl + tf * (bb + f * samples[i])) * u;
                float band = (bb + f * (samples[i] - low)) * t;
                float high = samples[i] - low - k * band;
                bb = band + f * high;
                bl = low  + f * band;
                low = band * (1F - shelfCentre) + low * shelfCentre;
                high = band * (1F - shelfCentre) + high * shelfCentre;
                float lh = low * (1F - lowHi) + high * lowHi;
                float hl = high * (1F - lowHi) + low * lowHi;
                samples[i] = lh * (1F - i) + (samples[i] - hl) * i;//lpf default
            }
        }
    }

    /* 1P H(s) = 1 / (s + fb) */
    //ONE POLE TRI-VARIANT FILTER
    public static class OnePole implements DSP {
        float fs, f, f2, b, lowHi;

        public void setRate(int rate) {
            fs = (float)rate;
        }

        public void setFK(float fc, float lh) {//fb feedback not k*s denominator
            f   = (float)Math.tan(PI * fc / fs);
            f2   = 1 / (1 + f);
            lowHi = lh;
        }

        public void setParams(int[] maxScaled) {
            setFK(log(maxScaled[0], 500F, 4F),
                    lin(maxScaled[1], 0F, 1F));
        }

        public void process(float[] samples) {
            for(int i = 0; i < samples.length; ++i) {
                float out = (f * samples[i] + b) * f2;
                b = f * (samples[i] - out) + out;
                samples[i] = out * (1F - lowHi) + (samples[i] - out) * lowHi;;//lpf default
            }
        }
    }

    /* Perform calculus. Uses the end point integral method.
    An end point integral does not depend on an approximation like the
    Simpson's rule, and just needs estimates of the differentials at the end point.
    Subtracting two end point integrals gives the definite integral.
    Series acceleration means few differentials have to be evaluated unlike other
    integral methods.
     */
    public class Calculus {
        double h;

        public Calculus(double sampleStep) {
            h = sampleStep;
        }

        public double[] differential(double[] input) {
            //coefficients via http://web.media.mit.edu/~crtaylor/calculator.html
            //all done from prospective of sample input[0]
            double[] output = new double[9];
            double t = h;
            output[0] = input[0];
            double co1[] = {105, -960, 3920, -9408, 14700, -15680, 11760, -6720, 2283};
            output[1] = sum(co1, input, 0, 8, 1) / (840 * t);
            t *= h;
            double co2[] = {3267, -29664, 120008, -284256, 435330, -448672, 312984, -138528, 29531};
            output[2] = sum(co2, input, 0, 8, 1) / (5040 * t);
            t *= h;
            double co3[] = {469, -4216, 16830, -39128, 58280, -57384, 36706, -13960, 2403};
            output[3] = sum(co3, input, 0, 8, 1) / (240 * t);
            t *= h;
            double co4[] = {967, -8576, 33636, -76352, 109930, -102912, 61156, -21056, 3207};
            output[4] = sum(co4, input, 0, 8, 1) / (240 * t);
            t *= h;
            double co5[] = {35, -305, 1170, -2581, 3580, -3195, 1790, -575, 81};
            output[5] = sum(co5, input, 0, 8, 1) / (6 * t);
            t *= h;
            double co6[] = {23, -196, 732, -1564, 2090, -1788, 956, -292, 39};
            output[6] = sum(co6, input, 0, 8, 1) / (4 * t);
            t *= h;
            double co7[] = {7, -58, 210, -434, 560, -462, 238, -70, 9};
            output[7] = sum(co7, input, 0, 8, 1) / (2 * t);
            t *= h;
            double co8[] = {1, -8, 28, -56, 70, -56, 28, -8, 1};
            output[8] = sum(co8, input, 0, 8, 1) / (1 * t);
            return output;
        }

        public double future(double[] input) {
            double co1[] = {1, -9, 36, -84, 126, -126, 84, -36, 9};//0th differential in the future
            return sum(co1, input, 0, 8, 1);
        }

        public double sum(double[] c, double[] input, int begin, int end, int step) {
            double residual = 0.0;
            double add = 0.0;
            double temp;
            for(; begin <= end; begin += step) {
                temp = c[begin] * (input[begin]);
                double test = add + (temp + residual);
                residual = (temp + residual) - (test - add);
                add = test;
            }
            return add;
        }

        long tick;

        public void atTick(long now) {
            tick = now;
        }

        double sigma;

        public void setSigma(double sigmaValue) {
            sigma = sigmaValue;
        }

        public void next() {
            tick++;
        }

        public double[] expDecay(double[] input, int begin, int end,
                 int step, boolean splitDistribute) {//from now
            double[] output = new double[input.length];
            long tt = tick - (end - begin) / step;//work out sampled start time
            for(; begin <= end; begin += step) {
                output[begin] = input[begin] * expm1(-(sigma * h * tt)) +
                        (splitDistribute ? 0.0 : input[begin]);
                tt += 1;
            }
            return output;
        }

        public void cumSum(double[] input, int begin, int end, int step) {
            double residual = 0.0;
            double add = 0.0;
            double temp;
            for(; begin <= end; begin += step) {
                temp = input[begin];
                double test = add + (temp + residual);
                residual = (temp + residual) - (test - add);
                input[begin] = add = test;
            }
        }

        /*===================================================================
         * THREE BELOW ACCELERATIONS WORK WITH CUMULATIVE SUMS OF SERIES
         * ================================================================*/

        public boolean seriesAccel(double[] input, int begin, int end, int step) {
            if (begin == end) {
                return true;//convergence extra not possible
            }
            begin += step;
            end -= step;
            double temp;
            double nm1;
            double np1;
            double temp2;
            if (begin > end) {
                input[begin] = (input[begin - 1] + input[begin]) /2.0;
                return true;//convergence extra not possible
            }
            boolean cov = (begin == end);
            for(; begin <= end; begin += step) {
                //Shank's method
                nm1 = input[begin - step];
                np1 = input[begin + step];
                temp = temp2 = (np1 - input[begin]);
                temp *= temp;
                temp2 -= input[begin] - nm1;
                if (temp2 == 0.0) {
                    temp = 0.0;//pass through as no delta
                } else {
                    temp /= temp2;
                }
                input[begin - step] = np1 - temp;
            }
            return cov;
        }

        public double seriesAccelLim(double[] input, int begin, int end, int step) {
            while (!seriesAccel(input, begin, end, step)) {//overwrite
                end -= 2;//two off end
            }
            return input[begin];
        }

        public void preMul(double[] c, double[] input, int begin, int end, int step) {
            for(; begin <= end; begin += step) {
                input[begin] = input[begin] * c[begin];//pre multiply by coefficients
            }
        }

        public void integralPreMul(double[] input) {
            double fact = 1.0;//and sign
            double time = (double) tick * h;
            double xacc = time;
            for (int i = 0; i < 9; ++i) {
                input[i] *= xacc * fact;
                fact /= -(double)i;
                xacc *= time;
            }
        }

        public double integral(double[] input) {//input[9]
            input = differential(input);
            integralPreMul(input);
            cumSum(input, 0, 8, 1);
            return seriesAccelLim(input, 0, 8, 1);
        }

        public double sparseLaplace(double[] input) {
            double[] c = expDecay(input, 0, 8, 1, true);
            double i = integral(input);
            i += integral(c);
            return i;
        }
    }
}