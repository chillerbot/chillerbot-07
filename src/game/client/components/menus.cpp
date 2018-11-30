/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <math.h>

#include <base/system.h>
#include <base/math.h>
#include <base/vmath.h>

#include <engine/config.h>
#include <engine/editor.h>
#include <engine/engine.h>
#include <engine/friends.h>
#include <engine/keys.h>
#include <engine/serverbrowser.h>
#include <engine/storage.h>
#include <engine/shared/config.h>

#include <game/version.h>
#include <generated/protocol.h>

#include <generated/client_data.h>
#include <game/client/components/binds.h>
#include <game/client/components/camera.h>
#include <game/client/components/console.h>
#include <game/client/components/sounds.h>
#include <game/client/gameclient.h>
#include <game/client/lineinput.h>
#include <mastersrv/mastersrv.h>

#include "maplayers.h"
#include "countryflags.h"
#include "menus.h"
#include "skins.h"

float CMenus::ms_ButtonHeight = 25.0f;
float CMenus::ms_ListheaderHeight = 17.0f;
float CMenus::ms_FontmodHeight = 0.8f;
float CMenus::ms_BackgroundAlpha = 0.25f;


CMenus::CMenus()
{
	m_Popup = POPUP_NONE;
	m_NextPopup = POPUP_NONE;
	m_ActivePage = PAGE_INTERNET;
	m_GamePage = PAGE_GAME;
	m_SidebarTab = 0;
	m_SidebarActive = true;
	m_ShowServerDetails = true;

	m_NeedRestartGraphics = false;
	m_NeedRestartSound = false;
	m_TeePartSelected = SKINPART_BODY;
	m_aSaveSkinName[0] = 0;
	m_RefreshSkinSelector = true;
	m_pSelectedSkin = 0;
	m_MenuActive = true;
	m_SeekBarActivatedTime = 0;
	m_SeekBarActive = true;
	m_UseMouseButtons = true;

	SetMenuPage(PAGE_START);

	m_PopupActive = false;

	m_EscapePressed = false;
	m_EnterPressed = false;
	m_TabPressed = false;
	m_DeletePressed = false;
	m_UpArrowPressed = false;
	m_DownArrowPressed = false;

	m_LastInput = time_get();

	str_copy(m_aCurrentDemoFolder, "demos", sizeof(m_aCurrentDemoFolder));
	m_aCallvoteReason[0] = 0;

	m_SelectedFilter = 0;

	m_SelectedServer.m_Filter = -1;
	m_SelectedServer.m_Index = -1;
	m_ActiveListBox = ACTLB_THEME;
}

float CMenus::ButtonFade(CButtonContainer *pBC, float Seconds, int Checked)
{
	return 0.0f;
}

int CMenus::UiDoListboxEnd(CListBoxState* pState, bool *pItemActivated)
{
	return 0;
}

void CMenus::RenderLoading()
{

}

int CMenus::MenuImageScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	return 0;
}

const CMenus::CMenuImage *CMenus::FindMenuImage(const char *pName)
{
	for(int i = 0; i < m_lMenuImages.size(); i++)
	{
		if(str_comp(m_lMenuImages[i].m_aName, pName) == 0)
			return &m_lMenuImages[i];
	}
	return 0;
}

void CMenus::UpdateVideoFormats()
{

}

void CMenus::UpdatedFilteredVideoModes()
{

}

void CMenus::UpdateVideoModeSettings()
{

}

void CMenus::OnInit()
{
	Console()->Chain("add_favorite", ConchainServerbrowserUpdate, this);
	Console()->Chain("remove_favorite", ConchainServerbrowserUpdate, this);
	Console()->Chain("add_friend", ConchainFriendlistUpdate, this);
	Console()->Chain("remove_friend", ConchainFriendlistUpdate, this);
	Console()->Chain("snd_enable_music", ConchainToggleMusic, this);
}

void CMenus::PopupMessage(const char *pTopic, const char *pBody, const char *pButton, int Next)
{

}

int CMenus::Render()
{
	return 0;
}


void CMenus::SetActive(bool Active)
{

}

void CMenus::OnReset()
{

}

bool CMenus::OnMouseMove(float x, float y)
{
	return false;
}

bool CMenus::OnInput(IInput::CEvent e)
{
	return false;
}

void CMenus::OnConsoleInit()
{

}

void CMenus::OnStateChange(int NewState, int OldState)
{

}

extern "C" void font_debug_render();

void CMenus::OnRender()
{

}

void CMenus::RenderBackground()
{

}

void CMenus::ConchainToggleMusic(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{

}

void CMenus::ToggleMusic()
{

}

void CMenus::SetMenuPage(int NewPage)
{

}
