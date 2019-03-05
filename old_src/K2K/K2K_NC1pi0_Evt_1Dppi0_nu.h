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

#ifndef K2K_NC1PI0_EVT_1DPPI0_NU_H_SEEN
#define K2K_NC1PI0_EVT_1DPPI0_NU_H_SEEN

#include "Measurement1D.h"

class K2K_NC1pi0_Evt_1Dppi0_nu : public Measurement1D {
public:

  K2K_NC1pi0_Evt_1Dppi0_nu(nuiskey samplekey);
  virtual ~K2K_NC1pi0_Evt_1Dppi0_nu() {};

  void FillEventVariables(FitEvent *event);
  bool isSignal(FitEvent *event);
};
  
#endif