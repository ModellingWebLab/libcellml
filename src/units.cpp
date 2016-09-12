/*
Copyright libCellML Contributors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "libcellml/units.h"

#include <algorithm>
#include <cassert>
#include <map>
#include <vector>

#include "libcellml/import.h"
#include "utilities.h"

namespace libcellml {

/**
 * @brief Map Prefix to their string forms.
 *
 * An internal map used to convert a Prefix into its string form.
 */
std::map<Prefix, const std::string> prefixToString =
{
    {Prefix::ATTO, "atto"},
    {Prefix::CENTI, "centi"},
    {Prefix::DECA, "deca"},
    {Prefix::DECI, "deci"},
    {Prefix::EXA, "exa"},
    {Prefix::FEMTO, "femto"},
    {Prefix::GIGA, "giga"},
    {Prefix::HECTO, "hecto"},
    {Prefix::KILO, "kilo"},
    {Prefix::MEGA, "mega"},
    {Prefix::MICRO, "micro"},
    {Prefix::MILLI, "milli"},
    {Prefix::NANO, "nano"},
    {Prefix::PETA, "peta"},
    {Prefix::PICO, "pico"},
    {Prefix::TERA, "tera"},
    {Prefix::YOCTO, "yocto"},
    {Prefix::YOTTA, "yotta"},
    {Prefix::ZEPTO, "zepto"},
    {Prefix::ZETTA, "zetta"}
};

/**
 * @brief Map StandardUnit to their string forms.
 *
 * An internal map used to convert a standard unit into its string form.
 */
std::map<Units::StandardUnit, const std::string> standardUnitToString =
{
    {Units::StandardUnit::AMPERE, "ampere"},
    {Units::StandardUnit::BECQUEREL, "becquerel"},
    {Units::StandardUnit::CANDELA, "candela"},
    {Units::StandardUnit::CELSIUS, "celsius"},
    {Units::StandardUnit::COULOMB, "coulomb"},
    {Units::StandardUnit::DIMENSIONLESS, "dimensionless"},
    {Units::StandardUnit::FARAD, "farad"},
    {Units::StandardUnit::GRAM, "gram"},
    {Units::StandardUnit::GRAY, "gray"},
    {Units::StandardUnit::HENRY, "henry"},
    {Units::StandardUnit::HERTZ, "hertz"},
    {Units::StandardUnit::JOULE, "joule"},
    {Units::StandardUnit::KATAL, "katal"},
    {Units::StandardUnit::KELVIN, "kelvin"},
    {Units::StandardUnit::KILOGRAM, "kilogram"},
    {Units::StandardUnit::LITER, "liter"},
    {Units::StandardUnit::LITRE, "litre"},
    {Units::StandardUnit::LUMEN, "lumen"},
    {Units::StandardUnit::LUX, "lux"},
    {Units::StandardUnit::METER, "meter"},
    {Units::StandardUnit::METRE, "metre"},
    {Units::StandardUnit::MOLE, "mole"},
    {Units::StandardUnit::NEWTON, "newton"},
    {Units::StandardUnit::OHM, "ohm"},
    {Units::StandardUnit::PASCAL, "pascal"},
    {Units::StandardUnit::RADIAN, "radian"},
    {Units::StandardUnit::SECOND, "second"},
    {Units::StandardUnit::SIEMENS, "siemens"},
    {Units::StandardUnit::SIEVERT, "sievert"},
    {Units::StandardUnit::STERADIAN, "steradian"},
    {Units::StandardUnit::TESLA, "tesla"},
    {Units::StandardUnit::VOLT, "volt"},
    {Units::StandardUnit::WATT, "watt"},
    {Units::StandardUnit::WEBER, "weber"}
};

/**
 * @brief The Unit struct.
 *
 * An internal structure to capture a unit definition.  The
 * prefix can be expressed using either an integer or an enum.
 * The enum structure member is given preference if both are set.
 */
struct Unit
{
    std::string mReference; /**< Reference to the units for the unit.*/
    std::string mPrefix; /**< String expression of the prefix for the unit.*/
    std::string mExponent; /**< Exponent for the unit.*/
    std::string mMultiplier; /**< Multiplier for the unit.*/
    std::string mOffset; /**< Offset for the unit.*/
};

/**
 * @brief The Units::UnitsImpl struct.
 *
 * The private implementation for the Units class.
 */
struct Units::UnitsImpl
{
    std::vector<Unit>::iterator findUnit(const std::string &reference);
    bool mBaseUnit = false; /**< Flag to determine if this Units is a base unit or not.*/
    std::vector<Unit> mUnits; /**< A vector of unit defined for this Units.*/
};

std::vector<Unit>::iterator Units::UnitsImpl::findUnit(const std::string &reference)
{
    return std::find_if(mUnits.begin(), mUnits.end(),
                        [=](const Unit& u) -> bool { return u.mReference == reference; });
}

Units::Units()
    : mPimpl(new UnitsImpl())
{
}

Units::~Units()
{
    delete mPimpl;
}

Units::Units(const Units& rhs)
    : ImportedEntity(rhs)
    , mPimpl(new UnitsImpl())
{
    mPimpl->mBaseUnit = rhs.mPimpl->mBaseUnit;
    mPimpl->mUnits = rhs.mPimpl->mUnits;
}

Units::Units(Units &&rhs)
    : ImportedEntity(std::move(rhs))
    , mPimpl(rhs.mPimpl)
{
    rhs.mPimpl = nullptr;
}

Units& Units::operator=(Units e)
{
    ImportedEntity::operator= (e);
    e.swap(*this);
    return *this;
}

void Units::swap(Units &rhs)
{
    std::swap(this->mPimpl, rhs.mPimpl);
}

std::string Units::doSerialisation(Format format) const
{
    std::string repr = "";
    if (format == Format::XML) {
        if (getName().length()) {
            if (isImport()) {
                repr += "<import xlink:href=\"" + getImport()->getSource() + "\" xmlns:xlink=\"http://www.w3.org/1999/xlink\"";
                if (getImport()->getId().length()) {
                    repr += " id=\"" + getImport()->getId() + "\"";
                }
                repr += "><units units_ref=\"" + getImportReference() + "\" name=\"" + getName() + "\"";
                if (getId().length()) {
                    repr += " id=\"" + getId() + "\"";
                }
                repr += "/></import>";
            } else {
                bool endTag = false;
                repr += "<units name=\"" + getName() + "\"";
                if (getId().length()) {
                    repr += " id=\"" + getId() + "\"";
                }
                if (isBaseUnit()) {
                    repr += " base_unit=\"yes\"";
                } else if (mPimpl->mUnits.size() > 0) {
                    endTag = true;
                    repr += ">";
                    for (std::vector<Unit>::size_type i = 0; i != mPimpl->mUnits.size(); ++i) {
                        repr += "<unit";
                        Unit u = mPimpl->mUnits[i];
                        if (u.mExponent.length()) {
                            repr += " exponent=\"" + u.mExponent + "\"";
                        }
                        if (u.mMultiplier.length()) {
                            repr += " multiplier=\"" + u.mMultiplier + "\"";
                        }
                        if (u.mOffset.length()) {
                            repr += " offset=\"" + u.mOffset + "\"";
                        }
                        if (u.mPrefix.length()) {
                            repr += " prefix=\"" + u.mPrefix + "\"";
                        }
                        repr += " units=\"" + u.mReference + "\"";
                        repr += "/>";
                    }
                }
                if (endTag) {
                    repr += "</units>";
                } else {
                    repr += "/>";
                }
            }
        }
    }

    return repr;
}

bool Units::isBaseUnit() const
{
    return mPimpl->mBaseUnit;
}

void Units::setBaseUnit(bool state)
{
    mPimpl->mBaseUnit = state;
}

void Units::addUnit(const std::string &reference, const std::string &prefix, double exponent,
                    double multiplier, double offset)
{
    Unit u;
    u.mReference = reference;
    // Allow all nonzero user-specified prefixes
    try
    {
        double prefixDouble = std::stod(prefix);
        if (prefixDouble != 0.0) {
            u.mPrefix = prefix;
        }
    } catch (std::invalid_argument) {
        u.mPrefix = prefix;
    } catch (std::out_of_range) {
        u.mPrefix = prefix;
    }
    if (exponent != 1.0) {
        u.mExponent = convertDoubleToString(exponent);
    }
    if (multiplier != 1.0) {
        u.mMultiplier = convertDoubleToString(multiplier);
    }
    if (offset != 0.0) {
        u.mOffset = convertDoubleToString(offset);
    }
    mPimpl->mUnits.push_back(u);
}

void Units::addUnit(const std::string &reference, Prefix prefix, double exponent,
                    double multiplier, double offset)
{
    auto search = prefixToString.find(prefix);
    assert(search != prefixToString.end());
    const std::string prefixString = search->second;
    addUnit(reference, prefixString, exponent, multiplier, offset);
}

void Units::addUnit(const std::string &reference, double prefix, double exponent,
                    double multiplier, double offset)
{
    const std::string prefixString = convertDoubleToString(prefix);
    addUnit(reference, prefixString, exponent, multiplier, offset);
}

void Units::addUnit(const std::string &reference, double exponent)
{
    addUnit(reference, "0.0", exponent, 1.0, 0.0);
}

void Units::addUnit(const std::string &reference)
{
    addUnit(reference, "0.0", 1.0, 1.0, 0.0);
}

void Units::addUnit(StandardUnit standardRef, const std::string &prefix, double exponent,
                    double multiplier, double offset)
{
   const std::string reference = standardUnitToString.find(standardRef)->second;
   addUnit(reference, prefix, exponent, multiplier, offset);
}

void Units::addUnit(StandardUnit standardRef, Prefix prefix, double exponent,
                    double multiplier, double offset)
{
   const std::string reference = standardUnitToString.find(standardRef)->second;
   const std::string prefixString = prefixToString.find(prefix)->second;
   addUnit(reference, prefixString, exponent, multiplier, offset);
}

void Units::addUnit(StandardUnit standardRef, double prefix, double exponent,
                    double multiplier, double offset)
{
    const std::string reference = standardUnitToString.find(standardRef)->second;
    const std::string prefixString = convertDoubleToString(prefix);
    addUnit(reference, prefixString, exponent, multiplier, offset);
}

void Units::addUnit(StandardUnit standardRef, double exponent)
{
    const std::string reference = standardUnitToString.find(standardRef)->second;
    addUnit(reference, "0.0", exponent, 1.0, 0.0);
}

void Units::addUnit(StandardUnit standardRef)
{
    const std::string reference = standardUnitToString.find(standardRef)->second;
    addUnit(reference, "0.0", 1.0, 1.0, 0.0);
}

void Units::getUnitAttributes(StandardUnit standardRef, std::string &prefix, double &exponent, double &multiplier, double &offset) const
{
    std::string dummyReference;
    const std::string reference = standardUnitToString.find(standardRef)->second;
    auto result = mPimpl->findUnit(reference);
    getUnitAttributes(result - mPimpl->mUnits.begin(), dummyReference, prefix, exponent, multiplier, offset);
}

void Units::getUnitAttributes(const std::string &reference, std::string &prefix, double &exponent, double &multiplier, double &offset) const
{
    std::string dummyReference;
    auto result = mPimpl->findUnit(reference);
    getUnitAttributes(result - mPimpl->mUnits.begin(), dummyReference, prefix, exponent, multiplier, offset);
}

void Units::getUnitAttributes(size_t index, std::string &reference, std::string &prefix, double &exponent, double &multiplier, double &offset) const
{
    Unit u;
    if (index < mPimpl->mUnits.size()) {
        u = mPimpl->mUnits.at(index);
    }
    reference = u.mReference;
    prefix = u.mPrefix;
    if (u.mExponent.length()) {
        exponent = std::stod(u.mExponent);
    } else {
        exponent = 1.0;
    }
    if (u.mMultiplier.length()) {
        multiplier = std::stod(u.mMultiplier);
    } else {
        multiplier = 1.0;
    }
    if (u.mOffset.length()) {
        offset = std::stod(u.mOffset);
    } else {
        offset = 0.0;
    }
}

void Units::removeUnit(const std::string &reference)
{
    auto result = mPimpl->findUnit(reference);
    if (result != mPimpl->mUnits.end()) {
        mPimpl->mUnits.erase(result);
    }
}

void Units::removeUnit(size_t index)
{
    if (index < mPimpl->mUnits.size()) {
        mPimpl->mUnits.erase(mPimpl->mUnits.begin() + index);
    }
}

void Units::removeUnit(StandardUnit standardRef)
{
    const std::string reference = standardUnitToString.find(standardRef)->second;
    removeUnit(reference);
}

void Units::removeAllUnits()
{
    mPimpl->mUnits.clear();
}

void Units::setSourceUnits(const ImportPtr &imp, const std::string &name)
{
    setImport(imp);
    setImportReference(name);
}

size_t Units::unitCount() const
{
    return mPimpl->mUnits.size();
}

}
