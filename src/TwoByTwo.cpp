#include "plugin.hpp"


struct TwoByTwo : Module {

    const float PRESET_FADE = 0.1;

	enum ParamIds {
        MUTE_PARAM,
        FADE_PARAM,
        SCALE_PARAM,
        NUM_PARAMS
	};

	enum InputIds {
		ENUMS(IN_INPUTS, 4),
        TRIGGER_MUTE_INPUT,
		NUM_INPUTS
	};

	enum OutputIds {
		ENUMS(OUT_OUTPUTS, 4),
		NUM_OUTPUTS
	};

	enum LightIds {
        MUTE_LIGHT,
        RAMP_LIGHT,
        NUM_LIGHTS
	};

    enum States {
        HIGH,
        LOW,
        RAMP_UP,
        RAMP_DOWN
    };

    States state;
    dsp::BooleanTrigger muteTrigger;
    float fadeTimeEllapsed;
    bool exponentialFade = true; // If not it's linear.

	TwoByTwo() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(MUTE_PARAM, 0.0, 1.0, 0.0, "Mute channels");
        configParam(FADE_PARAM, 0.01, 1.0, PRESET_FADE, "Fade-in/out time");
        configParam(SCALE_PARAM, 0.0, 2.0, 0.0, "Fade-in/out scale");
	}

	void process(const ProcessArgs& args) override {
	}
};

template <typename BASE>
struct MuteLight : BASE {
    MuteLight() {
        this->box.size = mm2px(Vec(6.f, 6.f));
    }
};

struct TwoByTwoWidget : ModuleWidget {
	TwoByTwoWidget(TwoByTwo* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/2x2Mute.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH,
                        RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH,
                        RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<LEDBezel>(mm2px(Vec(15.24, 42.0)),
                    module, TwoByTwo::MUTE_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 99.5)),
                    module, TwoByTwo::FADE_PARAM));
        addParam(createParamCentered<CKSSThree>(mm2px(Vec(15.24, 112.0)),
                    module, TwoByTwo::SCALE_PARAM));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 22.0)),
                    module, TwoByTwo::IN_INPUTS + 0));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 32.0)),
                    module, TwoByTwo::IN_INPUTS + 1));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 62.0)),
                    module, TwoByTwo::IN_INPUTS + 2));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 72.0)),
                    module, TwoByTwo::IN_INPUTS + 3));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 52.0)),
                    module, TwoByTwo::TRIGGER_MUTE_INPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.86, 22.0)),
                    module, TwoByTwo::OUT_OUTPUTS + 0));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.86, 32.0)),
                    module, TwoByTwo::OUT_OUTPUTS + 1));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.86, 62.0)),
                    module, TwoByTwo::OUT_OUTPUTS + 2));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.86, 72.0)),
                    module, TwoByTwo::OUT_OUTPUTS + 3));

        addChild(createLightCentered<MuteLight<GreenLight>>(mm2px(Vec(15.24, 42.0)),
                    module, TwoByTwo::MUTE_LIGHT));
        addChild(createLightCentered<SmallLight<YellowLight>>(mm2px(Vec(15.24, 91.5)),
                    module, TwoByTwo::RAMP_LIGHT));
	}
};


Model* modelTwoByTwo = createModel<TwoByTwo, TwoByTwoWidget>("TwoByTwo");
