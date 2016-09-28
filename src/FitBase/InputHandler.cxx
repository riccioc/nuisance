// Copyright 2016 L. Pickering, P Stowell, R. Terri, C. Wilkinson, C. Wret

/*******************************************************************************
*    This file is part of NUISANCE.
*
*    NUISANCE is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    NUISANCE is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with NUISANCE.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

#include "InputHandler.h"


//****************************************************************************
InputHandler::InputHandler(std::string handle, std::string infile_name) {
//****************************************************************************
  
  LOG(SAM) << "Creating InputHandler for " << handle << "..." << std::endl;
  LOG(SAM) << " -> [" << infile_name << "]" << std::endl;

  // Initial Setup
  fMaxEvents    = FitPar::Config().GetParI("MAXEVENTS");
  fIsJointInput = false;
  fEvent        = FitEvent();
  fSignalEvent  = BaseFitEvt();
  fInput        = infile_name;
  fName         = handle;

  // Parse Infile to allow enviornmental flags
  fInputFile = ParseInputFile(fInput);
  LOG(SAM) << " -> Type  = " << fInputType << std::endl;
  LOG(SAM) << " -> Input = " << fInputFile << std::endl;

  // Automatically check what sort of event file it is
  if (fInputType.compare("JOINT")){
    fInputRootFile = new TFile(fInputFile.c_str(), "READ");
    
    if (!fInputRootFile || fInputRootFile->IsZombie()){
      ERR(FTL) << "Cannot find InputFile!" << endl;
      throw;
    }
  }
  
  // Setup the handler for each type
  if (!fInputType.compare("NEUT"))
    ReadNeutFile();
  else if (!fInputType.compare("NUWRO"))
    ReadNuWroFile();
  else if (!fInputType.compare("GENIE"))
    ReadGenieFile();
  else if (!fInputType.compare("GiBUU_nu"))
    ReadGiBUUFile(false);
  else if (!fInputType.compare("GiBUU_nub"))
    ReadGiBUUFile(true);
  else if (!fInputType.compare("HIST"))
    ReadHistogramFile();
  else if (!fInputType.compare("BNSPLN"))
    ReadBinSplineFile();
  else if (!fInputType.compare("EVSPLN"))
    ReadEventSplineFile();
  else if (!fInputType.compare("NUANCE"))
    ReadNuanceFile();
  else if (!fInputType.compare("JOINT"))
    ReadJointFile();
  else {
    LOG(FTL) << " -> ERROR: Invalid Event File Type" << std::endl;
    fInputRootFile->ls();
    throw;
  }

  // Setup MaxEvents After setup of ttree
  if (fMaxEvents > 1 && fMaxEvents < fNEvents) {
    LOG(SAM) << " -> Reading only "   << fMaxEvents
	     << " events from total." << std::endl;
    fNEvents = fMaxEvents;
  }

  fFluxList.push_back(fFluxHist);
  fEventList.push_back(this->fEventHist);
  fXSecList.push_back(this->fXSecHist);

  LOG(SAM) << " -> Finished handler initialisation." << std::endl;
  return;
};

//********************************************************************
std::string InputHandler::ParseInputFile(std::string inputstring) {
  //********************************************************************

  // Parse out the input_type
  const int nfiletypes = 8;
  const std::string filetypes[nfiletypes] = {"NEUT",     "NUWRO",    "GENIE",
                                             "EVSPLN",   "JOINT",    "NUANCE",
                                             "GiBUU_nu", "GiBUU_nub"};

  for (int i = 0; i < nfiletypes; i++) {
    std::string temptypes = filetypes[i] + ":";
    if (inputstring.find(temptypes) != std::string::npos) {
      fInputType = filetypes[i];
      inputstring.replace(inputstring.find(temptypes), temptypes.size(), "");
      break;
    }
  }

  // If no input type ERROR!
  if (fInputType.empty()){
    ERR(FTL) << "No input type supplied for InputHandler!" << endl;
    ERR(FTL) << "Problematic Input: " << inputstring << endl;
    throw;
  }
  
  // Parse out envir flags
  const int nfiledir = 5;
  const std::string filedir[nfiledir] = {"NEUT_DIR", "NUWRO_DIR", "GENIE_DIR",
                                         "NUANCE_DIR", "EVSPLN_DIR"};

  for (int i = 0; i < nfiledir; i++) {
    std::string tempdir = "@" + filedir[i];
    if (inputstring.find(tempdir) != std::string::npos) {
      std::string event_folder = FitPar::Config().GetParS(filedir[i]);
      inputstring.replace(inputstring.find(tempdir), tempdir.size(),
                          event_folder);

      break;
    }
  }

  return inputstring;
}

//********************************************************************
bool InputHandler::CanIGoFast() {
//********************************************************************

  if (eventType == 6) {
    return true;
  }
  return false;
}

//********************************************************************
void InputHandler::ReadEventSplineFile() {
//********************************************************************
  LOG(SAM) << " -> Setting up SPLINE inputs" << std::endl;

  // Event Type 7 SPLINES
  fEventType = 6;

  // Get flux histograms NEUT supplies
  fFluxHist  = (TH1D*) fInputRootFile->Get((fName + "_FLUX").c_str());
  fEventHist = (TH1D*) fInputRootFile->Get((fName + "_EVT" ).c_str());
  fXSecHist  = (TH1D*) fInputRootFile->Get((fName + "_XSEC").c_str());

  // Setup Spline Stuff
  fSplineHead = (FitSplineHead*)fInputRootFile->Get((fName + "_splineHead").c_str());
  
  tn = new TChain(Form("%s", (fName + "_splineEvents").c_str()), "");
  tn->Add(Form("%s/%s", fInputFile.c_str(),(fName + "_splineEvents").c_str()));

  // Assign nvect
  fNEvents = tn->GetEntries();
  tn->SetBranchAddress("FitEvent", &fEvent);

  // Load Dial Coeffs into vector
  for (int i = 0; i < fNEvents; i++) {
    tn->GetEntry(i);
    tn->Show(i);
    fAllSplines.push_back(*fEvent.dial_coeff);
  }

  // Set MAXEVENTS CALC Here before we load in splines
  if (fMaxEvents > 1 and fMaxEvents < fNEvents) {
    LOG(SAM) << " -> Reading only " << fMaxEvents
             << " events from total spline events." << std::endl;
    fNEvents = fMaxEvents;
  }

  // Load all the splines into signal memory
  //  for (int i = 0; i < fNEvents; i++){
  //    tn->GetEntry(i);
  //    BaseFitEvt* base_event = (new BaseFitEvt(fEvent));
  //    base_event->fType=6;
  //    fSignalEvents.push_back( base_event );
  //  }

  // Print out what was read in
  LOG(SAM) << " -> Successfully Read SPLINE file" << std::endl;
  if (LOG_LEVEL(SAM)) PrintStartInput();

  int cnt = 1;
  std::list<FitSpline*>::iterator spl_iter =
      this->fSplineHead->SplineObjects.begin();
  for (; spl_iter != this->fSplineHead->SplineObjects.end(); spl_iter++) {
    FitSpline* spl = (*spl_iter);

    LOG(SAM) << " -> Spline " << cnt << ". " << spl->id << " " << spl->form
             << " "
             << "NDIM(" << spl->ndim << ") "
             << "NPAR(" << spl->npar << ") "
             << "PTS(" << spl->points << ") " << std::endl;
    cnt++;
  }
}

//********************************************************************
FitSplineHead* InputHandler::GetSplineHead() {
  //********************************************************************
  return fSplineHead;
}

//********************************************************************
void InputHandler::ReadJointFile() {
  //********************************************************************

  LOG(SAM) << " -> Reading list of inputs from file" << std::endl;
  
  fIsJointInput = true;

  // Parse Input File
  std::string line;
  std::ifstream card(fInputFile.c_str(), ifstream::in);
  std::vector<std::string> input_lines;

  while (std::getline(card, line, '\n')) {
    std::istringstream stream(line);

    // Add normalisation option for second line
    input_lines.push_back(line);

    // Split to get normalisation
  }

  card.close();

  // Loop over input and get the flux files
  // Using a temporary input handler to do this, which is a bit dodge.
  int count_low = 0;
  int temp_type = -1;
  for (UInt_t i = 0; i < input_lines.size(); i++) {
    // Create Temporary InputHandlers inside
    InputHandler* temp_input = new InputHandler(
        std::string(Form("temp_input_%i", i)), input_lines.at(i));

    if (temp_type != temp_input->GetType() and i > 0) {
      ERR(FTL) << " Can't use joint events with mismatched trees yet!"
               << std::endl;
      ERR(FTL) << " Make them all the same type!" << std::endl;
    }

    temp_type = temp_input->GetType();

    TH1D* temp_flux = (TH1D*)temp_input->GetFluxHistogram()->Clone();
    TH1D* temp_evts = (TH1D*)temp_input->GetEventHistogram()->Clone();
    TH1D* temp_xsec = (TH1D*)temp_input->GetXSecHistogram()->Clone();
    int temp_events = temp_input->GetNEvents();

    temp_flux->SetName(
        (fName + "_" + temp_input->GetInputStateString() + "_FLUX")
            .c_str());
    temp_evts->SetName(
        (fName + "_" + temp_input->GetInputStateString() + "_EVT")
            .c_str());
    temp_xsec->SetName(
        (fName + "_" + temp_input->GetInputStateString() + "_XSEC")
            .c_str());

    fFluxList.push_back(temp_flux);
    fEventList.push_back(temp_evts);
    fXSecList.push_back(temp_xsec);

    fJointIndexLow.push_back(count_low);
    fJointIndexHigh.push_back(count_low + temp_events);
    fJointIndexHist.push_back((TH1D*)temp_evts->Clone());

    count_low += temp_events;

    if (i == 0) {
      fFluxHist = (TH1D*)temp_flux->Clone();
      fEventHist = (TH1D*)temp_evts->Clone();
    } else {
      fFluxHist->Add(temp_flux);
      fEventHist->Add(temp_evts);
    }
    std::cout << "Added Input File " << input_lines.at(i) << std::endl
              << " with " << temp_events << std::endl;
  }

  // Now have all correctly normalised histograms all we need to do is setup the
  // TChains

  // Input Assumes all the same type
  std::string tree_name = "";
  if (temp_type == 0)
    tree_name = "neuttree";
  else if (temp_type == 1)
    tree_name = "treeout";

  // Add up the TChains
  tn = new TChain(tree_name.c_str());
  for (UInt_t i = 0; i < input_lines.size(); i++) {
    // PARSE INPUT
    std::cout << "Adding new tchain " << input_lines.at(i) << std::endl;
    std::string temp_file = ParseInputFile(input_lines.at(i));
    tn->Add(temp_file.c_str());
  }

  // Setup Events
  fNEvents = tn->GetEntries();
  if (temp_type == 0) {
#ifdef __NEUT_ENABLED__
    fEventType = 0;
    neut_event = NULL;
    tn->SetBranchAddress("vectorbranch", &neut_event);
    fEvent.SetEventAddress(&neut_event);
#endif
  } else if (temp_type == 1) {
#ifdef __NUWRO_ENABLED__
    fEventType = 1;
    nuwro_event = NULL;
    tn->SetBranchAddress("e", &nuwro_event);
    fEvent.SetEventAddress(&nuwro_event);
#endif
  }

  // Normalise event histogram PDFS for weights
  for (UInt_t i = 0; i < input_lines.size(); i++) {
    TH1D* temp_hist = (TH1D*)fJointIndexHist.at(i)->Clone();
    fJointIndexScale.push_back(
        double(fNEvents) / fEventHist->Integral("width") *
        fJointIndexHist.at(i)->Integral("width") /
        double(fJointIndexHigh.at(i) - fJointIndexLow.at(i)));

    temp_hist->Scale(double(fNEvents) / fEventHist->Integral("width"));
    temp_hist->Scale(fJointIndexHist.at(i)->Integral("width") /
                     double(fJointIndexHigh.at(i)));

    fJointIndexHist.at(i) = temp_hist;
  }

  fEventHist->SetNameTitle((fName + "_EVT").c_str(),
                                (fName + "_EVT").c_str());
  fFluxHist->SetNameTitle((fName + "_FLUX").c_str(),
                               (fName + "_FLUX").c_str());

  return;
}

//********************************************************************
void InputHandler::ReadNeutFile() {
//********************************************************************

#ifdef __NEUT_ENABLED__

  LOG(SAM) << " -> Setting up NEUT inputs" << std::endl;

  // Event Type 0 Neut
  fEventType = kNEUT;

  // Get flux histograms NEUT supplies
  fFluxHist = (TH1D*)fInputRootFile->Get(
      (PlotUtils::GetObjectWithName(fInputRootFile, "flux")).c_str());
  fFluxHist->SetNameTitle((fName + "_FLUX").c_str(),
                               (fName + "; E_{#nu} (GeV)").c_str());

  fEventHist = (TH1D*)fInputRootFile->Get(
      (PlotUtils::GetObjectWithName(fInputRootFile, "evtrt")).c_str());
  fEventHist->SetNameTitle(
      (fName + "_EVT").c_str(),
      (fName + "; E_{#nu} (GeV); Event Rate").c_str());

  fXSecHist = (TH1D*)fEventHist->Clone();
  fXSecHist->Divide(fFluxHist);
  fXSecHist->SetNameTitle(
      (fName + "_XSEC").c_str(),
      (fName + "_XSEC;E_{#nu} (GeV); XSec (1#times10^{-38} cm^{2})")
          .c_str());

  // Read in the file once only
  tn = new TChain("neuttree", "");
  tn->Add(Form("%s/neuttree", fInputFile.c_str()));

  // Assign nvect
  fNEvents = tn->GetEntries();
  neut_event = NULL;
  tn->SetBranchAddress("vectorbranch", &neut_event);

  // Make the custom event read in nvect when calling CalcKinematics
  fEvent.SetEventAddress(&neut_event);

  // Print out what was read in
  LOG(SAM) << " -> Successfully Read NEUT file" << std::endl;
  if (LOG_LEVEL(SAM)) PrintStartInput();

#else
  ERR(FTL) << "ERROR: Invalid Event File Provided" << std::endl;
  ERR(FTL) << "NEUT Input Not Enabled." << std::endl;
  ERR(FTL) << "Rebuild with --enable-neut or check FitBuild.h!" << std::endl;
  exit(-1);
#endif

  return;
}

//********************************************************************
void InputHandler::ReadNuWroFile() {
//********************************************************************

#ifdef __NUWRO_ENABLED__

  LOG(SAM) << " -> Setting up Nuwro inputs" << std::endl;

  // Event Type 1 == NuWro
  fEventType = kNUWRO;

  // Setup the TChain for nuwro event tree
  tn = new TChain("treeout");
  tn->AddFile(fInputFile.c_str());

  // Get entries and nuwro_event
  fNEvents = tn->GetEntries();
  nuwro_event = NULL;
  tn->SetBranchAddress("e", &nuwro_event);
  fEvent.SetEventAddress(&nuwro_event);

  // Check if we have saved an xsec histogram before
  fFluxHist = (TH1D*)fInputRootFile->Get(
      (PlotUtils::GetObjectWithName(fInputRootFile, "FluxHist")).c_str());
  fEventHist = (TH1D*)fInputRootFile->Get(
      (PlotUtils::GetObjectWithName(fInputRootFile, "EvtHist")).c_str());

  // Check if we are forcing plot generation (takes time)
  bool regenFlux = FitPar::Config().GetParB("input.regen_nuwro_plots");
  if (regenFlux)
    LOG(SAM)
        << " -> Forcing NuWro XSec/Flux plots to be generated at the start. "
        << std::endl;

  // Already generated flux and event histograms
  if (fFluxHist and fEventHist and !regenFlux) {
    fXSecHist = (TH1D*)fInputRootFile->Get(
        (PlotUtils::GetObjectWithName(fInputRootFile, "xsec")).c_str());

    fFluxHist->SetNameTitle((fName + "_FLUX").c_str(),
                                 (fName + "_FLUX").c_str());
    fEventHist->SetNameTitle((fName + "_EVT").c_str(),
                                  (fName + "_EVT").c_str());
    fXSecHist->SetNameTitle((fName + "_XSEC").c_str(),
                                 (fName + "_XSEC").c_str());

    // Need to regenerate if not found
  } else {
    LOG(SAM)
        << " -> No NuWro XSec or Flux Histograms found, need to regenerate!"
        << std::endl;

    // Can grab flux histogram from the pars
    tn->GetEntry(0);

    int beamtype = nuwro_event->par.beam_type;

    if (beamtype == 0) {
      std::string fluxstring = nuwro_event->par.beam_energy;
      std::vector<double> fluxvals =
          PlotUtils::FillVectorDFromString(fluxstring, " ");
      int pdg = nuwro_event->par.beam_particle;
      double Elow = double(fluxvals[0]) / 1000.0;
      double Ehigh = double(fluxvals[1]) / 1000.0;

      std::cout << " - Adding new nuwro flux "
                << "pdg: " << pdg << "Elow: " << Elow << "Ehigh: " << Ehigh
                << std::endl;

      fFluxHist = new TH1D("fluxplot", "fluxplot", fluxvals.size() - 2, Elow, Ehigh);
      for (int j = 2; j < fluxvals.size(); j++) {
        cout << j << " " << fluxvals[j] << endl;
        fFluxHist->SetBinContent(j - 1, fluxvals[j]);
      }
    } else if (beamtype == 1) {
      std::string fluxstring = nuwro_event->par.beam_content;

      std::vector<std::string> fluxlines =
          PlotUtils::FillVectorSFromString(fluxstring, "\n");
      for (int i = 0; i < fluxlines.size(); i++) {
        std::vector<double> fluxvals =
            PlotUtils::FillVectorDFromString(fluxlines[i], " ");

        int pdg = int(fluxvals[0]);
        double pctg = double(fluxvals[1]) / 100.0;
        double Elow = double(fluxvals[2]) / 1000.0;
        double Ehigh = double(fluxvals[3]) / 1000.0;

        std::cout << " - Adding new nuwro flux "
                  << "pdg: " << pdg << "pctg: " << pctg << "Elow: " << Elow
                  << "Ehigh: " << Ehigh << std::endl;

        TH1D* fluxplot =
            new TH1D("fluxplot", "fluxplot", fluxvals.size() - 4, Elow, Ehigh);
        for (int j = 4; j < fluxvals.size(); j++) {
          fluxplot->SetBinContent(j + 1, fluxvals[j]);
        }

        if (fFluxHist)
          fFluxHist->Add(fluxplot);
        else
          fFluxHist = (TH1D*)fluxplot->Clone();
      }
    }

    fFluxHist->SetNameTitle("nuwro_flux",
                                 "nuwro_flux;E_{#nu} (GeV); Flux");

    fEventHist = (TH1D*)fFluxHist->Clone();
    fEventHist->Reset();
    fEventHist->SetNameTitle("nuwro_evt", "nuwro_evt");

    fXSecHist = (TH1D*)fFluxHist->Clone();
    fXSecHist->Reset();
    fXSecHist->SetNameTitle("nuwro_xsec", "nuwro_xsec");

    // Start Processing
    LOG(SAM) << " -> Processing NuWro Input Flux for " << fNEvents
             << " events (This can take a while...) " << std::endl;

    double Enu = 0.0;
    double TotXSec = 0.0;
    double totaleventmode = 0.0;
    double totalevents = 0.0;

    // --- loop
    for (int i = 0; i < fNEvents; i++) {
      tn->GetEntry(i);

      if (i % 100000 == 0) cout << " i " << i << std::endl;
      // Get Variables
      Enu = nuwro_event->in[0].E() / 1000.0;
      TotXSec = nuwro_event->weight;

      // Fill a flux and xsec histogram
      fEventHist->Fill(Enu);
      fXSecHist->Fill(Enu, TotXSec);

      // Keep Tally
      totaleventmode += TotXSec;
      totalevents++;
    };

    LOG(SAM) << " -> Flux Processing Loop Finished." << std::endl;

    if (fEventHist->Integral() == 0.0) {
      std::cout << "ERROR NO EVENTS FOUND IN RANGE! " << std::endl;
      exit(-1);
    }

    // Sort out plot scaling
    double AvgXSec = (totaleventmode * 1.0E38 / (totalevents + 0.));
    LOG(SAM) << " -> Average XSec = " << AvgXSec << std::endl;

    fEventHist->Scale(1.0 / fEventHist->Integral());  // Convert to PDF
    fEventHist->Scale(fFluxHist->Integral() *
                           AvgXSec);  // Convert to Proper Event Rate

    fXSecHist->Add(fEventHist);          // Get Event Rate Plot
    fXSecHist->Divide(fFluxHist);  // Make XSec Plot

    // fEventHist = (TH1D*)fFluxHist->Clone();
    // fEventHist->Multiply(fXSecHist);

    // Clear over/underflows incase they mess with integrals later.
    fFluxHist->SetBinContent(0, 0.0);
    fFluxHist->SetBinContent(fFluxHist->GetNbinsX() + 2, 0.0);

    fEventHist->SetBinContent(0, 0.0);
    fEventHist->SetBinContent(fEventHist->GetNbinsX() + 2, 0.0);

    LOG(SAM)
        << " -> Finished making NuWro event plots. Saving them for next time..."
        << std::endl;

    TFile* temp_save_file = new TFile(fInputFile.c_str(), "UPDATE");
    temp_save_file->cd();

    fFluxHist->Write("nuwro_flux", TObject::kOverwrite);
    fEventHist->Write("nuwro_evtrt", TObject::kOverwrite);
    fXSecHist->Write("nuwro_xsec", TObject::kOverwrite);
    temp_save_file->ls();


    temp_save_file->Close();
    delete temp_save_file;

    fFluxHist->SetNameTitle((fName + "_FLUX").c_str(),
                                 (fName + "_FLUX").c_str());
    fEventHist->SetNameTitle((fName + "_EVT").c_str(),
                                  (fName + "_EVT").c_str());
    fXSecHist->SetNameTitle((fName + "_XSEC").c_str(),
                                 (fName + "_XSEC").c_str());
  }

  // Print out what was read in
  LOG(SAM) << " -> Successfully Read NUWRO file" << std::endl;
  if (LOG_LEVEL(SAM)) PrintStartInput();

#else
  ERR(FTL) << "ERROR: Invalid Event File Provided" << std::endl;
  ERR(FTL) << "NuWro Input Not Enabled." << std::endl;
  ERR(FTL) << "Rebuild with --enable-nuwro or check FitBuild.h!" << std::endl;
  exit(-1);
#endif

  return;
}

//********************************************************************
void InputHandler::ReadGenieFile() {
//********************************************************************

#ifdef __GENIE_ENABLED__

  // Event Type 1 NuWro
  fEventType = 5;

  // Open Root File
  LOG(SAM) << "Reading event file " << fInputFile << std::endl;

  // Get flux histograms NEUT supplies
  fFluxHist = (TH1D*)fInputRootFile->Get(
      (PlotUtils::GetObjectWithName(fInputRootFile, "spectrum")).c_str());
  fFluxHist->SetNameTitle((fName + "_FLUX").c_str(),
                               (fName + "; E_{#nu} (GeV)").c_str());

  fEventHist = (TH1D*)fInputRootFile->Get(
      (PlotUtils::GetObjectWithName(fInputRootFile, "spectrum")).c_str());
  fEventHist->SetNameTitle(
      (fName + "_EVT").c_str(),
      (fName + "; E_{#nu} (GeV); Event Rate").c_str());

  fXSecHist = (TH1D*)fInputRootFile->Get(
      (PlotUtils::GetObjectWithName(fInputRootFile, "spectrum")).c_str());
  fXSecHist->SetNameTitle(
      (fName + "_XSEC").c_str(),
      (fName + "; E_{#nu} (GeV); Event Rate").c_str());

  double average_xsec = 0.0;
  int total_events = 0;

  // Setup the TChain for nuwro event tree
  tn = new TChain("gtree");
  tn->AddFile(fInputFile.c_str());

  fNEvents = tn->GetEntries();
  LOG(SAM) << "Number of GENIE Eevents " << tn->GetEntries() << std::endl;
  genie_event = NULL;
  mcrec = NULL;
  //  NtpMCEventRecord * mcrec = 0; tree->SetBranchAddress(gmrec, &mcrec);
  tn->SetBranchAddress("gmcrec", &mcrec);

  fEventHist->Reset();

  // Make the custom event read in nvect when calling CalcKinematics
  fEvent.SetEventAddress(&mcrec);

  LOG(SAM) << "Processing GENIE flux events." << std::endl;
  for (int i = 0; i < fNEvents; i++) {
    tn->GetEntry(i);

    EventRecord& event = *(mcrec->event);
    GHepParticle* neu = event.Probe();

    GHepRecord genie_record = static_cast<GHepRecord>(event);

    //    double xsec = (genie_record.XSec() / (1E-38 * genie::units::cm2));
    double xsec = (1E+38/genie::units::cm2) * (1.0/2.0) * genie_record.XSec();

    average_xsec += xsec;
    total_events += 1;

    fEventHist->Fill(neu->E());
    fXSecHist->Fill(neu->E(), xsec);

    mcrec->Clear();
  }

  average_xsec = average_xsec / (total_events + 0.);
  fEventHist->Scale( average_xsec * fFluxHist->Integral("width") / total_events, "width" );
  fXSecHist = (TH1D*)fEventHist->Clone();
  fXSecHist->Divide(fFluxHist);

  // Set Titles
  fEventHist->SetNameTitle((fName + "_EVT").c_str(),
				(fName + "_EVT;E_{#nu} (GeV); Events (1#times10^{-38})")
				.c_str());
  
  fXSecHist->SetNameTitle((fName + "_XSEC").c_str(),
			       (fName + "_XSEC;E_{#nu} (GeV); XSec (1#times10^{-38} cm^{2})")
			       .c_str());

#else
  ERR(FTL) << "ERROR: Invalid Event File Provided" << std::endl;
  ERR(FTL) << "GENIE Input Not Enabled." << std::endl;
  ERR(FTL) << "Rebuild with --enable-genie or check FitBuild.h!" << std::endl;
  exit(-1);
#endif

  return;
}

//********************************************************************
void InputHandler::ReadGiBUUFile(bool IsNuBarDominant) {
//********************************************************************
#ifdef __GiBUU_ENABLED__
  fEventType = kGiBUU;

  // Open Root File
  LOG(SAM) << "Opening event file " << fInputFile << std::endl;
  TFile* rootFile = new TFile(fInputFile.c_str(), "READ");
  // Get flux histograms NEUT supplies
  TH1D* numuFlux = dynamic_cast<TH1D*>(rootFile->Get("numu_flux"));
  TH1D* numubFlux = dynamic_cast<TH1D*>(rootFile->Get("numub_flux"));

  if (numuFlux) {
    numuFlux = static_cast<TH1D*>(numuFlux->Clone());
    numuFlux->SetDirectory(NULL);
    numuFlux->SetNameTitle(
        (fName + "_numu_FLUX").c_str(),
        (fName + "; E_{#nu} (GeV); #Phi_{#nu} (A.U.)").c_str());
    fFluxList.push_back(numuFlux);
  }
  if (numubFlux) {
    numubFlux = static_cast<TH1D*>(numubFlux->Clone());
    numubFlux->SetDirectory(NULL);
    numubFlux->SetNameTitle(
        (fName + "_numub_FLUX").c_str(),
        (fName + "; E_{#nu} (GeV); #Phi_{#bar{#nu}} (A.U.)")
            .c_str());
    fFluxList.push_back(numubFlux);
  }
  rootFile->Close();

  // Set flux hist to the dominant mode
  fFluxHist = IsNuBarDominant ? numubFlux : numuFlux;

  if (!fFluxHist) {
    ERR(FTL) << "Couldn't find: "
             << (IsNuBarDominant ? "numub_flux" : "numu_flux")
             << " in input file: " << fInputRootFile->GetName() << std::endl;
    exit(1);
  }
  fFluxHist->SetNameTitle(
      (fName + "_FLUX").c_str(),
      (fName + "; E_{#nu} (GeV);" +
       (IsNuBarDominant ? "#Phi_{#bar{#nu}} (A.U.)" : "#Phi_{#nu} (A.U.)"))
          .c_str());
  tn = new TChain("giRooTracker");
  tn->AddFile(fInputFile.c_str());

  fEventHist =
      static_cast<TH1D*>(fFluxHist->Clone((fName + "_EVT").c_str()));
  fEventHist->Reset();
  fNEvents = tn->GetEntries();
  fEventHist->SetBinContent(
      1, double(fNEvents) / fEventHist->GetXaxis()->GetBinWidth(1));

  GiBUUStdHepReader* giRead = new GiBUUStdHepReader();
  giRead->SetBranchAddresses(tn);
  fEvent.SetEventAddress(giRead);
#endif
}

//********************************************************************
void InputHandler::ReadBinSplineFile() {
  //********************************************************************

  // Bin Splines are saved as one event for each histogram bin.
  // So just read in as normal event splines and it'll all get sorted easily.
}

//********************************************************************
void InputHandler::ReadHistogramFile() {
//********************************************************************

  // Convert the raw histogram into a series of events with X variables

  // So we don't have to pass stuff upsteam
}

//********************************************************************
void InputHandler::ReadNuanceFile() {
//********************************************************************

#ifdef __NUANCE_ENABLED__
  // Read in Nuance output ROOT file (converted from hbook)
  LOG(SAM) << " Reading NUANCE " << std::endl;
  fEventType = kNUANCE;

  // Read in NUANCE Tree
  tn = new TChain("h3");
  tn->AddFile(fInputFile.c_str());

  // Get entries and nuwro_event
  fNEvents = tn->GetEntries();
  nuance_event = new NuanceEvent();

  // SetBranchAddress for Nuance
  //  tn->SetBranchAddress("cc",&nuance_event->cc);
  //  tn->SetBranchAddress("bound",&nuance_event->bound);
  tn->SetBranchAddress("neutrino", &nuance_event->neutrino);
  tn->SetBranchAddress("target", &nuance_event->target);
  tn->SetBranchAddress("channel", &nuance_event->channel);
  //  tn->SetBranchAddress("iniQ", &nuance_event->iniQ);
  //  tn->SetBranchAddress("finQ", &nuance_event->finQ);
  //  tn->SetBranchAddress("lepton0", &nuance_event->lepton0);
  //  tn->SetBranchAddress("polar", &nuance_event->polar);
  //  tn->SetBranchAddress("qsq", &nuance_event->qsq);

  //  tn->SetBranchAddress("w", &nuance_event->w);
  //  tn->SetBranchAddress("x",&nuance_event->x);
  //  tn->SetBranchAddress("y",&nuance_event->y);

  tn->SetBranchAddress("p_neutrino", &nuance_event->p_neutrino);
  tn->SetBranchAddress("p_targ", &nuance_event->p_targ);
  //  tn->SetBranchAddress("vertex", &nuance_event->vertex);
  //  tn->SetBranchAddress("start",&nuance_event->start);
  //  tn->SetBranchAddress("depth",&nuance_event->depth);
  // tn->SetBranchAddress("flux",&nuance_event->flux);

  tn->SetBranchAddress("n_leptons", &nuance_event->n_leptons);
  tn->SetBranchAddress("p_ltot", &nuance_event->p_ltot);
  tn->SetBranchAddress("lepton", &nuance_event->lepton);
  tn->SetBranchAddress("p_lepton", &nuance_event->p_lepton);

  tn->SetBranchAddress("n_hadrons", &nuance_event->n_hadrons);
  tn->SetBranchAddress("p_htot", &nuance_event->p_htot);
  tn->SetBranchAddress("hadron", &nuance_event->hadron);
  tn->SetBranchAddress("p_hadron", &nuance_event->p_hadron);
  
  fEvent.SetEventAddress(&nuance_event);

  fFluxHist = new TH1D((fName + "_FLUX").c_str(),
			     (fName + "_FLUX").c_str(), 1, 0.0, 1.0);

  fFluxHist->SetBinContent(1, 1.0);

  fEventHist = new TH1D((fName + "_EVT").c_str(),
			      (fName + "_EVT").c_str(), 1, 0.0, 1.0);
  fEventHist->SetBinContent(1, fNEvents);

#else
  ERR(FTL) << "ERROR: Invalid Event File Provided" << std::endl;
  ERR(FTL) << "NUANCE Input Not Enabled." << std::endl;
  ERR(FTL) << "Rebuild with -DUSE_NUANCE=1!" << std::endl;
  exit(-1);
#endif
}

//********************************************************************
void InputHandler::PrintStartInput() {
  //********************************************************************

  LOG(SAM) << " -> Total events = " << fNEvents << std::endl;
  LOG(SAM) << " -> Energy Range = " << fFluxHist->GetXaxis()->GetXmin() << "-"
           << fFluxHist->GetXaxis()->GetXmax() << " GeV" << std::endl;
  LOG(SAM) << " -> Integrated Flux Hist = "
           << fFluxHist->Integral(0, fFluxHist->GetNbinsX(), "width")
           << std::endl;

  LOG(SAM) << " -> Integrated Event Hist = "
           << fEventHist->Integral(0, fEventHist->GetNbinsX(), "width")
           << std::endl;

  LOG(SAM) << " -> Integrated Inclusive XSec = "
           << (fEventHist->Integral(0, fEventHist->GetNbinsX(), "width") /
               fFluxHist->Integral(0, fFluxHist->GetNbinsX(), "width")) *
                  1E-38
           << std::endl;

  if (fEventType == kEVTSPLINE) return;

  // Get First event info
  tn->GetEntry(0);
  fEvent.CalcKinematics();
  LOG(SAM) << " -> Event 0. Neutrino PDG = " << fEvent.PartInfo(0)->fPID
           << std::endl;
  LOG(SAM) << "             Target A     = " << fEvent.TargetA
           << std::endl;
  LOG(SAM) << "             Target Z     = " << fEvent.TargetZ
           << std::endl;
}

//********************************************************************
std::string InputHandler::GetInputStateString() {
//********************************************************************
  
  tn->GetEntry(0);
  fEvent.CalcKinematics();
  std::ostringstream state;
  state << "T" << fEventType << "_PDG" << fEvent.PartInfo(0)->fPID << "_Z"
        << fEvent.TargetZ << "_A" << fEvent.TargetA;

  return state.str();
}

//********************************************************************
void InputHandler::ReadEvent(unsigned int i) {
  //********************************************************************

  bool using_events =
      (fEventType == kNEUT ||
       fEventType == 5 ||
       fEventType == kNUWRO ||
       fEventType == kEVTSPLINE ||
       fEventType == kNUANCE ||
       fEventType == kGiBUU);

  if (using_events) {
    tn->GetEntry(i);

    if (fEventType != kEVTSPLINE) fEvent.CalcKinematics();

    fEvent.Index = i;
    fEventIndex  = i;
    fEvent.InputWeight = GetInputWeight(i);

  } else {
    GetTreeEntry(i);
  }
}

//********************************************************************
void InputHandler::GetTreeEntry(const Long64_t i) {
//********************************************************************

  if (fEventType != kEVTSPLINE)
    tn->GetEntry(i);
  else
    (*(fEvent.dial_coeff)) = fAllSplines.at(i);

  fEventIndex = i;
  fEvent.InputWeight = GetInputWeight(i);
}

//********************************************************************
double InputHandler::GetInputWeight(const int entry) {
//********************************************************************

  if (fEventType == kGiBUU) {
    return fEvent.InputWeight;
  }

  if (!fIsJointInput) {
    return 1.0;
  }
  double weight = 1.0;

  // Find Histogram
  for (UInt_t j = 0; j < fJointIndexLow.size(); j++) {
    if (entry >= fJointIndexLow.at(j) and entry < fJointIndexHigh.at(j)) {
      weight *= fJointIndexScale.at(j);
      break;
    }
  }

  return weight;
}

//********************************************************************
int InputHandler::GetGenEvents() {
  //********************************************************************

  if (fEventType == 6)
    return fSplineHead->ngen_events;
  else
    return GetNEvents();
}

//********************************************************************
double InputHandler::TotalIntegratedFlux(double low, double high,
                                         std::string intOpt) {
//********************************************************************

  if( fEventType == kGiBUU){
    return 1.0;
  }
  
  int minBin = fFluxHist->GetXaxis()->FindBin(low);
  int maxBin = fFluxHist->GetXaxis()->FindBin(high);

  double integral =
      fFluxHist->Integral(minBin, maxBin + 1, intOpt.c_str());

  return integral;
};

//********************************************************************
double InputHandler::PredictedEventRate(double low, double high,
                                        std::string intOpt) {
//********************************************************************

  int minBin = fFluxHist->GetXaxis()->FindBin(low);
  int maxBin = fFluxHist->GetXaxis()->FindBin(high);

  return fEventHist->Integral(minBin, maxBin + 1, intOpt.c_str());
}
