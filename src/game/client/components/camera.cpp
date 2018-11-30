/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>

#include <base/math.h>
#include <game/collision.h>
#include <game/client/gameclient.h>
#include <game/client/component.h>

#include "camera.h"
#include "controls.h"

CCamera::CCamera()
{

}

void CCamera::OnRender()
{

}

void CCamera::ChangePosition(int PositionNumber)
{

}

int CCamera::GetCurrentPosition()
{
	return 0;
}

void CCamera::ConSetPosition(IConsole::IResult *pResult, void *pUserData)
{

}

void CCamera::OnConsoleInit()
{
	Console()->Register("set_position", "iii", CFGFLAG_CLIENT, ConSetPosition, this, "Sets the rotation position");
}

void CCamera::OnStateChange(int NewState, int OldState)
{

}
