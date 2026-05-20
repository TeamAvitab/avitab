/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018-2026 Folke Will and Avitab Contributors
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Affero General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Affero General Public License for more details.
 *
 *   You should have received a copy of the GNU Affero General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <sstream>
#include <iomanip>
#include "Navigation.h"

namespace navdb {

RadioFrequency::RadioFrequency(int frq, int places, FrequencyUnit unit, const std::string& desc):
    frequency(frq),
    places(places),
    unit(unit),
    description(desc)
{
}

std::string RadioFrequency::getFrequencyString(bool appendUnits) const {
    if (unit == FrequencyUnit::MHZ) {
        std::ostringstream str;
        int factor = 1;
        for (int i = 0; i < places; i++) {
            factor *= 10;
        }
        int beforeDot = frequency / factor;
        int afterDot = frequency % factor;
        str << beforeDot << "." << std::setw(places) << std::setfill('0') << afterDot << (appendUnits ? " MHz" : "");
        return str.str();
    } else if (unit == FrequencyUnit::KHZ) {
        return std::to_string(frequency) + (appendUnits ? " kHz" : "");
    } else {
        return "<unit error>";
    }
}

} /* namespace navdb */
