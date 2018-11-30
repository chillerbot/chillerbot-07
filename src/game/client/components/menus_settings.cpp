/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <base/color.h>
#include <base/math.h>

#include <engine/engine.h>
#include <engine/storage.h>
#include <engine/external/json-parser/json.h>
#include <engine/shared/config.h>

#include <generated/protocol.h>
#include <generated/client_data.h>

#include <game/client/components/maplayers.h>
#include <game/client/components/sounds.h>
#include <game/client/gameclient.h>
#include <game/client/animstate.h>

#include "binds.h"
#include "countryflags.h"
#include "menus.h"

CMenusKeyBinder CMenus::m_Binder;

CMenusKeyBinder::CMenusKeyBinder()
{
	m_TakeKey = false;
	m_GotKey = false;
}

bool CMenusKeyBinder::OnInput(IInput::CEvent Event)
{
	if(m_TakeKey)
	{
		if(Event.m_Flags&IInput::FLAG_PRESS)
		{
			m_Key = Event;
			m_GotKey = true;
			m_TakeKey = false;
		}
		return true;
	}

	return false;
}

int CMenus::DoButton_Customize(CButtonContainer *pBC, IGraphics::CTextureHandle Texture, int SpriteID, const CUIRect *pRect, float ImageRatio)
{
	return 0;
}

void CMenus::SaveSkinfile()
{

}

void CMenus::RenderHSLPicker(CUIRect MainView)
{

}

void CMenus::RenderSkinSelection(CUIRect MainView)
{

}

void CMenus::RenderSkinPartSelection(CUIRect MainView)
{

}

class CLanguage
{
public:
	CLanguage() {}
	CLanguage(const char *n, const char *f, int Code) : m_Name(n), m_FileName(f), m_CountryCode(Code) {}

	string m_Name;
	string m_FileName;
	int m_CountryCode;

	bool operator<(const CLanguage &Other) { return m_Name < Other.m_Name; }
};


int CMenus::ThemeScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	return 0;
}

int CMenus::ThemeIconScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	return 0; // no existing theme
}

void LoadLanguageIndexfile(IStorage *pStorage, IConsole *pConsole, sorted_array<CLanguage> *pLanguages)
{
	// read file data into buffer
	const char *pFilename = "languages/index.json";
	IOHANDLE File = pStorage->OpenFile(pFilename, IOFLAG_READ, IStorage::TYPE_ALL);
	if(!File)
	{
		pConsole->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "localization", "couldn't open index file");
		return;
	}
	int FileSize = (int)io_length(File);
	char *pFileData = (char *)mem_alloc(FileSize, 1);
	io_read(File, pFileData, FileSize);
	io_close(File);

	// parse json data
	json_settings JsonSettings;
	mem_zero(&JsonSettings, sizeof(JsonSettings));
	char aError[256];
	json_value *pJsonData = json_parse_ex(&JsonSettings, pFileData, FileSize, aError);
	mem_free(pFileData);

	if(pJsonData == 0)
	{
		pConsole->Print(IConsole::OUTPUT_LEVEL_ADDINFO, pFilename, aError);
		return;
	}

	// extract data
	const json_value &rStart = (*pJsonData)["language indices"];
	if(rStart.type == json_array)
	{
		for(unsigned i = 0; i < rStart.u.array.length; ++i)
		{
			char aFileName[128];
			str_format(aFileName, sizeof(aFileName), "languages/%s.json", (const char *)rStart[i]["file"]);
			pLanguages->add(CLanguage((const char *)rStart[i]["name"], aFileName, (json_int_t)rStart[i]["code"]));
		}
	}

	// clean up
	json_value_free(pJsonData);
}

void CMenus::RenderLanguageSelection(CUIRect MainView, bool Header)
{

}

void CMenus::RenderThemeSelection(CUIRect MainView, bool Header)
{

}

void CMenus::RenderSettingsGeneral(CUIRect MainView)
{

}

void CMenus::RenderSettingsPlayer(CUIRect MainView)
{

}

void CMenus::RenderSettingsTeeCustom(CUIRect MainView)
{

}

void CMenus::RenderSettingsTee(CUIRect MainView)
{

}

void CMenus::RenderSettingsControls(CUIRect MainView)
{

}

bool CMenus::DoResolutionList(CUIRect* pRect, CListBoxState* pListBoxState,
							  const sorted_array<CVideoMode>& lModes)
{
	return false;
}

void CMenus::RenderSettingsGraphics(CUIRect MainView)
{

}

void CMenus::RenderSettingsSound(CUIRect MainView)
{

}

void CMenus::RenderSettings(CUIRect MainView)
{

}
