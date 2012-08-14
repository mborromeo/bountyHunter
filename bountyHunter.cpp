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
1.0
*/

#include "bzfsAPI.h"
#include "plugin_utils.h"

class bountyHunter : public bz_Plugin
{
public:

    virtual const char* Name (){return "Bounty Hunter";}
    virtual void Init (const char* config);
    virtual void Event (bz_EventData* eventData);
    virtual void Cleanup ();

};

BZ_PLUGIN(bountyHunter);

int playerKills[256] = {0};
int rampage[5] = {0, 6, 12, 18, 24};

void bountyHunter::Init(const char* /*commandLine*/)
{
    bz_debugMessage(4,"bountyHunter plugin loaded");
    
    Register(bz_ePlayerDieEvent);
    Register(bz_ePlayerPartEvent);
}

void bountyHunter::Event(bz_EventData* eventData)
{
    switch (eventData->eventType)
    {
        case bz_ePlayerDieEvent: // A player dies
        {
            bz_PlayerDieEventData_V1* diedata = (bz_PlayerDieEventData_V1*)eventData;
            playerKills[diedata->killerID]++;
            
            int sizeOfRampageArray = sizeof(rampage)/sizeof(int);

            if (playerKills[diedata->playerID] > 0)
            {
                for (int i = 0; i < sizeOfRampageArray; i++)
                {
                    if (playerKills[diedata->playerID] >= rampage[i] && playerKills[diedata->playerID] < rampage[i+1])
                    {
                        int killerScore = bz_getPlayerWins(diedata->killerID);
                        killerScore = killerScore + ((5*i) - 1);
                        bz_setPlayerWins(diedata->killerID, killerScore);
                        
                        bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "%s has scored a %i point bounty by killing %s", bz_getPlayerByIndex(diedata->killerID)->callsign.c_str(), killerScore + 1, bz_getPlayerByIndex(diedata->playerID)->callsign.c_str());
                    }
                }
                
                playerKills[diedata->playerID] = 0;
            }
        }
        break;
        
        case bz_ePlayerPartEvent:
        {
            bz_PlayerJoinPartEventData_V1* partdata = (bz_PlayerJoinPartEventData_V1*)eventData;
            
            playerKills[partdata->playerID] = 0; //Reset the player slots number of kills with Useless when they leave
        }
        break;

        default:break;
    }
}