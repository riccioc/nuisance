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

#ifndef ANL_CC1PI0_EVT_1DQ2_NU_H_SEEN
#define ANL_CC1PI0_EVT_1DQ2_NU_H_SEEN

// Fit Includes
#include "Measurement1D.h"

class ANL_CC1pi0_Evt_1DQ2_nu : public Measurement1D {
public:
	ANL_CC1pi0_Evt_1DQ2_nu(nuiskey samplekey);
	virtual ~ANL_CC1pi0_Evt_1DQ2_nu() {};

	void FillEventVariables(FitEvent *event);
	bool isSignal(FitEvent *event);

private:
	double HadCut;
};

#endif