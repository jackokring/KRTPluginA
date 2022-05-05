#pragma once
//some of these not required
#include <widget/FramebufferWidget.hpp>
#include <widget/SvgWidget.hpp>
#include <app/SvgKnob.hpp>
#include <app/SvgPort.hpp>
#include <app/ModuleLightWidget.hpp>
#include <app/SvgSwitch.hpp>
#include <app/SvgScrew.hpp>
#include <asset.hpp>

namespace componentlibrary {

using namespace window;

////////////////////
// Color scheme
////////////////////

//static const NVGcolor SCHEME_RED = nvgRGB(0xed, 0x2c, 0x24);
//static const NVGcolor SCHEME_GREEN = nvgRGB(0x90, 0xc7, 0x3e);
//static const NVGcolor SCHEME_BLUE = nvgRGB(0x29, 0xb2, 0xef);

////////////////////
// Lights
////////////////////

template <typename TBase = app::ModuleLightWidget>
struct KTSvgLight : TBase {
	widget::FramebufferWidget* fb;
	widget::SvgWidget* sw;

	KTSvgLight() {
		fb = new widget::FramebufferWidget;
		this->addChild(fb);

		sw = new widget::SvgWidget;
		fb->addChild(sw);
	}

	void setSvg(std::shared_ptr<Svg> svg) {
		sw->setSvg(svg);
		fb->box.size = sw->box.size;
		this->box.size = sw->box.size;
	}
};
using KSvgLight = KTSvgLight<>;

template <typename TBase = app::ModuleLightWidget>
struct KTGrayModuleLightWidget : TBase {
	KTGrayModuleLightWidget() {
		this->bgColor = nvgRGBA(0x33, 0x33, 0x33, 0xff);
		this->borderColor = nvgRGBA(0, 0, 0, 53);
	}
};
using KGrayModuleLightWidget = KTGrayModuleLightWidget<>;

/** Reads two adjacent lightIds, so `lightId` and `lightId + 1` must be defined */
template <typename TBase = KGrayModuleLightWidget>
struct KTGreenRedLight : TBase {
	KTGreenRedLight() {
		this->addBaseColor(SCHEME_GREEN);
		this->addBaseColor(SCHEME_RED);
	}
};
using KGreenRedLight = KTGreenRedLight<>;

template <typename TBase = KGrayModuleLightWidget>
struct KTRedGreenBlueLight : TBase {
	KTRedGreenBlueLight() {
		this->addBaseColor(SCHEME_RED);
		this->addBaseColor(SCHEME_GREEN);
		this->addBaseColor(SCHEME_BLUE);
	}
};
using KRedGreenBlueLight = KTRedGreenBlueLight<>;

/** Based on the size of 2mm LEDs */
template <typename TBase>
struct KSmallLight : KTSvgLight<TBase> {
	KSmallLight() {
		this->setSvg(Svg::load(asset::plugin(pluginInstance, "res/SmallLight.svg")));
	}
};

template <typename TBase = KGrayModuleLightWidget>
struct KTGreenLight : TBase {
	KTGreenLight() {
		this->addBaseColor(SCHEME_GREEN);
	}
};
using KGreenLight = KTGreenLight<>;

////////////////////
// Knobs
////////////////////

struct KRoundKnob : app::SvgKnob {
	widget::SvgWidget* bg;

	KRoundKnob() {
		minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;

		bg = new widget::SvgWidget;
		fb->addChildBelow(bg, tw);
	}
};

struct KRoundBlackKnob : KRoundKnob {
	KRoundBlackKnob() {
		setSvg(Svg::load(asset::plugin(pluginInstance, "res/RoundBlackKnob.svg")));
		bg->setSvg(Svg::load(asset::plugin(pluginInstance, "res/RoundBlackKnob_bg.svg")));
	}
};

struct KRoundBlackSnapKnob : KRoundBlackKnob {
	KRoundBlackSnapKnob() {
		snap = true;
	}
};

////////////////////
// Ports
////////////////////

struct KPJ301MPort : app::SvgPort {
	KPJ301MPort() {
		setSvg(Svg::load(asset::plugin(pluginInstance, "res/PJ301M.svg")));
	}
};


////////////////////
// Switches
////////////////////

struct KVCVBezel : app::SvgSwitch {
	KVCVBezel() {
		momentary = true;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/VCVBezel.svg")));
	}
};
using KLEDBezel = KVCVBezel;

template <typename TLightBase = WhiteLight>
struct KVCVLightBezel : VCVBezel {
	app::ModuleLightWidget* light;

	KVCVLightBezel() {
		light = new VCVBezelLight<TLightBase>;
		// Move center of light to center of box
		light->box.pos = box.size.div(2).minus(light->box.size.div(2));
		addChild(light);
	}

	app::ModuleLightWidget* getLight() {
		return light;
	}
};
template <typename TLightBase = WhiteLight>
using KLEDLightBezel = KVCVLightBezel<TLightBase>;

////////////////////
// Misc
////////////////////

struct KScrewSilver : app::SvgScrew {
	KScrewSilver() {
		setSvg(Svg::load(asset::plugin(pluginInstance, "res/ScrewSilver.svg")));
	}
};


} // namespace componentlibrary
