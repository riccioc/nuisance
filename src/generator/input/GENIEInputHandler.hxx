// Copyright 2018 L. Pickering, P Stowell, R. Terri, C. Wilkinson, C. Wret

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

#pragma once

#include "event/FullEvent.hxx"

#include "input/IInputHandler.hxx"

#include "exception/exception.hxx"

#include "utility/ROOTUtility.hxx"

#ifdef GENIE_V3_INTERFACE
#include "Framework/Ntuple/NtpMCEventRecord.h"
#else
#include "Ntuple/NtpMCEventRecord.h"
#endif

#include <memory>

namespace fhicl {
class ParameterSet;
}

class GENIEInputHandler : public IInputHandler {
  mutable nuis::utility::TreeFile fInputTree;
  mutable nuis::event::FullEvent fReaderEvent;
  mutable std::vector<double> fWeightCache;
  mutable genie::NtpMCEventRecord *fGenieNtpl;

  bool fKeepIntermediates;
  bool fKeepNuclearParticles;

public:
  NEW_NUIS_EXCEPT(weight_cache_miss);

  GENIEInputHandler();
  GENIEInputHandler(GENIEInputHandler const &) = delete;
  GENIEInputHandler(GENIEInputHandler &&);

  void Initialize(fhicl::ParameterSet const &);
  nuis::event::MinimalEvent const &GetMinimalEvent(ev_index_t idx) const;
  nuis::event::FullEvent const &GetFullEvent(ev_index_t idx) const;
  double GetEventWeight(ev_index_t idx) const;
  size_t GetNEvents() const;

  double GetXSecScaleFactor(std::pair<double, double> const &EnuRange) const;

  nuis::GeneratorManager::Generator_id_t GetGeneratorId();
};
