#include "samples/ISample.hxx"

#include "core/FullEvent.hxx"
#include "core/InputManager.hxx"

#include <iostream>
#include <limits>

using namespace nuis::core;

class NuisToNuWro : public ISample {
public:
  InputManager::Input_id_t fIH_id;

  NuisToNuWro()
      : fIH_id(std::numeric_limits<InputManager::Input_id_t>::max()) {}

  void Initialize(fhicl::ParameterSet const &ps) {
    fIH_id = InputManager::Get().EnsureInputLoaded(ps);
  }

  void ProcessEvent(FullEvent const &ps) {
    std::cout << ps.fNuWroEvent->dyn << std::endl;
  }

  void ProcessSample(size_t nmax) {
    if (fIH_id == std::numeric_limits<InputManager::Input_id_t>::max()) {
      throw uninitialized_ISample();
    }

    IInputHandler const &IH = InputManager::Get().GetInputHandler(fIH_id);
    size_t n = 0;
    for (auto const &fe : IH) {
      if (++n > nmax) {
        break;
      }
      ProcessEvent(fe);
    }
  }

  void Write() {}
  std::string Name() { return "NuisToNuWro"; }
};

DECLARE_PLUGIN(ISample, NuisToNuWro);