#include "MINERvA_CCNpip_XSec_1Dpmu_nu.h"

// The constructor
MINERvA_CCNpip_XSec_1Dpmu_nu::MINERvA_CCNpip_XSec_1Dpmu_nu(std::string inputfile, FitWeight *rw, std::string  type, std::string fakeDataFile) {

  fName = "MINERvA_CCNpip_XSec_1Dpmu_nu_2016";
  plotTitles = "; p_{#mu} (GeV); d#sigma/dp_{#mu} (cm^{2}/GeV/nucleon)";
  EnuMin = 1.5;
  EnuMax = 10;
  isDiag = false;
  Measurement1D::SetupMeasurement(inputfile, type, rw, fakeDataFile);

  this->SetDataValues(std::string(std::getenv("EXT_FIT"))+"/data/MINERvA/CCNpip/2016_upd/ccnpip_pmu.txt");

  // MINERvA mucked up the scaling in the data-release where everything was bin-width normalised to the first bin, not the nth bin
  double binOneWidth = dataHist->GetBinWidth(1);
  for (int i = 0; i < dataHist->GetNbinsX()+1; i++) {
    double binNWidth = dataHist->GetBinWidth(i+1);
    dataHist->SetBinContent(i+1, dataHist->GetBinContent(i+1)*1E-40);
    dataHist->SetBinError(i+1, dataHist->GetBinContent(i+1)*dataHist->GetBinError(i+1)/100.);
    dataHist->SetBinContent(i+1, dataHist->GetBinContent(i+1)*binOneWidth/binNWidth);
    dataHist->SetBinError(i+1, dataHist->GetBinError(i+1)*binOneWidth/binNWidth);
  }

  // This is a correlation matrix, FIX IT
  this->SetCovarMatrixFromText(std::string(std::getenv("EXT_FIT"))+"/data/MINERvA/CCNpip/2016_upd/ccnpip_pmu_corr.txt", dataHist->GetNbinsX());

  this->SetupDefaultHist();

  scaleFactor = this->eventHist->Integral("width")*double(1E-38)/double(nevents)/TotalIntegratedFlux("width");
};

void MINERvA_CCNpip_XSec_1Dpmu_nu::FillEventVariables(FitEvent *event) {

  TLorentzVector Pnu = (event->PartInfo(0))->fP;
  TLorentzVector Ppip;
  TLorentzVector Pmu;

  // Loop over the particle stack
  for (unsigned int j = 2; j < event->Npart(); ++j) {
    if (!(event->PartInfo(j))->fIsAlive && (event->PartInfo(j))->fStatus != 0) continue;
    int PID = (event->PartInfo(j))->fPID;
    if (PID == 211 && event->PartInfo(j)->fP.E() > Ppip.E()) {
      Ppip = event->PartInfo(j)->fP;
    } else if (PID == 13) {
        Pmu = (event->PartInfo(j))->fP;
    }
  }

  double hadMass = FitUtils::Wrec(Pnu, Pmu);
  double pmu = -999;

  if (hadMass > 100 && hadMass < 1800) {
    pmu = FitUtils::p(Pmu);
  }

  this->X_VAR = pmu;

  return;
};

//******************************************************************** 
bool MINERvA_CCNpip_XSec_1Dpmu_nu::isSignal(FitEvent *event) {
//******************************************************************** 
  int dummy;
  return SignalDef::isCCNpip_MINERvA(event, dummy, EnuMin, EnuMax, false);
}
