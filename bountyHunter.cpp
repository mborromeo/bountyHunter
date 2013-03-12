/*
Copyright (c) 2012 Vladimir Jimenez
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote products
     derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


*** Bounty Hunter Details ***
Author:
Vladimir Jimenez (allejo)

Description:
Kill someone on a rapmage, get extra points

Slash Commands:
N/A

License:
BSD

Version:
1.0.3
*/

#include "bzfsAPI.h"
#include "plugin_utils.h"

class bountyHunter : public bz_Plugin
{
public:

    virtual const char* Name (){return "Bounty Hunter";}
    virtual void Init (const char* config);
    virtual void Event (bz_EventData* eventData);
    virtual void Cleanup (void);
};

BZ_PLUGIN(bountyHunter);

int numberOfKills[256] = {0};
int rampage[8] = {0, 6, 12, 18, 24, 30, 36, 999};
int lastPlayerDied = -1;
int flagID = -1;
double timeDropped = 0;

void bountyHunter::Init(const char* /*commandLine*/)
{
    bz_debugMessage(4,"bountyHunter plugin loaded");

    Register(bz_eFlagDroppedEvent);
    Register(bz_ePlayerDieEvent);
    Register(bz_ePlayerPartEvent);
}

void bountyHunter::Cleanup(void)
{
    bz_debugMessage(4,"bountyHunter plugin unloaded");
    Flush();
}

void bountyHunter::Event(bz_EventData* eventData)
{
    switch (eventData->eventType)
    {
        case bz_ePlayerDieEvent: // A player dies
        {
            bz_PlayerDieEventData_V1* diedata = (bz_PlayerDieEventData_V1*)eventData;

            if (diedata->killerID == 253)
                return;

            if (diedata->playerID != diedata->killerID)
                numberOfKills[diedata->killerID]++;

            int killerScore = bz_getPlayerWins(diedata->killerID);
            int killerRampageScore = 0;
            int killerBonusScore = 0;
            int sizeOfRampageArray = sizeof(rampage)/sizeof(int);

            if (diedata->playerID == lastPlayerDied && diedata->playerID != diedata->killerID && timeDropped + 3 > bz_getCurrentTime())
            {
                killerBonusScore = 2;
                std::string flag = bz_getName(flagID).c_str();
                std::string tmp;

                if (flag == "R*") tmp = "red";
                else if (flag == "G*") tmp = "green";
                else if (flag == "B*") tmp = "blue";
                else tmp = "purple";

                bz_sendTextMessagef(BZ_SERVER, diedata->killerID, "Shooting the %s team flag carrier has earned you 2 bounty points", tmp.c_str());
            }

            if (numberOfKills[diedata->playerID] > 0)
            {
                for (int i = 0; i < sizeOfRampageArray; i++)
                {
                    if (numberOfKills[diedata->playerID] >= rampage[i] && numberOfKills[diedata->playerID] < rampage[i+1] && diedata->playerID != diedata->killerID)
                    {
                        killerRampageScore = 2 * i;

                        if ((2 * i) > 0)
                        {
                            bz_sendTextMessagef(BZ_SERVER, diedata->killerID, "Stopping %s's rampage earned you %i bounty points", bz_getPlayerByIndex(diedata->playerID)->callsign.c_str(), (2*i));
                        }
                    }
                }
            }

            killerScore = killerScore + killerRampageScore + killerBonusScore;
            bz_setPlayerWins(diedata->killerID, killerScore);
            numberOfKills[diedata->playerID] = 0;
        }
        break;

        case bz_eFlagDroppedEvent:
        {
            bz_FlagDroppedEventData_V1* flagdropdata = (bz_FlagDroppedEventData_V1*)eventData;

            std::string flag = bz_getName(flagdropdata->flagID).c_str();

            if (flag != "R*" || flag != "G*" || flag != "B*" || flag != "P*")
                return;

            if (flagdropdata->flagID == 0 || flagdropdata->flagID == 1)
            {
                lastPlayerDied = flagdropdata->playerID;
                flagID = flagdropdata->flagID;
                timeDropped = bz_getCurrentTime();
            }
        }
        break;

        case bz_ePlayerPartEvent:
        {
            bz_PlayerJoinPartEventData_V1* partdata = (bz_PlayerJoinPartEventData_V1*)eventData;

            numberOfKills[partdata->playerID] = 0;
        }
        break;

        default:break;
    }
}