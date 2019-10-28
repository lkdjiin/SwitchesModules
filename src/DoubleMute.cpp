#include "plugin.hpp"

struct DoubleMute : Module {

    const float PRESET_FADE = 0.1;

    enum ParamIds {
        MUTE_PARAM,
        FADE_IN_PARAM,
        FADE_OUT_PARAM,
        SCALE_IN_PARAM,
        SCALE_OUT_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        IN1_INPUT,
        IN2_INPUT,
        TRIGGER_MUTE_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        OUT1_OUTPUT,
        OUT2_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        MUTE_LIGHT,
        RAMP_UP_LIGHT,
        RAMP_DOWN_LIGHT,
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

    DoubleMute() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(MUTE_PARAM, 0.0, 1.0, 0.0, "Mute channels");
        configParam(FADE_IN_PARAM, 0.01, 1.0, PRESET_FADE, "Fade-in time");
        configParam(FADE_OUT_PARAM, 0.01, 1.0, PRESET_FADE, "Fade-out time");
        configParam(SCALE_IN_PARAM, 0.0, 2.0, 0.0, "Fade-in scale");
        configParam(SCALE_OUT_PARAM, 0.0, 2.0, 0.0, "Fade-out scale");

        onReset();
    }

    void onReset() override {
        lights[RAMP_UP_LIGHT].setBrightness(0.f);
        lights[RAMP_DOWN_LIGHT].setBrightness(0.f);
    }

    void process(const ProcessArgs& args) override {
        setState();

        switch(state) {
            case HIGH:
                lights[MUTE_LIGHT].setBrightness(0.9f);
                high();
                break;
            case RAMP_UP:
                rampUp(args.sampleTime);
                break;
            case RAMP_DOWN:
                rampDown(args.sampleTime);
                break;
            case LOW:
            default:
                break;
        }
    }

    void setState() {
        if (muteTrigger.process(params[MUTE_PARAM].getValue() > 0.f)
                || inputs[TRIGGER_MUTE_INPUT].getNormalVoltage(0.f) > 0.f) {
            switch(state) {
                case HIGH:
                    state = RAMP_DOWN;
                    fadeTimeEllapsed = rampDownTime();
                    lights[MUTE_LIGHT].setBrightness(0.f);
                    lights[RAMP_DOWN_LIGHT].setBrightness(0.9f);
                    lights[RAMP_UP_LIGHT].setBrightness(0.f);
                    break;
                case RAMP_UP:
                    state = RAMP_DOWN;
                    fadeTimeEllapsed = rampUpToDownTime();
                    lights[MUTE_LIGHT].setBrightness(0.f);
                    lights[RAMP_DOWN_LIGHT].setBrightness(0.9f);
                    lights[RAMP_UP_LIGHT].setBrightness(0.f);
                    break;
                case RAMP_DOWN:
                    state = RAMP_UP;
                    fadeTimeEllapsed = rampDownToUpTime();
                    lights[MUTE_LIGHT].setBrightness(0.9f);
                    lights[RAMP_DOWN_LIGHT].setBrightness(0.f);
                    lights[RAMP_UP_LIGHT].setBrightness(0.9f);
                    break;
                case LOW:
                default:
                    state = RAMP_UP;
                    fadeTimeEllapsed = 0.f;
                    lights[MUTE_LIGHT].setBrightness(0.9f);
                    lights[RAMP_DOWN_LIGHT].setBrightness(0.f);
                    lights[RAMP_UP_LIGHT].setBrightness(0.9f);
                    break;
            }
        }
    }

    float rampUpToDownTime() {
        float ratio = fadeTimeEllapsed / rampUpTime();
        return rampDownTime() * ratio;
    }

    float rampDownToUpTime() {
        float ratio = fadeTimeEllapsed / rampDownTime();
        return rampUpTime() * ratio;
    }

    float rampDownTime() {
        float time = params[FADE_OUT_PARAM].getValue();
        int mult = (int) std::round(params[SCALE_OUT_PARAM].getValue());
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

    float rampUpTime() {
        float time = params[FADE_IN_PARAM].getValue();
        int mult = (int) std::round(params[SCALE_IN_PARAM].getValue());
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

    void rampUp(float sampleTime) {
        fadeTimeEllapsed += sampleTime;
        float userValue = rampUpTime();
        float mult = fadeTimeEllapsed / userValue;
        mult = clamp(mult, 0.f, 1.f);

        if (exponentialFade) {
            mult = rescale(std::pow(50.f, mult), 1.f, 50.f, 0.f, 1.f);
        }

        if (inputs[IN1_INPUT].isConnected() && outputs[OUT1_OUTPUT].isConnected()) {
            outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage() * mult);
        }
        if (inputs[IN2_INPUT].isConnected() && outputs[OUT2_OUTPUT].isConnected()) {
            outputs[OUT2_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage() * mult);
        }
        if (fadeTimeEllapsed >= userValue) {
            state = HIGH;
            lights[RAMP_UP_LIGHT].setBrightness(0.f);
        } else {
            lights[RAMP_UP_LIGHT].setBrightness(1.f - (mult));
        }
    }

    void high() {
        if (inputs[IN1_INPUT].isConnected() && outputs[OUT1_OUTPUT].isConnected()) {
            outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage());
        }
        if (inputs[IN2_INPUT].isConnected() && outputs[OUT2_OUTPUT].isConnected()) {
            outputs[OUT2_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage());
        }
    }

    void rampDown(float sampleTime) {
        fadeTimeEllapsed -= sampleTime;
        float userValue = rampDownTime();
        float mult = fadeTimeEllapsed / userValue;
        mult = clamp(mult, 0.f, 1.f);

        if (exponentialFade) {
            mult = rescale(std::pow(50.f, mult), 1.f, 50.f, 0.f, 1.f);
        }

        if (inputs[IN1_INPUT].isConnected() && outputs[OUT1_OUTPUT].isConnected()) {
            outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage() * mult);
        }
        if (inputs[IN2_INPUT].isConnected() && outputs[OUT2_OUTPUT].isConnected()) {
            outputs[OUT2_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage() * mult);
        }
        if (fadeTimeEllapsed <= 0.f) {
            state = LOW;
            lights[RAMP_DOWN_LIGHT].setBrightness(0.f);
        } else {
            lights[RAMP_DOWN_LIGHT].setBrightness(mult);
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

struct DoubleMuteWidget : ModuleWidget {
    DoubleMuteWidget(DoubleMute* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/DoubleMute.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH,
                        RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH,
                        RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParamCentered<LEDBezel>(mm2px(Vec(15.24, 32.0)),
                    module, DoubleMute::MUTE_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 68.5)),
                    module, DoubleMute::FADE_IN_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.24, 99.5)),
                    module, DoubleMute::FADE_OUT_PARAM));
        addParam(createParamCentered<CKSSThree>(mm2px(Vec(15.24, 81.0)),
                    module, DoubleMute::SCALE_IN_PARAM));
        addParam(createParamCentered<CKSSThree>(mm2px(Vec(15.24, 112.0)),
                    module, DoubleMute::SCALE_OUT_PARAM));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 22.0)),
                    module, DoubleMute::IN1_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 42.0)),
                    module, DoubleMute::IN2_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 52.0)),
                    module, DoubleMute::TRIGGER_MUTE_INPUT));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.86, 22.0)),
                    module, DoubleMute::OUT1_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.86, 42.0)),
                    module, DoubleMute::OUT2_OUTPUT));

        addChild(createLightCentered<MuteLight<GreenLight>>(mm2px(Vec(15.24, 32.0)),
                    module, DoubleMute::MUTE_LIGHT));
        addChild(createLightCentered<SmallLight<YellowLight>>(mm2px(Vec(22.86, 68.5)),
                    module, DoubleMute::RAMP_UP_LIGHT));
        addChild(createLightCentered<SmallLight<YellowLight>>(mm2px(Vec(22.86, 99.5)),
                    module, DoubleMute::RAMP_DOWN_LIGHT));
    }

    struct DoubleMuteModeItem : MenuItem {
        DoubleMute *module;
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

        DoubleMute *module = dynamic_cast<DoubleMute*>(this->module);
        assert(module);

        MenuLabel *themeLabel = new MenuLabel();
        themeLabel->text = "Audio Mode";
        menu->addChild(themeLabel);

        DoubleMuteModeItem *item1 = new DoubleMuteModeItem();
        item1->text = "Exponential";
        item1->module = module;
        item1->mode = true;
        menu->addChild(item1);

        DoubleMuteModeItem *item2 = new DoubleMuteModeItem();
        item2->text = "Linear";
        item2->module = module;
        item2->mode = false;
        menu->addChild(item2);
    }

};


Model* modelDoubleMute = createModel<DoubleMute, DoubleMuteWidget>("DoubleMute");
