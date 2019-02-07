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

#ifndef EVENT_FULLEVENT_HXX_SEEN
#define EVENT_FULLEVENT_HXX_SEEN

#include "event/MinimalEvent.hxx"
#include "event/Particle.hxx"

#include "string_parsers/to_string.hxx"

#include <sstream>

namespace nuis {
namespace event {

///\brief The full, internal event format.
class FullEvent : public MinimalEvent {
public:
  struct StatusParticles {
    Particle::Status_t status;
    std::vector<Particle> particles;
  };

  FullEvent();
  FullEvent(FullEvent const &) = delete;
  FullEvent(FullEvent &&);
  FullEvent& operator=(FullEvent &&);

  FullEvent Clone() const;

  std::vector<StatusParticles> ParticleStack;

  void ClearParticleStack();

  std::string to_string() const;
};

} // namespace event
} // namespace nuis

#endif
