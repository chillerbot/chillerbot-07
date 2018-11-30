/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/tl/array.h>

#include <engine/keys.h>
#include <engine/demo.h>
#include <engine/serverbrowser.h>
#include <engine/shared/config.h>
#include <engine/storage.h>

#include <game/layers.h>
#include <game/client/gameclient.h>
#include <game/client/component.h>

#include <game/client/components/camera.h>
#include <game/client/components/mapimages.h>


#include "maplayers.h"

CMapLayers::CMapLayers(int t)
{
	m_Type = t;
	m_CurrentLocalTick = 0;
	m_LastLocalTick = 0;
	m_EnvelopeUpdate = false;
	m_pMenuMap = 0;
	m_pMenuLayers = 0;
	m_OnlineStartTime = 0;
}

void CMapLayers::OnStateChange(int NewState, int OldState)
{
	if(NewState == IClient::STATE_ONLINE)
		m_OnlineStartTime = Client()->LocalTime(); // reset time for non-scynchronized envelopes
}

void CMapLayers::LoadBackgroundMap()
{

}

void CMapLayers::OnInit()
{

}

void CMapLayers::OnMapLoad()
{

}

void CMapLayers::LoadEnvPoints(const CLayers *pLayers, array<CEnvPoint>& lEnvPoints)
{

}

void CMapLayers::EnvelopeUpdate()
{

}

void CMapLayers::EnvelopeEval(float TimeOffset, int Env, float *pChannels, void *pUser)
{

}

void CMapLayers::OnRender()
{

}

void CMapLayers::ConchainBackgroundMap(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{

}

void CMapLayers::OnConsoleInit()
{
	Console()->Chain("cl_menu_map", ConchainBackgroundMap, this);
	Console()->Chain("cl_show_menu_map", ConchainBackgroundMap, this);
}

void CMapLayers::BackgroundMapUpdate()
{

}
