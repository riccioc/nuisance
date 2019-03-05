#include "samples/IEventProcessor.hxx"

#include "event/FullEvent.hxx"

#include "input/InputManager.hxx"

#include "utility/FullEventUtility.hxx"
#include "utility/ROOTUtility.hxx"

#include "fhiclcpp/ParameterSet.h"

#include "generator/utility/NuWroUtility.hxx"

#include <iostream>
#include <limits>

using namespace nuis::event;
using namespace nuis::input;
using namespace nuis::utility;
using namespace nuis::nuwrotools;

class NuisToNuWro : public IEventProcessor {
public:
  InputManager::Input_id_t fIH_id;

  TreeFile fOutputTree;

  NuWroEvent *fOutputEvent;

  NuisToNuWro()
      : fIH_id(std::numeric_limits<InputManager::Input_id_t>::max()),
        fOutputTree(), fOutputEvent(nullptr) {}

  void Initialize(fhicl::ParameterSet const &ps) {
    fIH_id = InputManager::Get().EnsureInputLoaded(ps);

    fOutputTree =
        MakeNewTTree(ps.get<std::string>("output_file"), "treeout", "RECREATE");
    fOutputTree.tree->Branch("e", &fOutputEvent);
  }

  void ProcessEvent(FullEvent const &ps) {

    fOutputEvent->in.clear();
    fOutputEvent->out.clear();
    fOutputEvent->post.clear();

    std::pair<NuWroFlags, int> NuMode = GetFlagsDynEquivalent(ps.mode);

    fOutputEvent->flag = NuMode.first;
    fOutputEvent->dyn = NuMode.second;

    for (Particle const &part : GetISParticles(ps)) {
      particle nuwro_part(part.pdg, part.P4.M());
      nuwro_part.set_momentum(vect(part.P4.X(), part.P4.Y(), part.P4.Z()));
      fOutputEvent->in.push_back(nuwro_part);
    }

    for (Particle const &part : GetPrimaryFSParticles(ps)) {
      particle nuwro_part(part.pdg, part.P4.M());
      nuwro_part.set_momentum(vect(part.P4.X(), part.P4.Y(), part.P4.Z()));
      fOutputEvent->out.push_back(nuwro_part);
    }

    for (Particle const &part : GetNuclearLeavingParticles(ps)) {
      particle nuwro_part(part.pdg, part.P4.M());
      nuwro_part.set_momentum(vect(part.P4.X(), part.P4.Y(), part.P4.Z()));
      fOutputEvent->post.push_back(nuwro_part);
    }

    fOutputTree.tree->Fill();
  }

  void ProcessSample(size_t nmax) {
    if (fIH_id == std::numeric_limits<InputManager::Input_id_t>::max()) {
      throw uninitialized_IEventProcessor();
    }

    IInputHandler const &IH = InputManager::Get().GetInputHandler(fIH_id);

    size_t NEvsToProcess = std::min(nmax, IH.GetNEvents());
    size_t NToShout = NEvsToProcess / 10;
    std::cout << "[INFO]: Processing " << NEvsToProcess
              << " input events to NuWro format." << std::endl;

    size_t n = 0;
    for (auto const &fe : IH) {
      if (++n > NEvsToProcess) {
        break;
      }
      if (NToShout && !(n % NToShout)) {
        std::cout << "[INFO]: Processed " << n << "/" << NEvsToProcess
                  << " events." << std::endl;
      }
      ProcessEvent(fe);
    }
  }

  void Write() { fOutputTree.file->Write(); }
  std::string Name() { return "NuisToNuWro"; }
};

DECLARE_PLUGIN(IEventProcessor, NuisToNuWro);