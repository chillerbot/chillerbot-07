/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>

#include <engine/config.h>
#include <engine/demo.h>
#include <engine/friends.h>
#include <engine/serverbrowser.h>
#include <engine/shared/config.h>
#include <generated/protocol.h>
#include <generated/client_data.h>
#include <game/client/animstate.h>
#include <game/client/gameclient.h>

#include "menus.h"
#include "motd.h"
#include "voting.h"

void CMenus::GetSwitchTeamInfo(CSwitchTeamInfo *pInfo)
{
	pInfo->m_aNotification[0] = 0;
	int TeamMod = m_pClient->m_aClients[m_pClient->m_LocalClientID].m_Team != TEAM_SPECTATORS ? -1 : 0;
	pInfo->m_AllowSpec = true;
	pInfo->m_TimeLeft = 0;

	if(TeamMod + m_pClient->m_GameInfo.m_aTeamSize[TEAM_RED] + m_pClient->m_GameInfo.m_aTeamSize[TEAM_BLUE] >= m_pClient->m_ServerSettings.m_PlayerSlots)
	{
		str_format(pInfo->m_aNotification, sizeof(pInfo->m_aNotification), Localize("Only %d active players are allowed"), m_pClient->m_ServerSettings.m_PlayerSlots);
	}
	else if(m_pClient->m_ServerSettings.m_TeamLock)
		str_copy(pInfo->m_aNotification, Localize("Teams are locked"), sizeof(pInfo->m_aNotification));
	else if(m_pClient->m_TeamCooldownTick + 1 >= Client()->GameTick())
	{
		pInfo->m_TimeLeft = (m_pClient->m_TeamCooldownTick - Client()->GameTick()) / Client()->GameTickSpeed() + 1;
		str_format(pInfo->m_aNotification, sizeof(pInfo->m_aNotification), Localize("Teams are locked. Time to wait before changing team: %02d:%02d"), pInfo->m_TimeLeft / 60, pInfo->m_TimeLeft % 60);
		pInfo->m_AllowSpec = false;
	}
}

void CMenus::RenderGame(CUIRect MainView)
{

}

void CMenus::RenderPlayers(CUIRect MainView)
{
}

void CMenus::RenderServerInfo(CUIRect MainView)
{

}

void CMenus::RenderServerControlServer(CUIRect MainView)
{

}

void CMenus::RenderServerControlKick(CUIRect MainView, bool FilterSpectators)
{

}

void CMenus::HandleCallvote(int Page, bool Force)
{
	if(Page == 0)
		m_pClient->m_pVoting->CallvoteOption(m_CallvoteSelectedOption, m_aCallvoteReason, Force);
	else if(Page == 1)
	{
		if(m_CallvoteSelectedPlayer >= 0 && m_CallvoteSelectedPlayer < MAX_CLIENTS &&
			m_pClient->m_aClients[m_CallvoteSelectedPlayer].m_Active)
		{
			m_pClient->m_pVoting->CallvoteKick(m_CallvoteSelectedPlayer, m_aCallvoteReason, Force);
			SetActive(false);
		}
	}
	else if(Page == 2)
	{
		if(m_CallvoteSelectedPlayer >= 0 && m_CallvoteSelectedPlayer < MAX_CLIENTS &&
			m_pClient->m_aClients[m_CallvoteSelectedPlayer].m_Active)
		{
			m_pClient->m_pVoting->CallvoteSpectate(m_CallvoteSelectedPlayer, m_aCallvoteReason, Force);
			SetActive(false);
		}
	}
}

void CMenus::RenderServerControl(CUIRect MainView)
{

}

