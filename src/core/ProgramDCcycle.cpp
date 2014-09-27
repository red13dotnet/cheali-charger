/*
    cheali-charger - open source firmware for a variety of LiPo chargers
    Copyright (C) 2013  Paweł Stawicki. All right reserved.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ProgramDCcycle.h"
#include "Program.h"
#include "DelayStrategy.h"
#include "Settings.h"

using namespace Program;

namespace ProgramDCcycle {
    uint8_t currentCycle;
    char cycleMode;

    Strategy::statusType runCycleDischargeCommon(BatteryGroup prog1);
    Strategy::statusType runCycleChargeCommon(BatteryGroup prog1, bool mode);
}

Strategy::statusType ProgramDCcycle::runDCcycle(BatteryGroup prog1)
{ //TODO_NJ
    Strategy::statusType status;

    for(currentCycle = 1; currentCycle <= settings.DCcycles_; currentCycle++) {

        //discharge
        AnalogInputs::resetAccumulatedMeasurements();
        cycleMode='D';
        status = runCycleDischargeCommon(prog1);
        if(status != Strategy::COMPLETE) {break;} 
 
        //waiting after discharge
        cycleMode='W';
        status = runDCRestTime();
        if(status != Strategy::COMPLETE) { break; }

        //charge
        Screen::resetETA();
        AnalogInputs::resetAccumulatedMeasurements();
        cycleMode='C';

        //lastcharge? (no need wait at end?)
        if (currentCycle != settings.DCcycles_) {
            status = runCycleChargeCommon(prog1, true); //independent exit
        } else {
            status = runCycleChargeCommon(prog1, false);
        }
        if(status != Strategy::COMPLETE) {break;}

        //waiting after charge
        cycleMode='W';
        if (currentCycle != settings.DCcycles_) {
            status = runDCRestTime();
            if(status != Strategy::COMPLETE) { break; }
            else {status = Strategy::COMPLETE;}
        }
    }
    return status;
}

Strategy::statusType ProgramDCcycle::runCycleDischargeCommon(BatteryGroup prog1)
{
    Strategy::statusType status;

    if(prog1 == LiXX)
    {
        status = runDischarge(true);
    }

    if(prog1 == NiXX)
    {
        status = runNiXXDischarge(true);
    }

    if(prog1 == Pb)
    {
        status = runDischarge(true);
    }

    return status;
}

Strategy::statusType ProgramDCcycle::runCycleChargeCommon(BatteryGroup prog1, bool mode)
{
    Strategy::statusType status;

    //lastcharge? (no need wait at end?)
    if (currentCycle != settings.DCcycles_) {
        if(prog1 == LiXX) {
            status =  runTheveninChargeBalance(mode);
        } //independent exit
        if(prog1 == NiXX) {
            status =  runDeltaCharge(mode);
        } //independent exit
        if(prog1 == Pb) {
            status =  runTheveninCharge(settings.Lixx_Imin_, mode);
        }  //independent exit
    } else {
        if(prog1 == LiXX) {
            status = runTheveninChargeBalance();
        } //normal exit point
        if(prog1 == NiXX) {
            status = runDeltaCharge();
        } //normal exit point
        if(prog1 == Pb) {
            status =  runTheveninCharge(settings.Lixx_Imin_);
        }   //normal exit point
    }

    return status;
}
