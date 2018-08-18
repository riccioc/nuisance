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

#ifndef CORE_FULLEVENT_HXX_SEEN
#define CORE_FULLEVENT_HXX_SEEN

#include "core/MinimalEvent.hxx"
#include "core/Particle.hxx"


namespace nuis {
namespace core {

///\brief The full, internal event format.
class FullEvent : public MinimalEvent {
public:
  struct StatusParticles {
    Particle::Status_t status;
    std::vector<Particle> particles;
  };

  FullEvent();
  FullEvent(FullEvent const&) = delete;
  FullEvent(FullEvent&&);
  std::vector<StatusParticles> ParticleStack;

  void ClearParticleStack();
};

} // namespace core
} // namespace nuis
#endif
