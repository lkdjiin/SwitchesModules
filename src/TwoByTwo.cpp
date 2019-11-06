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
        GROUP1_LIGHT,
        GROUP2_LIGHT,
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
    float fadeInTime;
    float fadeOutTime;
    bool exponentialFade = false; // If not it's linear.

    TwoByTwo() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(MUTE_PARAM, 0.0, 1.0, 0.0, "Mute channels");
        configParam(FADE_PARAM, 0.01, 1.0, PRESET_FADE, "Fade-in/out time");
        configParam(SCALE_PARAM, 0.0, 2.0, 0.0, "Fade-in/out scale");

        onReset();
    }

    void onReset() override {
        lights[RAMP_LIGHT].setBrightness(0.f);
        lights[GROUP1_LIGHT].setBrightness(0.f);
        lights[GROUP2_LIGHT].setBrightness(0.f);
    }

    // We need an initial state. For that matter, HIGH is as good as LOW.
    void onAdd() override {
        if (state != HIGH && state != LOW) {
            state = HIGH;
        }
    }

    void process(const ProcessArgs& args) override {
        setState();

        switch(state) {
            case HIGH:
                lights[MUTE_LIGHT].setBrightness(0.9f);
                lights[GROUP1_LIGHT].setBrightness(0.9f);
                lights[GROUP2_LIGHT].setBrightness(0.f);
                high();
                break;
            case RAMP_UP:
                // Fade in group 1.
                rampUp(args.sampleTime, 0, 1);
                // Fade out group 2.
                rampDown(args.sampleTime, 2, 3);
                break;
            case RAMP_DOWN:
                // Fade out group 1.
                rampDown(args.sampleTime, 0, 1);
                // Fade in group 2.
                rampUp(args.sampleTime, 2, 3);
                break;
            case LOW:
                lights[MUTE_LIGHT].setBrightness(0.f);
                lights[GROUP1_LIGHT].setBrightness(0.f);
                lights[GROUP2_LIGHT].setBrightness(0.9f);
                low();
                break;
            default:
                printf("*** NO STATE :(((\n");
                break;
        }
    }

    void setState() {
        if (muteTrigger.process(params[MUTE_PARAM].getValue() > 0.f)
                || inputs[TRIGGER_MUTE_INPUT].getNormalVoltage(0.f) > 0.f) {
            switch(state) {
                case HIGH:
                    state = RAMP_DOWN;
                    fadeOutTime = rampTime();
                    fadeInTime = 0.f;
                    lights[MUTE_LIGHT].setBrightness(0.f);
                    lights[RAMP_LIGHT].setBrightness(0.9f);
                    break;
                case RAMP_UP:
                    state = RAMP_DOWN;
                    exchangeFades();
                    lights[MUTE_LIGHT].setBrightness(0.f);
                    lights[RAMP_LIGHT].setBrightness(0.9f);
                    break;
                case RAMP_DOWN:
                    state = RAMP_UP;
                    exchangeFades();
                    lights[MUTE_LIGHT].setBrightness(0.9f);
                    lights[RAMP_LIGHT].setBrightness(0.9f);
                    break;
                case LOW:
                    state = RAMP_UP;
                    fadeOutTime = rampTime();
                    fadeInTime = 0.f;
                    lights[MUTE_LIGHT].setBrightness(0.9f);
                    lights[RAMP_LIGHT].setBrightness(0.9f);
                    break;
                default:
                    printf("*** NO STATE WHEN TRIGGERED!\n");
                    break;
            }
        }
    }

    // This is useful when we trigger a RAMP_UP from a RAMP_DOWN state
    // and vice versa. We exchange fade-in and fade-out to (kind of)
    // move backward.
    void exchangeFades() {
        float temp = fadeInTime;
        fadeInTime = fadeOutTime;
        fadeOutTime = temp;
    }

    // The time of the fade-in/out, as set by the user.
    float rampTime() {
        float time = params[FADE_PARAM].getValue();
        int mult = (int) std::round(params[SCALE_PARAM].getValue());
        switch(mult) {
            case 1:
                time *= 10;
                break;
            case 2:
                time *= 100;
                break;
            default:
                break;
        }
        return time;
    }

    // Play group 1, full amplitude.
    void high() {
        if (inputs[IN_INPUTS + 0].isConnected() &&
               outputs[OUT_OUTPUTS + 0].isConnected()) {
            outputs[OUT_OUTPUTS + 0].setVoltage(inputs[IN_INPUTS + 0].getVoltage());
        }
        if (inputs[IN_INPUTS + 1].isConnected() &&
               outputs[OUT_OUTPUTS + 1].isConnected()) {
            outputs[OUT_OUTPUTS + 1].setVoltage(inputs[IN_INPUTS + 1].getVoltage());
        }
    }
    //
    // Play group 2, full amplitude.
    void low() {
        if (inputs[IN_INPUTS + 2].isConnected() &&
               outputs[OUT_OUTPUTS + 2].isConnected()) {
            outputs[OUT_OUTPUTS + 2].setVoltage(inputs[IN_INPUTS + 2].getVoltage());
        }
        if (inputs[IN_INPUTS + 3].isConnected() &&
               outputs[OUT_OUTPUTS + 3].isConnected()) {
            outputs[OUT_OUTPUTS + 3].setVoltage(inputs[IN_INPUTS + 3].getVoltage());
        }
    }

    // Fade-in the 2 inputs of a given group.
    void rampUp(float sampleTime, int channelA, int channelB) {
        fadeInTime += sampleTime;
        float userValue = rampTime();
        float mult = fadeInTime / userValue;
        mult = clamp(mult, 0.f, 1.f);

        if (exponentialFade) {
            mult = rescale(std::pow(50.f, mult), 1.f, 50.f, 0.f, 1.f);
        }

        if (inputs[IN_INPUTS + channelA].isConnected() &&
               outputs[OUT_OUTPUTS + channelA].isConnected()) {
            outputs[OUT_OUTPUTS + channelA].setVoltage(
                    inputs[IN_INPUTS + channelA].getVoltage() * mult);
        }
        if (inputs[IN_INPUTS + channelB].isConnected() &&
               outputs[OUT_OUTPUTS + channelB].isConnected()) {
            outputs[OUT_OUTPUTS + channelB].setVoltage(
                    inputs[IN_INPUTS + channelB].getVoltage() * mult);
        }
        if (fadeInTime >= userValue) {
            if (state == RAMP_UP) {
                state = HIGH;
                lights[RAMP_LIGHT].setBrightness(0.f);
            }
        }
    }

    // Fade-out the 2 inputs of a given group.
    void rampDown(float sampleTime, int channelA, int channelB) {
        fadeOutTime -= sampleTime;
        float userValue = rampTime();
        float mult = fadeOutTime / userValue;
        mult = clamp(mult, 0.f, 1.f);

        if (exponentialFade) {
            mult = rescale(std::pow(50.f, mult), 1.f, 50.f, 0.f, 1.f);
        }

        if (inputs[IN_INPUTS + channelA].isConnected() &&
               outputs[OUT_OUTPUTS + channelA].isConnected()) {
            outputs[OUT_OUTPUTS + channelA].setVoltage(
                    inputs[IN_INPUTS + channelA].getVoltage() * mult);
        }

        if (inputs[IN_INPUTS + channelB].isConnected() &&
               outputs[OUT_OUTPUTS + channelB].isConnected()) {
            outputs[OUT_OUTPUTS + channelB].setVoltage(
                    inputs[IN_INPUTS + channelB].getVoltage() * mult);
        }

        if (fadeOutTime <= 0.f) {
            if (state == RAMP_DOWN) {
                state = LOW;
                lights[RAMP_LIGHT].setBrightness(0.f);
            }
        } else {
            lights[RAMP_LIGHT].setBrightness(mult);
        }
    }

    json_t *dataToJson() override {
        float value;
        value = (state == HIGH || state == RAMP_UP) ? 1 : 0;
        json_t *rootJ = json_object();
        json_object_set_new(rootJ, "state", json_integer(value));
        json_object_set_new(rootJ, "exponentialFade", json_integer((int) exponentialFade));
        return rootJ;
    }

    void dataFromJson(json_t *rootJ) override {
        json_t *stateJ = json_object_get(rootJ, "state");
        if (stateJ) {
            float value = json_integer_value(stateJ);
            if (value == 1) {
                state = HIGH;
            } else {
                state = LOW;
            }
        }

        json_t *exponentialFadeJ = json_object_get(rootJ, "exponentialFade");
        if (exponentialFadeJ) {
            exponentialFade = json_integer_value(exponentialFadeJ);
        }
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
        addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(15.24, 27.0)),
                    module, TwoByTwo::GROUP1_LIGHT));
        addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(15.24, 67.0)),
                    module, TwoByTwo::GROUP2_LIGHT));
    }

    struct TwoByTwoModeItem : MenuItem {
        TwoByTwo *module;
        bool mode;
        void onAction(const event::Action &e) override {
            module->exponentialFade = mode;
        }
        void step() override {
            rightText = (module->exponentialFade == mode) ? "✔" : "";
        }
    };

    void appendContextMenu(Menu *menu) override {
        MenuLabel *spacerLabel = new MenuLabel();
        menu->addChild(spacerLabel);

        TwoByTwo *module = dynamic_cast<TwoByTwo*>(this->module);
        assert(module);

        MenuLabel *themeLabel = new MenuLabel();
        themeLabel->text = "Audio Mode";
        menu->addChild(themeLabel);

        TwoByTwoModeItem *item1 = new TwoByTwoModeItem();
        item1->text = "Exponential";
        item1->module = module;
        item1->mode = true;
        menu->addChild(item1);

        TwoByTwoModeItem *item2 = new TwoByTwoModeItem();
        item2->text = "Linear";
        item2->module = module;
        item2->mode = false;
        menu->addChild(item2);
    }

};


Model* modelTwoByTwo = createModel<TwoByTwo, TwoByTwoWidget>("TwoByTwo");
