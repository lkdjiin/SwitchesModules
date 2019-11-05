#include "plugin.hpp"


struct TwoByTwo : Module {
	enum ParamIds {
		RECT3742_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		RECT3744_LIGHT,
		NUM_LIGHTS
	};

	TwoByTwo() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(RECT3742_PARAM, 0.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct TwoByTwoWidget : ModuleWidget {
	TwoByTwoWidget(TwoByTwo* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/2x2Mute.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<RoundBlackKnob>(mm2px(Vec(12.435, 54.716)), module, TwoByTwo::RECT3742_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.326, 38.5)), module, TwoByTwo::INPUT_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.106, 38.5)), module, TwoByTwo::OUTPUT_OUTPUT));

		addChild(createLight<MediumLight<RedLight>>(mm2px(Vec(13.435, 55.716)), module, TwoByTwo::RECT3744_LIGHT));
	}
};


Model* modelTwoByTwo = createModel<TwoByTwo, TwoByTwoWidget>("TwoByTwo");
