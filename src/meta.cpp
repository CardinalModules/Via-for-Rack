#include "meta.hpp"
#include "via_module.hpp"

#define META_OVERSAMPLE_AMOUNT 8
#define META_OVERSAMPLE_QUALITY 6


struct alignas(32) Meta : Via<META_OVERSAMPLE_AMOUNT, META_OVERSAMPLE_QUALITY> {

    struct SHButtonQuantity : ParamQuantity {

        std::string modes[6] = {"Off", "Sample and track A", "Resample B", "Sample and track A, resample B", "Sample and track A and B", "Resample A and B"};

        float getDisplayValue() override {
            if (!module)
                return Quantity::getDisplayValue();

            Meta * metaModule = (Meta *) module;

            return metaModule->virtualModule.metaUI.button1Mode;
        }

        std::string getDisplayValueString() override {
            return modes[(int) getDisplayValue()];
        }

        std::string getString() override {
            return getDisplayValueString();
        }

    };

    struct TableButtonQuantity : ParamQuantity {

        std::string modes[3][8] = {{"", "", "", "", "", "", "", ""},
                                    {"", "", "", "", "", "", "", ""},
                                    {"", "", "", "", "", "", "", ""}};

        float getDisplayValue() override {
            if (!module)
                return Quantity::getDisplayValue();

            Meta * metaModule = (Meta *) module;

            return metaModule->virtualModule.metaUI.button2Mode;
        }

        // std::string getDisplayValueString() override {
        //     return modes[(int) getDisplayValue()];
        // }

        // std::string getString() override {
        //     return getDisplayValueString();
        // }

    };

    Meta() : Via() {
        
        virtualIO = &virtualModule;



        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(KNOB1_PARAM, 0, 4095.0, 2048.0, "Time/frequency", "", 0.0, 1.0/4095.0);
        configParam(KNOB2_PARAM, 0, 4095.0, 2048.0, "Time/frequency", "", 0.0, 1.0/4095.0);
        configParam(KNOB3_PARAM, 0, 4095.0, 2048.0, "Wave shape", "", 0.0, 1.0/4095.0);
        configParam(B_PARAM, -1.0, 1.0, 0.5, "B input scale");
        configParam(CV2AMT_PARAM, 0, 1.0, 1.0, "Time 2 CV amount");
        configParam(A_PARAM, -5.0, 5.0, 5.0, "A input normal (overriden by patch)");
        configParam(CV3AMT_PARAM, 0, 1.0, 1.0, "Wave shape 2 CV amount");
        
        configParam<SHButtonQuantity>(BUTTON1_PARAM, 0.0, 1.0, 0.0, "");
        configParam<TableButtonQuantity>(BUTTON2_PARAM, 0.0, 1.0, 0.0, "Wavetable up");
        configParam(BUTTON3_PARAM, 0.0, 1.0, 0.0, "Frequency range");
        configParam(BUTTON4_PARAM, 0.0, 1.0, 0.0, "TRIG input configuration");
        configParam<TableButtonQuantity>(BUTTON5_PARAM, 0.0, 1.0, 0.0, "Wavetable down");
        configParam(BUTTON6_PARAM, 0.0, 1.0, 0.0, "Enable/disable looping");
        
        configParam(TRIGBUTTON_PARAM, 0.0, 5.0, 0.0, "Manual trigger");

        onSampleRateChange();
        presetData[0] = virtualModule.metaUI.stockPreset1;
        presetData[1] = virtualModule.metaUI.stockPreset2;
        presetData[2] = virtualModule.metaUI.stockPreset3;
        presetData[3] = virtualModule.metaUI.stockPreset4;
        presetData[4] = virtualModule.metaUI.stockPreset5;
        presetData[5] = virtualModule.metaUI.stockPreset6;
    
    }

    void process(const ProcessArgs &args) override;

    ViaMeta virtualModule;

    void onSampleRateChange() override {
        float sampleRate = APP->engine->getSampleRate();

        ledDecay = 16.0/sampleRate;

        if (sampleRate == 44100.0) {
            divideAmount = 1;
            virtualModule.metaController.audioBaseIncrement = 39562;
            virtualModule.metaController.drumBaseIncrement = 66466;
        } else if (sampleRate == 48000.0) {
            divideAmount = 1;
            virtualModule.metaController.audioBaseIncrement = 36347;
            virtualModule.metaController.drumBaseIncrement = 61065;
        } else if (sampleRate == 88200.0) {
            divideAmount = 2;
            virtualModule.metaController.audioBaseIncrement = 39562;
            virtualModule.metaController.drumBaseIncrement = 66466;
        } else if (sampleRate == 96000.0) {
            divideAmount = 2;
            virtualModule.metaController.audioBaseIncrement = 36347;
            virtualModule.metaController.drumBaseIncrement = 61065;
        } else if (sampleRate == 176400.0) {
            divideAmount = 4;
            virtualModule.metaController.audioBaseIncrement = 39562;
            virtualModule.metaController.drumBaseIncrement = 66466;
        } else if (sampleRate == 192000.0) {
            divideAmount = 4;
            virtualModule.metaController.audioBaseIncrement = 36347;
            virtualModule.metaController.drumBaseIncrement = 61065;
        } else if (sampleRate == 352800.0) {
            divideAmount = 8;
            virtualModule.metaController.audioBaseIncrement = 39562;
            virtualModule.metaController.drumBaseIncrement = 66466;
        } else if (sampleRate == 384000.0) {
            divideAmount = 8;
            virtualModule.metaController.audioBaseIncrement = 36347;
            virtualModule.metaController.drumBaseIncrement = 61065;
        } else if (sampleRate == 705600.0) {
            divideAmount = 16;
            virtualModule.metaController.audioBaseIncrement = 39562;
            virtualModule.metaController.drumBaseIncrement = 66466;
        } else if (sampleRate == 768000.0) {
            divideAmount = 16;
            virtualModule.metaController.audioBaseIncrement = 36347;
            virtualModule.metaController.drumBaseIncrement = 61065;
        }
        
    }

    json_t *dataToJson() override {

        json_t *rootJ = json_object();
        
        // freq
        json_object_set_new(rootJ, "meta_modes", json_integer(virtualModule.metaUI.modeStateBuffer));

        return rootJ;
    }

    int32_t testMode;
    
    void dataFromJson(json_t *rootJ) override {

        json_t *modesJ = json_object_get(rootJ, "meta_modes");
        virtualModule.metaUI.modeStateBuffer = json_integer_value(modesJ);
        virtualModule.metaUI.loadFromEEPROM(0);
        virtualModule.metaUI.recallModuleState();

    }
    
};

void Meta::process(const ProcessArgs &args) {

    clockDivider++;

    if (clockDivider >= divideAmount) {

        // update the "slow IO" (not audio rate) every 16 samples
        // needs to scale with sample rate somehow
        slowIOPrescaler++;
        if (slowIOPrescaler == 16) {
            slowIOPrescaler = 0;
            updateSlowIO();
            virtualModule.slowConversionCallback();
            virtualModule.ui_dispatch(SENSOR_EVENT_SIG);
            virtualModule.metaUI.incrementTimer();
            
            processTriggerButton();
            
            virtualModule.blinkTimerCount += virtualModule.blinkTimerEnable;
            virtualModule.blankTimerCount += virtualModule.blankTimerEnable;
            if (virtualModule.blinkTimerCount > virtualModule.blinkTimerOverflow) {
                virtualModule.blinkTimerCount = 0;
                virtualModule.blinkTimerEnable = 0;
                virtualModule.blankTimerEnable = 1;
                virtualModule.auxTimer1InterruptCallback();
            }
            if (virtualModule.blankTimerCount > virtualModule.blankTimerOverflow) {
                virtualModule.blankTimerCount = 0;
                virtualModule.blankTimerEnable = 0;
                virtualModule.auxTimer2InterruptCallback();
            }

            updateLEDs();

        }

        updateAudioRate();

    }
    
}

// Custom parameter widgets




struct MetaWidget : ModuleWidget  {

    MetaWidget(Meta *module) {

        setModule(module);

    	box.size = Vec(12 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/meta.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParam<ViaSifamBlack>(Vec(9.022 + .753, 30.90), module, Meta::KNOB1_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(68.53 + .753, 30.90), module, Meta::KNOB2_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(68.53 + .753, 169.89), module, Meta::KNOB3_PARAM));
        addParam(createParam<ViaSifamGrey>(Vec(9.022 + .753, 169.89), module, Meta::B_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(128.04 + .753, 30.90), module, Meta::CV2AMT_PARAM));
        addParam(createParam<ViaSifamGrey>(Vec(128.04 + .753, 100.4), module, Meta::A_PARAM));
        addParam(createParam<ViaSifamBlack>(Vec(128.04 + .753, 169.89), module, Meta::CV3AMT_PARAM));
        
        addParam(createParam<TransparentButton>(Vec(10.5 + .753, 80), module, Meta::BUTTON1_PARAM));
        addParam(createParam<TransparentButton>(Vec(47 + .753, 77.5), module, Meta::BUTTON2_PARAM));
        addParam(createParam<TransparentButton>(Vec(85 + .753, 80), module, Meta::BUTTON3_PARAM));
        addParam(createParam<TransparentButton>(Vec(10.5 + .753, 129), module, Meta::BUTTON4_PARAM));
        addParam(createParam<TransparentButton>(Vec(46 + .753, 131.5), module, Meta::BUTTON5_PARAM));
        addParam(createParam<TransparentButton>(Vec(85 + .753, 129), module, Meta::BUTTON6_PARAM));
        
        addParam(createParam<ViaPushButton>(Vec(132.7 + .753, 320), module, Meta::TRIGBUTTON_PARAM));

        addInput(createInput<ViaJack>(Vec(8.07 + 1.053, 241.12), module, Meta::A_INPUT));
        addInput(createInput<ViaJack>(Vec(8.07 + 1.053, 282.62), module, Meta::B_INPUT));
        addInput(createInput<ViaJack>(Vec(8.07 + 1.053, 324.02), module, Meta::MAIN_LOGIC_INPUT));
        addInput(createInput<ViaJack>(Vec(45.75 + 1.053, 241.12), module, Meta::CV1_INPUT));
        addInput(createInput<ViaJack>(Vec(45.75 + 1.053, 282.62), module, Meta::CV2_INPUT));
        addInput(createInput<ViaJack>(Vec(45.75 + 1.053, 324.02), module, Meta::CV3_INPUT));
        addInput(createInput<ViaJack>(Vec(135 + 1.053, 282.62), module, Meta::AUX_LOGIC_INPUT));

        addOutput(createOutput<ViaJack>(Vec(83.68 + 1.053, 241.12), module, Meta::LOGICA_OUTPUT));
        addOutput(createOutput<ViaJack>(Vec(83.68 + 1.053, 282.62), module, Meta::AUX_DAC_OUTPUT));
        addOutput(createOutput<ViaJack>(Vec(83.68 + 1.053, 324.02), module, Meta::MAIN_OUTPUT));
        addOutput(createOutput<ViaJack>(Vec(135 + 1.053, 241.12), module, Meta::AUX_LOGIC_OUTPUT));

        addChild(createLight<MediumLight<WhiteLight>>(Vec(35.8 + .753, 268.5), module, Meta::LED1_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(73.1 + .753, 268.5), module, Meta::LED2_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(35.8 + .753, 309.9), module, Meta::LED3_LIGHT));
        addChild(createLight<MediumLight<WhiteLight>>(Vec(73.1 + .753, 309.9), module, Meta::LED4_LIGHT));
        addChild(createLight<MediumLight<GreenRedLight>>(Vec(54.8 + .753, 179.6), module, Meta::OUTPUT_GREEN_LIGHT));
        addChild(createLight<LargeLight<RGBTriangle>>(Vec(59 + .753, 221), module, Meta::RED_LIGHT));

        }

    void appendContextMenu(Menu *menu) override {
        Meta *module = dynamic_cast<Meta*>(this->module);

        struct MetaAux2ModeHandler : MenuItem {
            Meta *module;
            int32_t logicMode;
            void onAction(const event::Action &e) override {
                module->virtualModule.metaUI.aux2Mode = logicMode;
                module->virtualModule.handleAux2ModeChange(logicMode);
                module->virtualModule.metaUI.storeMode(module->virtualModule.metaUI.aux2Mode, AUX_MODE2_MASK, AUX_MODE2_SHIFT);

            }
        };

        menu->addChild(new MenuEntry);
        menu->addChild(createMenuLabel("Logic out"));
        const std::string logicLabels[] = {
            "High during release",
            "High during attack",
        };
        for (int i = 0; i < (int) LENGTHOF(logicLabels); i++) {
            MetaAux2ModeHandler *aux2Item = createMenuItem<MetaAux2ModeHandler>(logicLabels[i], CHECKMARK(module->virtualModule.metaUI.aux2Mode == i));
            aux2Item->module = module;
            aux2Item->logicMode = i;
            menu->addChild(aux2Item);
        }

        struct MetaAux4ModeHandler : MenuItem {
            Meta *module;
            int32_t signalMode;
            void onAction(const event::Action &e) override {
                module->virtualModule.metaUI.aux4Mode = signalMode;
                module->virtualModule.handleAux4ModeChange(signalMode);
                module->virtualModule.metaUI.storeMode(module->virtualModule.metaUI.aux4Mode, AUX_MODE4_MASK, AUX_MODE4_SHIFT);
            }
        };

        menu->addChild(createMenuLabel("Signal out"));
        const std::string signalLabels[] = {
            "Triangle",
            "Contour",
        };
        for (int i = 0; i < (int) LENGTHOF(signalLabels); i++) {
            MetaAux4ModeHandler *aux4Item = createMenuItem<MetaAux4ModeHandler>(signalLabels[i], CHECKMARK(module->virtualModule.metaUI.aux4Mode == i));
            aux4Item->module = module;
            aux4Item->signalMode = i;
            menu->addChild(aux4Item);
        }

        struct MetaAux1ModeHandler : MenuItem {
            Meta *module;
            int32_t drumMode;
            void onAction(const event::Action &e) override {
                module->virtualModule.metaUI.aux1Mode = drumMode;
                module->virtualModule.metaUI.storeMode(module->virtualModule.metaUI.aux1Mode, AUX_MODE1_MASK, AUX_MODE1_SHIFT);
                if ((module->virtualModule.metaUI.button3Mode | module->virtualModule.metaUI.button6Mode) == 0) {
                    module->virtualModule.handleAux1ModeChange(drumMode);
                }
            }
        };

        menu->addChild(createMenuLabel("Drum signal out"));
        const std::string drumOutLabels[] = {
            "Triangle",
            "Contour",
            "Envelope",
            "Noise"
        };
        for (int i = 0; i < (int) LENGTHOF(drumOutLabels); i++) {
            MetaAux1ModeHandler *aux1Item = createMenuItem<MetaAux1ModeHandler>(drumOutLabels[i], CHECKMARK(module->virtualModule.metaUI.aux1Mode == i));
            aux1Item->module = module;
            aux1Item->drumMode = i;
            menu->addChild(aux1Item);
        }


        struct MetaTuneC4 : MenuItem {
            Meta *module;
            ModuleWidget *moduleWidget;

            int32_t mode;
            void onAction(const event::Action &e) override {

                int32_t audioOsc = !(module->virtualModule.metaUI.button3Mode) && module->virtualModule.metaUI.button6Mode;
                int32_t drumVoice = !(module->virtualModule.metaUI.button3Mode) && !(module->virtualModule.metaUI.button6Mode);

                if (audioOsc) {
                    moduleWidget->params[Meta::KNOB1_PARAM]->paramQuantity->setValue(2048.f);
                    moduleWidget->params[Meta::KNOB2_PARAM]->paramQuantity->setValue(0.f);
                } else if (drumVoice) {
                    moduleWidget->params[Meta::KNOB1_PARAM]->paramQuantity->setValue(4095.f);
                }

            }
        };

        menu->addChild(new MenuEntry);
        MetaTuneC4 *tuneC4 = new MetaTuneC4();
        tuneC4->text = "Tune to C4";
        tuneC4->module = module;
        tuneC4->moduleWidget = this;
        menu->addChild(tuneC4);

        struct PresetRecallItem : MenuItem {
            Meta *module;
            int preset;
            void onAction(const event::Action &e) override {
                module->virtualModule.metaUI.modeStateBuffer = preset;
                module->virtualModule.metaUI.loadFromEEPROM(0);
                module->virtualModule.metaUI.recallModuleState();
            }
        };

        struct StockPresetItem : MenuItem {
            Meta *module;
            Menu *createChildMenu() override {
                Menu *menu = new Menu();
                const std::string presetLabels[] = {
                    "Drum",
                    "Oscillator",
                    "AR Envelope",
                    "Looping AR",
                    "Modulation Sequence",
                    "Complex LFO",
                };
                for (int i = 0; i < (int) LENGTHOF(presetLabels); i++) {
                    PresetRecallItem *item = createMenuItem<PresetRecallItem>(presetLabels[i]);
                    item->module = module;
                    item->preset = module->presetData[i];
                    menu->addChild(item);
                }
                return menu;
            }
        };

        menu->addChild(new MenuEntry);
        StockPresetItem *stockPresets = createMenuItem<StockPresetItem>("Stock presets");
        stockPresets->module = module;
        menu->addChild(stockPresets);

    }

};


Model *modelMeta = createModel<Meta, MetaWidget>("META");

