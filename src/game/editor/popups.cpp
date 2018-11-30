/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <base/color.h>
#include <base/tl/array.h>

#include <engine/console.h>
#include <engine/input.h>
#include <engine/keys.h>
#include <engine/storage.h>

#include "editor.h"


void CEditor::UiDoPopupMenu()
{

}


void CEditor::PopupSelectImageInvoke(int Current, float x, float y)
{

}

int CEditor::PopupSelectImageResult()
{
	return 0;
}

void CEditor::PopupSelectGametileOpInvoke(float x, float y)
{

}

int CEditor::PopupSelectGameTileOpResult()
{
	return 0;
}

static bool s_AutoMapProceedOrder = false;

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
