/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/demo.h>
#include <engine/shared/config.h>
#include <generated/client_data.h>
#include <generated/protocol.h>
#include <game/client/animstate.h>
#include <game/client/gameclient.h>
#include <game/client/localization.h>
#include <game/client/components/countryflags.h>
#include <game/client/components/motd.h>

#include "menus.h"
#include "scoreboard.h"


CScoreboard::CScoreboard()
{
	m_PlayerLines = 0;
	OnReset();
}

void CScoreboard::ConKeyScoreboard(IConsole::IResult *pResult, void *pUserData)
{
	((CScoreboard *)pUserData)->m_Active = pResult->GetInteger(0) != 0;
}

void CScoreboard::OnReset()
{
	m_Active = false;

	for(int i = 0; i < MAX_CLIENTS; i++)
		ResetPlayerStats(i);
}

void CScoreboard::OnRelease()
{
	m_Active = false;
}

void CScoreboard::OnConsoleInit()
{
	Console()->Register("+scoreboard", "", CFGFLAG_CLIENT, ConKeyScoreboard, this, "Show scoreboard");
}

void CScoreboard::RenderGoals(float x, float y, float w)
{

}

float CScoreboard::RenderSpectators(float x, float y, float w)
{
	return 0.0f;
}

float CScoreboard::RenderScoreboard(float x, float y, float w, int Team, const char *pTitle, int Align)
{
	return 0.0f;
}

void CScoreboard::RenderRecordingNotification(float x)
{

}

void CScoreboard::OnRender()
{

}

void CScoreboard::OnMessage(int MsgType, void *pRawMsg)
{
	if(MsgType == NETMSGTYPE_SV_KILLMSG)
	{
		CNetMsg_Sv_KillMsg *pMsg = (CNetMsg_Sv_KillMsg *)pRawMsg;

		m_aPlayerStats[pMsg->m_Victim].m_Deaths++;
		if(pMsg->m_Victim != pMsg->m_Killer)
			m_aPlayerStats[pMsg->m_Killer].m_Kills++;
	}
	else if(MsgType == NETMSGTYPE_SV_CLIENTINFO && Client()->State() != IClient::STATE_DEMOPLAYBACK)
	{
		CNetMsg_Sv_ClientInfo *pMsg = (CNetMsg_Sv_ClientInfo *)pRawMsg;

		if(!m_pClient->m_aClients[pMsg->m_ClientID].m_Active)
			return;

		ResetPlayerStats(pMsg->m_ClientID);
	}
}

bool CScoreboard::Active()
{
	// if we actively wanna look on the scoreboard
	if(m_Active)
		return true;

	if(m_pClient->m_LocalClientID != -1 && m_pClient->m_aClients[m_pClient->m_LocalClientID].m_Team != TEAM_SPECTATORS)
	{
		// we are not a spectator, check if we are dead, don't follow a player and the game isn't paused
		if(!m_pClient->m_Snap.m_pLocalCharacter && !m_pClient->m_Snap.m_SpecInfo.m_Active &&
			!(m_pClient->m_Snap.m_pGameData && m_pClient->m_Snap.m_pGameData->m_GameStateFlags&GAMESTATEFLAG_PAUSED))
			return true;
	}

	// if the game is over
	if(m_pClient->m_Snap.m_pGameData && m_pClient->m_Snap.m_pGameData->m_GameStateFlags&(GAMESTATEFLAG_ROUNDOVER|GAMESTATEFLAG_GAMEOVER))
		return true;

	return false;
}

void CScoreboard::ResetPlayerStats(int ClientID)
{
	m_aPlayerStats[ClientID].Reset();
}

CUIRect CScoreboard::GetScoreboardRect()
{
	return m_TotalRect;
}

const char *CScoreboard::GetClanName(int Team)
{
	int ClanPlayers = 0;
	const char *pClanName = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!m_pClient->m_aClients[i].m_Active || m_pClient->m_aClients[i].m_Team != Team)
			continue;

		if(!pClanName)
		{
			pClanName = m_pClient->m_aClients[i].m_aClan;
			ClanPlayers++;
		}
		else
		{
			if(str_comp(m_pClient->m_aClients[i].m_aClan, pClanName) == 0)
				ClanPlayers++;
			else
				return 0;
		}
	}

	if(ClanPlayers > 1 && pClanName[0])
		return pClanName;
	else
		return 0;
}

CScoreboard::CPlayerStats::CPlayerStats()
{
	m_Kills = 0;
	m_Deaths = 0;
}

void CScoreboard::CPlayerStats::Reset()
{
	m_Kills = 0;
	m_Deaths = 0;
}
