/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <base/color.h>
#include <base/tl/array.h>

#include <engine/console.h>
#include <engine/input.h>
#include <engine/keys.h>
#include <engine/storage.h>

#include "editor.h"


// popup menu handling
static struct
{
	CUIRect m_Rect;
	void *m_pId;
	int (*m_pfnFunc)(CEditor *pEditor, CUIRect Rect);
	int m_IsMenu;
	void *m_pExtra;
} s_UiPopups[8];

static int g_UiNumPopups = 0;

void CEditor::UiInvokePopupMenu(void *pID, int Flags, float x, float y, float Width, float Height, int (*pfnFunc)(CEditor *pEditor, CUIRect Rect), void *pExtra)
{
	Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "editor", "invoked");
	if(x + Width > UI()->Screen()->w)
		x -= Width;
	if(y + Height > UI()->Screen()->h)
		y -= Height;
	s_UiPopups[g_UiNumPopups].m_pId = pID;
	s_UiPopups[g_UiNumPopups].m_IsMenu = Flags;
	s_UiPopups[g_UiNumPopups].m_Rect.x = x;
	s_UiPopups[g_UiNumPopups].m_Rect.y = y;
	s_UiPopups[g_UiNumPopups].m_Rect.w = Width;
	s_UiPopups[g_UiNumPopups].m_Rect.h = Height;
	s_UiPopups[g_UiNumPopups].m_pfnFunc = pfnFunc;
	s_UiPopups[g_UiNumPopups].m_pExtra = pExtra;
	g_UiNumPopups++;
}

void CEditor::UiDoPopupMenu()
{

}


int CEditor::PopupGroup(CEditor *pEditor, CUIRect View)
{
	return 0;
}

int CEditor::PopupLayer(CEditor *pEditor, CUIRect View)
{
	return 0;
}

int CEditor::PopupQuad(CEditor *pEditor, CUIRect View)
{
	return 0;
}

int CEditor::PopupPoint(CEditor *pEditor, CUIRect View)
{
	return 0;
}

int CEditor::PopupNewFolder(CEditor *pEditor, CUIRect View)
{
	return 0;
}

int CEditor::PopupMapInfo(CEditor *pEditor, CUIRect View)
{
	return 0;
}

int CEditor::PopupEvent(CEditor *pEditor, CUIRect View)
{
	return 0;
}


static int g_SelectImageSelected = -100;
static int g_SelectImageCurrent = -100;

int CEditor::PopupSelectImage(CEditor *pEditor, CUIRect View)
{
	return 0;
}

void CEditor::PopupSelectImageInvoke(int Current, float x, float y)
{

}

int CEditor::PopupSelectImageResult()
{
	if(g_SelectImageSelected == -100)
		return -100;

	g_SelectImageCurrent = g_SelectImageSelected;
	g_SelectImageSelected = -100;
	return g_SelectImageCurrent;
}

static int s_GametileOpSelected = -1;

int CEditor::PopupSelectGametileOp(CEditor *pEditor, CUIRect View)
{
	return 0;
}

void CEditor::PopupSelectGametileOpInvoke(float x, float y)
{

}

int CEditor::PopupSelectGameTileOpResult()
{
	if(s_GametileOpSelected < 0)
		return -1;

	int Result = s_GametileOpSelected;
	s_GametileOpSelected = -1;
	return Result;
}

static bool s_AutoMapProceedOrder = false;

int CEditor::PopupDoodadAutoMap(CEditor *pEditor, CUIRect View)
{
	return 0;
}

int CEditor::PopupSelectDoodadRuleSet(CEditor *pEditor, CUIRect View)
{
	return 0;
}

int CEditor::PopupSelectConfigAutoMap(CEditor *pEditor, CUIRect View)
{
	return 0;
}

void CEditor::PopupSelectConfigAutoMapInvoke(float x, float y)
{

}

bool CEditor::PopupAutoMapProceedOrder()
{
	if(s_AutoMapProceedOrder)
	{
		// reset
		s_AutoMapProceedOrder = false;
		return true;
	}

	return false;
}

int CEditor::PopupColorPicker(CEditor *pEditor, CUIRect View)
{
	return 0;
}
