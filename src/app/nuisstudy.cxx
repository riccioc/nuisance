#include "config/GlobalConfiguration.hxx"

#include "input/InputManager.hxx"

#include "event/MinimalEvent.hxx"

#include "samples/IEventProcessor.hxx"

#include "plugins/Instantiate.hxx"

#include "exception/exception.hxx"

#include "persistency/ROOTOutput.hxx"

#include "fhiclcpp/make_ParameterSet.h"

#include <string>

NEW_NUIS_EXCEPT(invalid_cli_arguments);

void SayUsage(char const *argv[]) {
  std::cout << "[USAGE]: " << argv[0]
            << "\n"
               "\t-c <config.fcl>             : FHiCL file containing study "
               "configuration. \n"
               "\t-s <sample name>            : FHiCL key of a single sample "
               "to run from the -c argument. \n"
            << std::endl;
}

std::string fhicl_file = "";
std::string named_sample = "";

void handleOpts(int argc, char const *argv[]) {
  int opt = 1;
  while (opt < argc) {
    if ((std::string(argv[opt]) == "-?") ||
        (std::string(argv[opt]) == "--help")) {
      SayUsage(argv);
      exit(0);
    } else if (std::string(argv[opt]) == "-c") {
      fhicl_file = argv[++opt];
    } else if (std::string(argv[opt]) == "-s") {
      named_sample = argv[++opt];
    } else {
      std::cout << "[ERROR]: Unknown option: " << argv[opt] << std::endl;
      SayUsage(argv);
      exit(1);
    }
    opt++;
  }
}

int main(int argc, char const *argv[]) {
  nuis::config::EnsureConfigurationRead("nuis.global.config.fcl");
  nuis::config::EnsureConfigurationRead("nuis.datacomparisons.fcl");

  handleOpts(argc, argv);

  if (!fhicl_file.size()) {
    SayUsage(argv);
    throw invalid_cli_arguments();
  }

  nuis::config::EnsureConfigurationRead(fhicl_file);

  size_t NMax = nuis::config::GetDocument().get<size_t>(
      "nmax", std::numeric_limits<size_t>::max());

  std::vector<fhicl::ParameterSet> samples;
  if (named_sample.size()) {
    samples.push_back(
        nuis::config::GetDocument().get<fhicl::ParameterSet>(named_sample));
  } else {
    samples = nuis::config::GetDocument().get<std::vector<fhicl::ParameterSet>>(
        "samples");
  }

  for (fhicl::ParameterSet const &samp_config : samples) {

    std::cout << "[INFO]: Reading sample: "
              << samp_config.get<std::string>("name") << std::endl;

    nuis::plugins::plugin_traits<IEventProcessor>::unique_ptr_t sample =
        nuis::plugins::Instantiate<IEventProcessor>(
            samp_config.get<std::string>("name"));

    sample->Initialize(samp_config);
    sample->ProcessSample(NMax);
    sample->Write();

    // Ensures no re-use of samples but cleans up the memory.
    nuis::input::InputManager::Get().Clear();
  }

  nuis::persistency::CloseOpenTFiles();
}
