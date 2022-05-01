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

namespace krtplugin {

using namespace window;

////////////////////
// Color scheme
////////////////////

static const NVGcolor SCHEME_RED = nvgRGB(0xed, 0x2c, 0x24);
static const NVGcolor SCHEME_GREEN = nvgRGB(0x90, 0xc7, 0x3e);
static const NVGcolor SCHEME_BLUE = nvgRGB(0x29, 0xb2, 0xef);

////////////////////
// Lights
////////////////////

template <typename TBase = app::ModuleLightWidget>
struct TSvgLight : TBase {
	widget::FramebufferWidget* fb;
	widget::SvgWidget* sw;

	TSvgLight() {
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
using SvgLight = TSvgLight<>;

template <typename TBase = app::ModuleLightWidget>
struct TGrayModuleLightWidget : TBase {
	TGrayModuleLightWidget() {
		this->bgColor = nvgRGBA(0x33, 0x33, 0x33, 0xff);
		this->borderColor = nvgRGBA(0, 0, 0, 53);
	}
};
using GrayModuleLightWidget = TGrayModuleLightWidget<>;

/** Reads two adjacent lightIds, so `lightId` and `lightId + 1` must be defined */
template <typename TBase = GrayModuleLightWidget>
struct TGreenRedLight : TBase {
	TGreenRedLight() {
		this->addBaseColor(SCHEME_GREEN);
		this->addBaseColor(SCHEME_RED);
	}
};
using GreenRedLight = TGreenRedLight<>;

template <typename TBase = GrayModuleLightWidget>
struct TRedGreenBlueLight : TBase {
	TRedGreenBlueLight() {
		this->addBaseColor(SCHEME_RED);
		this->addBaseColor(SCHEME_GREEN);
		this->addBaseColor(SCHEME_BLUE);
	}
};
using RedGreenBlueLight = TRedGreenBlueLight<>;

/** Based on the size of 2mm LEDs */
template <typename TBase>
struct SmallLight : TSvgLight<TBase> {
	SmallLight() {
		this->setSvg(Svg::load(asset::plugin(pluginInstance, "res/SmallLight.svg")));
	}
};

template <typename TBase = GrayModuleLightWidget>
struct TGreenLight : TBase {
	TGreenLight() {
		this->addBaseColor(SCHEME_GREEN);
	}
};
using GreenLight = TGreenLight<>;

////////////////////
// Knobs
////////////////////

struct RoundKnob : app::SvgKnob {
	widget::SvgWidget* bg;

	RoundKnob() {
		minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;

		bg = new widget::SvgWidget;
		fb->addChildBelow(bg, tw);
	}
};

struct RoundBlackKnob : RoundKnob {
	RoundBlackKnob() {
		setSvg(Svg::load(asset::plugin(pluginInstance, "res/RoundBlackKnob.svg")));
		bg->setSvg(Svg::load(asset::plugin(pluginInstance, "res/RoundBlackKnob_bg.svg")));
	}
};

struct RoundBlackSnapKnob : RoundBlackKnob {
	RoundBlackSnapKnob() {
		snap = true;
	}
};

////////////////////
// Ports
////////////////////

struct PJ301MPort : app::SvgPort {
	PJ301MPort() {
		setSvg(Svg::load(asset::plugin(pluginInstance, "res/PJ301M.svg")));
	}
};


////////////////////
// Switches
////////////////////

struct VCVBezel : app::SvgSwitch {
	VCVBezel() {
		momentary = true;
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/VCVBezel.svg")));
	}
};
using LEDBezel = VCVBezel;

template <typename TLightBase = WhiteLight>
struct VCVLightBezel : VCVBezel {
	app::ModuleLightWidget* light;

	VCVLightBezel() {
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
using LEDLightBezel = VCVLightBezel<TLightBase>;

////////////////////
// Misc
////////////////////

struct ScrewSilver : app::SvgScrew {
	ScrewSilver() {
		setSvg(Svg::load(asset::plugin(pluginInstance, "res/ScrewSilver.svg")));
	}
};

}
