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

int CMenus::DoIcon(int ImageId, int SpriteId, const CUIRect *pRect)
{
	return 0;
}

int CMenus::DoButton_Toggle(const void *pID, int Checked, const CUIRect *pRect, bool Active)
{
	return 0;
}

int CMenus::DoButton_Menu(CButtonContainer *pBC, const char *pText, int Checked, const CUIRect *pRect, const char *pImageName, int Corners, float r, float FontFactor, vec4 ColorHot, bool TextFade)
{
	return 0;
}

void CMenus::DoButton_KeySelect(CButtonContainer *pBC, const char *pText, int Checked, const CUIRect *pRect)
{

}

int CMenus::DoButton_MenuTab(const void *pID, const char *pText, int Checked, const CUIRect *pRect, int Corners)
{
	return 0;
}

int CMenus::DoButton_MenuTabTop(CButtonContainer *pBC, const char *pText, int Checked, const CUIRect *pRect, float Alpha, float FontAlpha, int Corners, float r, float FontFactor)
{
	return 0;
}

void CMenus::DoButton_MenuTabTop_Dummy(const char *pText, int Checked, const CUIRect *pRect, float Alpha)
{

}

int CMenus::DoButton_GridHeader(const void *pID, const char *pText, int Checked, const CUIRect *pRect)
{
	return 0;
}

int CMenus::DoButton_GridHeaderIcon(CButtonContainer *pBC, int ImageID, int SpriteID, const CUIRect *pRect, int Corners)
{
	return 0;
}

int CMenus::DoButton_CheckBox_Common(const void *pID, const char *pText, const char *pBoxText, const CUIRect *pRect, bool Checked)
{
	return 0;
}

int CMenus::DoButton_CheckBox(const void *pID, const char *pText, int Checked, const CUIRect *pRect)
{
	return 0;
}

int CMenus::DoButton_CheckBox_Number(const void *pID, const char *pText, int Checked, const CUIRect *pRect)
{
	return 0;
}

int CMenus::DoButton_SpriteID(CButtonContainer *pBC, int ImageID, int SpriteID, bool Checked, const CUIRect *pRect, int Corners, float r, bool Fade)
{
	return 0;
}

int CMenus::DoButton_SpriteClean(int ImageID, int SpriteID, const CUIRect *pRect)
{
	return 0;
}

int CMenus::DoButton_SpriteCleanID(const void *pID, int ImageID, int SpriteID, const CUIRect *pRect, bool Blend)
{
	return 0;
}

int CMenus::DoButton_MouseOver(int ImageID, int SpriteID, const CUIRect *pRect)
{
	return 0;
}

int CMenus::DoEditBox(void *pID, const CUIRect *pRect, char *pStr, unsigned StrSize, float FontSize, float *pOffset, bool Hidden, int Corners)
{
	return 0;
}

void CMenus::DoEditBoxOption(void *pID, char *pOption, int OptionLength, const CUIRect *pRect, const char *pStr,  float VSplitVal, float *pOffset, bool Hidden)
{

}

void CMenus::DoScrollbarOption(void *pID, int *pOption, const CUIRect *pRect, const char *pStr, float VSplitVal, int Min, int Max, bool infinite)
{

}

float CMenus::DoDropdownMenu(void *pID, const CUIRect *pRect, const char *pStr, float HeaderHeight, FDropdownCallback pfnCallback)
{
	return 0.0f;
}

float CMenus::DoIndependentDropdownMenu(void *pID, const CUIRect *pRect, const char *pStr, float HeaderHeight, FDropdownCallback pfnCallback, bool* pActive)
{
	return 0.0f;
}

void CMenus::DoInfoBox(const CUIRect *pRect, const char *pLabel, const char *pValue)
{

}

float CMenus::DoScrollbarV(const void *pID, const CUIRect *pRect, float Current)
{
	return 0.0f;
}

float CMenus::DoScrollbarH(const void *pID, const CUIRect *pRect, float Current)
{
	return 0.0f;
}

void CMenus::UiDoListboxHeader(CListBoxState* pState, const CUIRect *pRect, const char *pTitle,
							   float HeaderHeight, float Spacing)
{

}

void CMenus::UiDoListboxStart(CListBoxState* pState, const void *pID, float RowHeight,
							  const char *pBottomText, int NumItems, int ItemsPerRow, int SelectedIndex,
							  const CUIRect *pRect, bool Background)
{

}

int CMenus::UiDoListboxEnd(CListBoxState* pState, bool *pItemActivated)
{
	return 0;
}

int CMenus::DoKeyReader(CButtonContainer *pBC, const CUIRect *pRect, int Key)
{
	return 0;
}

void CMenus::RenderMenubar(CUIRect Rect)
{

}

void CMenus::RenderLoading()
{

}

void CMenus::RenderNews(CUIRect MainView)
{

}

void CMenus::RenderBackButton(CUIRect MainView)
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
