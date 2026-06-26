//
// Created by Sonora on 26.06.2026.
//
export module BetaSimLib.EventDispatcher;
import std;
import BetaSimLib.Utils.Event;
import BetaSimLib.Models.Experiment;

export namespace BetaSimLib::EventDispatcher {
    using namespace BetaSimLib::Utils;
    using namespace BetaSimLib::Models;

    class EventDispatcher {
    public:
        static Event<std::shared_ptr<const BaseExperimentConfig> > &GetExperimentConfigReadyEvent() { return ExperimentConfigReadyEvent; };
    private:
        inline static Event<std::shared_ptr<const BaseExperimentConfig>> ExperimentConfigReadyEvent;
    };
}
