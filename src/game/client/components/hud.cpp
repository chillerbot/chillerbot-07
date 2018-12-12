/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include <generated/protocol.h>
#include <generated/client_data.h>
#include <game/layers.h>
#include <game/client/gameclient.h>
#include <game/client/animstate.h>
#include <stdio.h>
#include "menus.h"
#include "controls.h"
#include "camera.h"
#include "hud.h"
#include "voting.h"
#include "binds.h"

CHud::CHud()
{
	// won't work if zero
	m_AverageFPS = 1.0f;

	m_WarmupHideTick = 0;
}

void CHud::OnReset()
{
	m_WarmupHideTick = 0;
}

void CHud::RenderGameTimer()
{

}

void CHud::RenderPauseTimer()
{

}

void CHud::RenderStartCountdown()
{

}

void CHud::RenderDeadNotification()
{

}

void CHud::RenderSuddenDeath()
{

}

void CHud::RenderScoreHud()
{

}

void CHud::RenderWarmupTimer()
{

}

void CHud::RenderFps()
{

}

void CHud::RenderConnectionWarning()
{
	if(Client()->ConnectionProblems())
	{
		printf("connection problems...\n");
	}
}

void CHud::RenderTeambalanceWarning()
{

}


void CHud::RenderVoting()
{
}

void CHud::RenderCursor()
{

}

void CHud::RenderNinjaBar(float x, float y, float Progress)
{

}

void CHud::RenderHealthAndAmmo(const CNetObj_Character *pCharacter)
{

}

void CHud::RenderSpectatorHud()
{

}

void CHud::RenderSpectatorNotification()
{
	if(m_pClient->m_aClients[m_pClient->m_LocalClientID].m_Team == TEAM_SPECTATORS &&
		m_pClient->m_TeamChangeTime + 5.0f >= Client()->LocalTime())
	{
		const char *pText = Localize("Click on a player or a flag to follow it");
		float FontSize = 16.0f;
		float w = TextRender()->TextWidth(0, FontSize, pText, -1);
		TextRender()->Text(0, 150 * Graphics()->ScreenAspect() + -w / 2, 30, FontSize, pText, -1);
	}
}

void CHud::OnRender()
{

}
