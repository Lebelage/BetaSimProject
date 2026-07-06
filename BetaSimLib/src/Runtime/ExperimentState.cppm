//
// Created by Sonora on 26.06.2026.
//
export module BetaSimLib.Runtime.ExperimentState;

import std;
import BetaSimLib.Models.Experiment;

export namespace BetaSimLib::Runtime {

    class ExperimentState {
    public:
        void SetConfig(std::shared_ptr<const Models::BaseExperimentConfig> newConfig) {
            std::lock_guard lock(mutex);
            config = std::move(newConfig);
        }

        std::shared_ptr<const Models::BaseExperimentConfig> GetConfig() const {
            std::lock_guard lock(mutex);
            return config;
        }

    private:
        mutable std::mutex mutex;
        std::shared_ptr<const Models::BaseExperimentConfig> config;
    };

}