/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <algorithm> // sort  TODO: remove this

#include <engine/external/json-parser/json.h>

#include <engine/config.h>
#include <engine/friends.h>
#include <engine/keys.h>
#include <engine/serverbrowser.h>
#include <engine/storage.h>
#include <engine/shared/config.h>

#include <generated/client_data.h>
#include <generated/protocol.h>

#include <game/version.h>
#include <game/client/render.h>
#include <game/client/ui.h>
#include <game/client/components/countryflags.h>

#include "menus.h"

CMenus::CColumn CMenus::ms_aBrowserCols[] = {
	{COL_BROWSER_FLAG,		-1,									" ",		-1, 87.0f, 0, {0}, {0}}, // Localize - these strings are localized within CLocConstString
	{COL_BROWSER_NAME,		IServerBrowser::SORT_NAME,			"Server",		0, 300.0f, 0, {0}, {0}},
	{COL_BROWSER_GAMETYPE,	IServerBrowser::SORT_GAMETYPE,		"Type",		1, 70.0f, 0, {0}, {0}},
	{COL_BROWSER_MAP,		IServerBrowser::SORT_MAP,			"Map",		1, 100.0f, 0, {0}, {0}},
	{COL_BROWSER_PLAYERS,	IServerBrowser::SORT_NUMPLAYERS,	"Players",	1, 60.0f, 0, {0}, {0}},
	{COL_BROWSER_PING,		IServerBrowser::SORT_PING,			"Ping",		1, 40.0f, 0, {0}, {0}},
};

CServerFilterInfo CMenus::CBrowserFilter::ms_FilterStandard = {IServerBrowser::FILTER_COMPAT_VERSION|IServerBrowser::FILTER_PURE|IServerBrowser::FILTER_PURE_MAP, 999, -1, 0, {0}, 0};
CServerFilterInfo CMenus::CBrowserFilter::ms_FilterFavorites = {IServerBrowser::FILTER_COMPAT_VERSION|IServerBrowser::FILTER_FAVORITE, 999, -1, 0, {0}, 0};
CServerFilterInfo CMenus::CBrowserFilter::ms_FilterAll = {IServerBrowser::FILTER_COMPAT_VERSION, 999, -1, 0, {0}, 0};


// filters
CMenus::CBrowserFilter::CBrowserFilter(int Custom, const char* pName, IServerBrowser *pServerBrowser)
{
	m_Extended = false;
	m_Custom = Custom;
	str_copy(m_aName, pName, sizeof(m_aName));
	m_pServerBrowser = pServerBrowser;
	switch(m_Custom)
	{
	case CBrowserFilter::FILTER_STANDARD:
		m_Filter = m_pServerBrowser->AddFilter(&ms_FilterStandard);
		break;
	case CBrowserFilter::FILTER_FAVORITES:
		m_Filter = m_pServerBrowser->AddFilter(&ms_FilterFavorites);
		break;
	default:
		m_Filter = m_pServerBrowser->AddFilter(&ms_FilterAll);
	}

	// init buttons
	m_SwitchButton = 0;
}

void CMenus::CBrowserFilter::Reset()
{
	switch(m_Custom)
	{
	case CBrowserFilter::FILTER_STANDARD:
		m_pServerBrowser->SetFilter(m_Filter, &ms_FilterStandard);
		break;
	case CBrowserFilter::FILTER_FAVORITES:
		m_pServerBrowser->SetFilter(m_Filter, &ms_FilterFavorites);
		break;
	default:
		m_pServerBrowser->SetFilter(m_Filter, &ms_FilterAll);
	}
}

void CMenus::CBrowserFilter::Switch()
{
	m_Extended ^= 1;
}

bool CMenus::CBrowserFilter::Extended() const
{
	return m_Extended;
}

int CMenus::CBrowserFilter::Custom() const
{
	return m_Custom;
}

int CMenus::CBrowserFilter::Filter() const
{
	return m_Filter;
}

const char* CMenus::CBrowserFilter::Name() const
{
	return m_aName;
}

const void *CMenus::CBrowserFilter::ID(int Index) const
{
	return m_pServerBrowser->GetID(m_Filter, Index);
}

int CMenus::CBrowserFilter::NumSortedServers() const
{
	return m_pServerBrowser->NumSortedServers(m_Filter);
}

int CMenus::CBrowserFilter::NumPlayers() const
{
	return m_pServerBrowser->NumSortedPlayers(m_Filter);
}

const CServerInfo *CMenus::CBrowserFilter::SortedGet(int Index) const
{
	if(Index < 0 || Index >= m_pServerBrowser->NumSortedServers(m_Filter))
		return 0;
	return m_pServerBrowser->SortedGet(m_Filter, Index);
}

void CMenus::CBrowserFilter::SetFilterNum(int Num)
{
	m_Filter = Num;
}

void CMenus::CBrowserFilter::GetFilter(CServerFilterInfo *pFilterInfo) const
{
	m_pServerBrowser->GetFilter(m_Filter, pFilterInfo);
}

void CMenus::CBrowserFilter::SetFilter(const CServerFilterInfo *pFilterInfo)
{
	m_pServerBrowser->SetFilter(m_Filter, pFilterInfo);
}

void CMenus::LoadFilters()
{
	// read file data into buffer
	const char *pFilename = "ui_settings.json";
	IOHANDLE File = Storage()->OpenFile(pFilename, IOFLAG_READ, IStorage::TYPE_ALL);
	if(!File)
		return;
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
		Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, pFilename, aError);
		return;
	}

	// extract settings data
	const json_value &rSettingsEntry = (*pJsonData)["settings"];
	if(rSettingsEntry["sidebar_active"].type == json_integer)
		m_SidebarActive = rSettingsEntry["sidebar_active"].u.integer;
	if(rSettingsEntry["sidebar_tab"].type == json_integer)
		m_SidebarTab = clamp(int(rSettingsEntry["sidebar_tab"].u.integer), 0, 2);

	// extract filter data
	int Extended = 0;
	const json_value &rFilterEntry = (*pJsonData)["filter"];
	for(unsigned i = 0; i < rFilterEntry.u.array.length; ++i)
	{
		char *pName = rFilterEntry[i].u.object.values[0].name;
		const json_value &rStart = *(rFilterEntry[i].u.object.values[0].value);
		if(rStart.type != json_object)
			continue;

		int Type = 0;
		if(rStart["type"].type == json_integer)
			Type = rStart["type"].u.integer;
		if(rStart["extended"].type == json_integer && rStart["extended"].u.integer)
			Extended = i;

		// filter setting
		CServerFilterInfo FilterInfo;
		for(int j = 0; j < CServerFilterInfo::MAX_GAMETYPES; ++j)
			FilterInfo.m_aGametype[j][0] = 0;
		const json_value &rSubStart = rStart["settings"];
		if(rSubStart.type == json_object)
		{
			if(rSubStart["filter_hash"].type == json_integer)
				FilterInfo.m_SortHash = rSubStart["filter_hash"].u.integer;
			const json_value &rGametypeEntry = rSubStart["filter_gametype"];
			if(rGametypeEntry.type == json_array)
			{
				for(unsigned j = 0; j < rGametypeEntry.u.array.length && j < CServerFilterInfo::MAX_GAMETYPES; ++j)
				{
					if(rGametypeEntry[j].type == json_string)
						str_copy(FilterInfo.m_aGametype[j], rGametypeEntry[j], sizeof(FilterInfo.m_aGametype[j]));
				}
			}
			if(rSubStart["filter_ping"].type == json_integer)
				FilterInfo.m_Ping = rSubStart["filter_ping"].u.integer;
			if(rSubStart["filter_serverlevel"].type == json_integer)
				FilterInfo.m_ServerLevel = rSubStart["filter_serverlevel"].u.integer;
			if(rSubStart["filter_address"].type == json_string)
				str_copy(FilterInfo.m_aAddress, rSubStart["filter_address"], sizeof(FilterInfo.m_aAddress));
			if(rSubStart["filter_country"].type == json_integer)
				FilterInfo.m_Country = rSubStart["filter_country"].u.integer;
		}

		// add filter
		m_lFilters.add(CBrowserFilter(Type, pName, ServerBrowser()));
		if(Type == CBrowserFilter::FILTER_STANDARD)		//	make sure the pure filter is enabled in the Teeworlds-filter
			FilterInfo.m_SortHash |= IServerBrowser::FILTER_PURE;
		m_lFilters[i].SetFilter(&FilterInfo);
	}

	// clean up
	json_value_free(pJsonData);

	m_lFilters[Extended].Switch();
}

void CMenus::SaveFilters()
{
	IOHANDLE File = Storage()->OpenFile("ui_settings.json", IOFLAG_WRITE, IStorage::TYPE_SAVE);
	if(!File)
		return;

	char aBuf[512];

	// settings
	const char *p = "{\"settings\": {\n";
	io_write(File, p, str_length(p));

	str_format(aBuf, sizeof(aBuf), "\t\"sidebar_active\": %d,\n", m_SidebarActive);
	io_write(File, aBuf, str_length(aBuf));
	str_format(aBuf, sizeof(aBuf), "\t\"sidebar_tab\": %d,\n", m_SidebarTab);
	io_write(File, aBuf, str_length(aBuf));

	// settings end
	p = "\t},\n";
	io_write(File, p, str_length(p));
	
	// filter
	p = " \"filter\": [";
	io_write(File, p, str_length(p));

	for(int i = 0; i < m_lFilters.size(); i++)
	{
		// part start
		if(i == 0)
			p = "\n";
		else
			p = ",\n"; 
		io_write(File, p, str_length(p));

		str_format(aBuf, sizeof(aBuf), "\t{\"%s\": {\n", m_lFilters[i].Name());
		io_write(File, aBuf, str_length(aBuf));

		str_format(aBuf, sizeof(aBuf), "\t\t\"type\": %d,\n", m_lFilters[i].Custom());
		io_write(File, aBuf, str_length(aBuf));
		str_format(aBuf, sizeof(aBuf), "\t\t\"extended\": %d,\n", m_lFilters[i].Extended()?1:0);
		io_write(File, aBuf, str_length(aBuf));

		// filter setting
		CServerFilterInfo FilterInfo;
		m_lFilters[i].GetFilter(&FilterInfo);

		str_format(aBuf, sizeof(aBuf), "\t\t\"settings\": {\n");
		io_write(File, aBuf, str_length(aBuf));
		str_format(aBuf, sizeof(aBuf), "\t\t\t\"filter_hash\": %d,\n", FilterInfo.m_SortHash);
		io_write(File, aBuf, str_length(aBuf));
		str_format(aBuf, sizeof(aBuf), "\t\t\t\"filter_gametype\": [");
		io_write(File, aBuf, str_length(aBuf));
		for(unsigned j = 0; j < CServerFilterInfo::MAX_GAMETYPES && FilterInfo.m_aGametype[j][0]; ++j)
		{
			str_format(aBuf, sizeof(aBuf), "\n\t\t\t\t\"%s\",", FilterInfo.m_aGametype[j]);
			io_write(File, aBuf, str_length(aBuf));
		}
		p = "\n\t\t\t],\n";
		io_write(File, p, str_length(p));
		str_format(aBuf, sizeof(aBuf), "\t\t\t\"filter_ping\": %d,\n", FilterInfo.m_Ping);
		io_write(File, aBuf, str_length(aBuf));
		str_format(aBuf, sizeof(aBuf), "\t\t\t\"filter_serverlevel\": %d,\n", FilterInfo.m_ServerLevel);
		io_write(File, aBuf, str_length(aBuf));
		str_format(aBuf, sizeof(aBuf), "\t\t\t\"filter_address\": \"%s\",\n", FilterInfo.m_aAddress);
		io_write(File, aBuf, str_length(aBuf));
		str_format(aBuf, sizeof(aBuf), "\t\t\t\"filter_country\": %d,\n\t\t\t}", FilterInfo.m_Country);
		io_write(File, aBuf, str_length(aBuf));

		// part end
		p = "\n\t\t}\n\t}";
		io_write(File, p, str_length(p));
	}

	// filter end
	p = "]\n}";
	io_write(File, p, str_length(p));

	io_close(File);
}

void CMenus::RemoveFilter(int FilterIndex)
{
	int Filter = m_lFilters[FilterIndex].Filter();
	ServerBrowser()->RemoveFilter(Filter);
	m_lFilters.remove_index(FilterIndex);

	// update filter indexes
	for(int i = 0; i < m_lFilters.size(); i++)
	{
		CBrowserFilter *pFilter = &m_lFilters[i];
		if(pFilter->Filter() > Filter)
			pFilter->SetFilterNum(pFilter->Filter()-1);
	}
}

void CMenus::Move(bool Up, int Filter)
{
	// move up
	CBrowserFilter Temp = m_lFilters[Filter];
	if(Up)
	{
		if(Filter > 0)
		{
			m_lFilters[Filter] = m_lFilters[Filter-1];
			m_lFilters[Filter-1] = Temp;
		}
	}
	else // move down
	{
		if(Filter < m_lFilters.size()-1)
		{
			m_lFilters[Filter] = m_lFilters[Filter+1];
			m_lFilters[Filter+1] = Temp;
		}
	}
}

void CMenus::SetOverlay(int Type, float x, float y, const void *pData)
{
	if(!m_PopupActive && m_InfoOverlay.m_Reset)
	{
		m_InfoOverlayActive = true;
		m_InfoOverlay.m_Type = Type;
		m_InfoOverlay.m_X = x;
		m_InfoOverlay.m_Y = y;
		m_InfoOverlay.m_pData = pData;
		m_InfoOverlay.m_Reset = false;
	}
}

// 1 = browser entry click, 2 = server info click
int CMenus::DoBrowserEntry(const void *pID, CUIRect View, const CServerInfo *pEntry, const CBrowserFilter *pFilter, bool Selected)
{
<<<<<<< HEAD
	// logic
	int ReturnValue = 0;
	int Inside = UI()->MouseInside(&View);

	if(UI()->CheckActiveItem(pID))
	{
		if(!UI()->MouseButton(0))
		{
			if(Inside >= 0)
				ReturnValue = 1;
			UI()->SetActiveItem(0);
		}
	}
	if(UI()->HotItem() == pID)
	{
		if(UI()->MouseButton(0))
			UI()->SetActiveItem(pID);

		CUIRect r = View;
		RenderTools()->DrawUIRect(&r, vec4(1.0f, 1.0f, 1.0f, 0.5f), CUI::CORNER_ALL, 4.0f);
	}

	if(Inside)
		UI()->SetHotItem(pID);

	vec3 TextBaseColor = vec3(1.0f, 1.0f, 1.0f);
	if(Selected || Inside)
	{
		TextBaseColor = vec3(0.0f, 0.0f, 0.0f);
		TextRender()->TextOutlineColor(1.0f, 1.0f, 1.0f, 0.25f);
	}

	float TextAlpha = (pEntry->m_NumClients == pEntry->m_MaxClients) ? 0.5f : 1.0f;
	for(int c = 0; c < NUM_BROWSER_COLS; c++)
	{
		CUIRect Button = ms_aBrowserCols[c].m_Rect;
		char aTemp[64];
		Button.x = ms_aBrowserCols[c].m_Rect.x;
		Button.y = View.y;
		Button.h = ms_aBrowserCols[c].m_Rect.h;
		Button.w = ms_aBrowserCols[c].m_Rect.w;

		int ID = ms_aBrowserCols[c].m_ID;

		if(ID == COL_BROWSER_FLAG)
		{
			CUIRect Rect = Button;
			CUIRect Icon;

			Rect.VSplitLeft(Rect.h, &Icon, &Rect);
			if(pEntry->m_Flags&IServerBrowser::FLAG_PASSWORD)
			{
				Icon.Margin(2.0f, &Icon);
				DoIcon(IMAGE_BROWSEICONS, Selected ? SPRITE_BROWSE_LOCK_B : SPRITE_BROWSE_LOCK_A, &Icon);
			}

			Rect.VSplitLeft(Rect.h, &Icon, &Rect);
			Icon.Margin(2.0f, &Icon);
			int Level = pEntry->m_ServerLevel;

			DoIcon(IMAGE_LEVELICONS, Level==0 ? (Selected ? SPRITE_LEVEL_A_B : SPRITE_LEVEL_A_A) : Level==1 ? (Selected ? SPRITE_LEVEL_B_B : SPRITE_LEVEL_B_A) : (Selected ? SPRITE_LEVEL_C_B : SPRITE_LEVEL_C_A), &Icon);

			Rect.VSplitLeft(Rect.h, &Icon, &Rect);
			Icon.Margin(2.0f, &Icon);
			if(DoButton_SpriteClean(IMAGE_BROWSEICONS, pEntry->m_Favorite ? SPRITE_BROWSE_STAR_A : SPRITE_BROWSE_STAR_B, &Icon))
			{
				if(!pEntry->m_Favorite)
					ServerBrowser()->AddFavorite(pEntry);
				else
					ServerBrowser()->RemoveFavorite(pEntry);
			}

			Rect.VSplitLeft(Rect.h, &Icon, &Rect);
			if(pEntry->m_FriendState != IFriends::FRIEND_NO)
			{
				Icon.Margin(2.0f, &Icon);
				DoIcon(IMAGE_BROWSEICONS, Selected ? SPRITE_BROWSE_HEART_B : SPRITE_BROWSE_HEART_A, &Icon);
			}
		}
		else if(ID == COL_BROWSER_NAME)
		{
			CTextCursor Cursor;
			float tw = TextRender()->TextWidth(0, 12.0f, pEntry->m_aName, -1);
			if(tw < Button.w)
				TextRender()->SetCursor(&Cursor, Button.x+Button.w/2.0f-tw/2.0f, Button.y, 12.0f, TEXTFLAG_RENDER);
			else
			{
				TextRender()->SetCursor(&Cursor, Button.x, Button.y, 12.0f, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
				Cursor.m_LineWidth = Button.w;
			}
			
			TextRender()->TextColor(TextBaseColor.r, TextBaseColor.g, TextBaseColor.b, TextAlpha);

			if(g_Config.m_BrFilterString[0] && (pEntry->m_QuickSearchHit&IServerBrowser::QUICK_SERVERNAME))
			{
				// highlight the parts that matches
				const char *pStr = str_find_nocase(pEntry->m_aName, g_Config.m_BrFilterString);
				if(pStr)
				{
					TextRender()->TextEx(&Cursor, pEntry->m_aName, (int)(pStr-pEntry->m_aName));
					TextRender()->TextColor(0.4f, 0.4f, 1.0f, TextAlpha);
					TextRender()->TextEx(&Cursor, pStr, str_length(g_Config.m_BrFilterString));
					TextRender()->TextColor(TextBaseColor.r, TextBaseColor.g, TextBaseColor.b, TextAlpha);
					TextRender()->TextEx(&Cursor, pStr+str_length(g_Config.m_BrFilterString), -1);
				}
				else
					TextRender()->TextEx(&Cursor, pEntry->m_aName, -1);
			}
			else
				TextRender()->TextEx(&Cursor, pEntry->m_aName, -1);
		}
		else if(ID == COL_BROWSER_MAP)
		{
			CTextCursor Cursor;
			float tw = TextRender()->TextWidth(0, 12.0f, pEntry->m_aMap, -1);
			if(tw < Button.w)
				TextRender()->SetCursor(&Cursor, Button.x+Button.w/2.0f-tw/2.0f, Button.y, 12.0f, TEXTFLAG_RENDER);
			else
			{
				TextRender()->SetCursor(&Cursor, Button.x, Button.y, 12.0f, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
				Cursor.m_LineWidth = Button.w;
			}

			TextRender()->TextColor(TextBaseColor.r, TextBaseColor.g, TextBaseColor.b, TextAlpha);

			if(g_Config.m_BrFilterString[0] && (pEntry->m_QuickSearchHit&IServerBrowser::QUICK_MAPNAME))
			{
				// highlight the parts that matches
				const char *pStr = str_find_nocase(pEntry->m_aMap, g_Config.m_BrFilterString);
				if(pStr)
				{
					TextRender()->TextEx(&Cursor, pEntry->m_aMap, (int)(pStr-pEntry->m_aMap));
					TextRender()->TextColor(0.4f, 0.4f, 1.0f, TextAlpha);
					TextRender()->TextEx(&Cursor, pStr, str_length(g_Config.m_BrFilterString));
					TextRender()->TextColor(TextBaseColor.r, TextBaseColor.g, TextBaseColor.b, TextAlpha);
					TextRender()->TextEx(&Cursor, pStr+str_length(g_Config.m_BrFilterString), -1);
				}
				else
					TextRender()->TextEx(&Cursor, pEntry->m_aMap, -1);
			}
			else
				TextRender()->TextEx(&Cursor, pEntry->m_aMap, -1);
		}
		else if(ID == COL_BROWSER_PLAYERS)
		{
			TextRender()->TextColor(TextBaseColor.r, TextBaseColor.g, TextBaseColor.b, TextAlpha);
			CServerFilterInfo FilterInfo;
			pFilter->GetFilter(&FilterInfo);

			int Num = (FilterInfo.m_SortHash&IServerBrowser::FILTER_SPECTATORS) ? pEntry->m_NumPlayers : pEntry->m_NumClients;
			int Max = (FilterInfo.m_SortHash&IServerBrowser::FILTER_SPECTATORS) ? pEntry->m_MaxPlayers : pEntry->m_MaxClients;
			if(FilterInfo.m_SortHash&IServerBrowser::FILTER_SPECTATORS)
			{
				int SpecNum = pEntry->m_NumClients - pEntry->m_NumPlayers;
				if(pEntry->m_MaxClients - pEntry->m_MaxPlayers < SpecNum)
					Max -= SpecNum;
			}
			if(FilterInfo.m_SortHash&IServerBrowser::FILTER_BOTS)
			{
				Num -= pEntry->m_NumBotPlayers;
				Max -= pEntry->m_NumBotPlayers;
				if(!(FilterInfo.m_SortHash&IServerBrowser::FILTER_SPECTATORS))
				{
					Num -= pEntry->m_NumBotSpectators;
					Max -= pEntry->m_NumBotSpectators;
				}

			}
			str_format(aTemp, sizeof(aTemp), "%d/%d", Num, Max);
			if(g_Config.m_BrFilterString[0] && (pEntry->m_QuickSearchHit&IServerBrowser::QUICK_PLAYER))
				TextRender()->TextColor(0.4f, 0.4f, 1.0f, TextAlpha);
			Button.y += 2.0f;
			UI()->DoLabel(&Button, aTemp, 12.0f, CUI::ALIGN_CENTER);
		}
		else if(ID == COL_BROWSER_PING)
		{
			int Ping = pEntry->m_Latency;

			vec4 Color;
			if(Selected || Inside)
			{
				Color = vec4(TextBaseColor.r, TextBaseColor.g, TextBaseColor.b, TextAlpha);
			}
			else
			{
				vec4 StartColor;
				vec4 EndColor;
				float MixVal;
				if(Ping <= 125)
				{
					StartColor = vec4(0.0f, 1.0f, 0.0f, TextAlpha);
					EndColor = vec4(1.0f, 1.0f, 0.0f, TextAlpha);
					
					MixVal = (Ping-50.0f)/75.0f;
				}
				else
				{
					StartColor = vec4(1.0f, 1.0f, 0.0f, TextAlpha);
					EndColor = vec4(1.0f, 0.0f, 0.0f, TextAlpha);
					
					MixVal = (Ping-125.0f)/75.0f;
				}
				Color = mix(StartColor, EndColor, MixVal);
			}
			
			str_format(aTemp, sizeof(aTemp), "%d", pEntry->m_Latency);
			TextRender()->TextColor(Color.r, Color.g, Color.b, Color.a);
			Button.y += 2.0f;
			UI()->DoLabel(&Button, aTemp, 12.0f, CUI::ALIGN_CENTER);
		}
		else if(ID == COL_BROWSER_GAMETYPE)
		{
			// gametype icon
			CUIRect Icon;
			Button.VSplitLeft(Button.h, &Icon, &Button);
			Icon.y -= 0.5f;
			/*if(!(pEntry->m_Flags&IServerBrowser::FLAG_PURE))
			{
				DoIcon(IMAGE_BROWSEICONS, Selected ? SPRITE_BROWSE_UNPURE_B : SPRITE_BROWSE_UNPURE_A, &Icon);
			}
			else*/
			{
				DoGameIcon(pEntry->m_aGameType, &Icon, Selected ? CGameIcon::GAMEICON_OFF : CGameIcon::GAMEICON_ON);
			}

			// gametype text
			CTextCursor Cursor;
			{
				TextRender()->SetCursor(&Cursor, Button.x, Button.y, 12.0f, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
				Cursor.m_LineWidth = Button.w;
			}

			TextRender()->TextColor(TextBaseColor.r, TextBaseColor.g, TextBaseColor.b, TextAlpha);
			TextRender()->TextEx(&Cursor, pEntry->m_aGameType, -1);
		}
	}

	// show server info
	if(!m_SidebarActive && m_ShowServerDetails && Selected)
	{
		CUIRect Info;
		View.HSplitTop(ms_aBrowserCols[0].m_Rect.h, 0, &View);
		View.VSplitLeft(160.0f, &Info, &View);
		RenderDetailInfo(Info, pEntry);

		RenderDetailScoreboard(View, pEntry, 4);

		if(ReturnValue && UI()->MouseInside(&View))
			ReturnValue++;
	}

	TextRender()->TextOutlineColor(0.0f, 0.0f, 0.0f, 0.3f);
	TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);

	return ReturnValue;
=======
	return 0;
>>>>>>> Remove sound and graphics from components
}

bool CMenus::RenderFilterHeader(CUIRect View, int FilterIndex)
{
	return false;
}

void CMenus::RenderServerbrowserOverlay()
{

}

void CMenus::RenderServerbrowserServerList(CUIRect View)
{

}

void CMenus::RenderServerbrowserSidebar(CUIRect View)
{

}

void CMenus::RenderServerbrowserFriendTab(CUIRect View)
{

}

void CMenus::RenderServerbrowserFilterTab(CUIRect View)
{
<<<<<<< HEAD
	CUIRect ServerFilter = View, FilterHeader, Button, Icon, Label;
	const float FontSize = 10.0f;
	const float LineSize = 16.0f;

	// new filter
	ServerFilter.HSplitBottom(LineSize, &ServerFilter, &Button);
	Button.VSplitLeft(60.0f, &Icon, &Button);
	static char s_aFilterName[32] = { 0 };
	static float s_FilterOffset = 0.0f;
	static int s_EditFilter = 0;
	DoEditBox(&s_EditFilter, &Icon, s_aFilterName, sizeof(s_aFilterName), FontSize, &s_FilterOffset, false, CUI::CORNER_L);
	RenderTools()->DrawUIRect(&Button, vec4(1.0f, 1.0f, 1.0f, 0.25f), CUI::CORNER_R, 5.0f);
	Button.VSplitLeft(Button.h, &Icon, &Label);
	Label.HMargin(2.0f, &Label);
	UI()->DoLabelScaled(&Label, Localize("New filter"), FontSize, CUI::ALIGN_LEFT);
	if(s_aFilterName[0])
		DoIcon(IMAGE_FRIENDICONS, UI()->MouseInside(&Button) ? SPRITE_FRIEND_PLUS_A : SPRITE_FRIEND_PLUS_B, &Icon);
	static CButtonContainer s_AddFilter;
	if(s_aFilterName[0] && UI()->DoButtonLogic(&s_AddFilter, "", 0, &Button))
	{
		m_lFilters.add(CBrowserFilter(CBrowserFilter::FILTER_CUSTOM, s_aFilterName, ServerBrowser()));
		s_aFilterName[0] = 0;
	}

	// slected filter
	CBrowserFilter *pFilter = 0;
	for(int i = 0; i < m_lFilters.size(); ++i)
	{
		if(m_lFilters[i].Extended())
		{
			pFilter = &m_lFilters[i];
			m_SelectedFilter = i;
			break;
		}
	}
	if(!pFilter)
		return;

	CServerFilterInfo FilterInfo;
	pFilter->GetFilter(&FilterInfo);

	// server filter
	ServerFilter.HSplitTop(ms_ListheaderHeight, &FilterHeader, &ServerFilter);
	RenderTools()->DrawUIRect(&FilterHeader, vec4(1, 1, 1, 0.25f), CUI::CORNER_T, 4.0f);
	RenderTools()->DrawUIRect(&ServerFilter, vec4(0, 0, 0, 0.15f), CUI::CORNER_B, 4.0f);
	FilterHeader.HMargin(2.0f, &FilterHeader);
	UI()->DoLabel(&FilterHeader, Localize("Server filter"), FontSize + 2.0f, CUI::ALIGN_CENTER);

	int NewSortHash = FilterInfo.m_SortHash;
	ServerFilter.HSplitTop(LineSize, &Button, &ServerFilter);
	static int s_BrFilterEmpty = 0;
	if(DoButton_CheckBox(&s_BrFilterEmpty, Localize("Has people playing"), FilterInfo.m_SortHash&IServerBrowser::FILTER_EMPTY, &Button))
		NewSortHash = FilterInfo.m_SortHash^IServerBrowser::FILTER_EMPTY;

	ServerFilter.HSplitTop(LineSize, &Button, &ServerFilter);
	static int s_BrFilterSpectators = 0;
	if(DoButton_CheckBox(&s_BrFilterSpectators, Localize("Count players only"), FilterInfo.m_SortHash&IServerBrowser::FILTER_SPECTATORS, &Button))
		NewSortHash = FilterInfo.m_SortHash^IServerBrowser::FILTER_SPECTATORS;

	ServerFilter.HSplitTop(LineSize, &Button, &ServerFilter);
	static int s_BrFilterFull = 0;
	if(DoButton_CheckBox(&s_BrFilterFull, Localize("Server not full"), FilterInfo.m_SortHash&IServerBrowser::FILTER_FULL, &Button))
		NewSortHash = FilterInfo.m_SortHash^IServerBrowser::FILTER_FULL;

	ServerFilter.HSplitTop(LineSize, &Button, &ServerFilter);
	static int s_BrFilterFriends = 0;
	if(DoButton_CheckBox(&s_BrFilterFriends, Localize("Show friends only"), FilterInfo.m_SortHash&IServerBrowser::FILTER_FRIENDS, &Button))
		NewSortHash = FilterInfo.m_SortHash^IServerBrowser::FILTER_FRIENDS;

	ServerFilter.HSplitTop(LineSize, &Button, &ServerFilter);
	static int s_BrFilterBots = 0;
	if(DoButton_CheckBox(&s_BrFilterBots, Localize("Hide bots"), FilterInfo.m_SortHash&IServerBrowser::FILTER_BOTS, &Button))
		NewSortHash = FilterInfo.m_SortHash^IServerBrowser::FILTER_BOTS;

	ServerFilter.HSplitTop(LineSize, &Button, &ServerFilter);
	static int s_BrFilterPw = 0;
	if(DoButton_CheckBox(&s_BrFilterPw, Localize("No password"), FilterInfo.m_SortHash&IServerBrowser::FILTER_PW, &Button))
		NewSortHash = FilterInfo.m_SortHash^IServerBrowser::FILTER_PW;

	ServerFilter.HSplitTop(LineSize, &Button, &ServerFilter);
	static int s_BrFilterCompatversion = 0;
	if(DoButton_CheckBox(&s_BrFilterCompatversion, Localize("Compatible version"), FilterInfo.m_SortHash&IServerBrowser::FILTER_COMPAT_VERSION, &Button))
		NewSortHash = FilterInfo.m_SortHash^IServerBrowser::FILTER_COMPAT_VERSION;

	ServerFilter.HSplitTop(LineSize, &Button, &ServerFilter);
	static int s_BrFilterPure = 0;
	if(DoButton_CheckBox(&s_BrFilterPure, Localize("Standard gametype"), FilterInfo.m_SortHash&IServerBrowser::FILTER_PURE, &Button) && pFilter->Custom() != CBrowserFilter::FILTER_STANDARD)
		NewSortHash = FilterInfo.m_SortHash^IServerBrowser::FILTER_PURE;

	ServerFilter.HSplitTop(LineSize, &Button, &ServerFilter);
	static int s_BrFilterPureMap = 0;
	if(DoButton_CheckBox(&s_BrFilterPureMap, Localize("Standard map"), FilterInfo.m_SortHash&IServerBrowser::FILTER_PURE_MAP, &Button))
		NewSortHash = FilterInfo.m_SortHash^IServerBrowser::FILTER_PURE_MAP;

	if(FilterInfo.m_SortHash != NewSortHash)
	{
		FilterInfo.m_SortHash = NewSortHash;
		pFilter->SetFilter(&FilterInfo);
	}

	ServerFilter.HSplitTop(5.0f, 0, &ServerFilter);

	ServerFilter.HSplitTop(LineSize, &Button, &ServerFilter);
	UI()->DoLabelScaled(&Button, Localize("Game types:"), FontSize, CUI::ALIGN_LEFT);
	ServerFilter.HSplitTop(LineSize, &Button, &ServerFilter);
	RenderTools()->DrawUIRect(&Button, vec4(0.0, 0.0, 0.0, 0.25f), CUI::CORNER_ALL, 2.0f);
	Button.HMargin(2.0f, &Button);
	UI()->ClipEnable(&Button);

	float Length = 0.0f;
	for(int i = 0; i < CServerFilterInfo::MAX_GAMETYPES; ++i)
	{
		if(FilterInfo.m_aGametype[i][0])
		{
			Length += TextRender()->TextWidth(0, FontSize, FilterInfo.m_aGametype[i], -1) + 14.0f;
		}
	}
	static float s_ScrollValue = 0.0f;
	bool NeedScrollbar = (Button.w - Length) < 0.0f;
	Button.x += min(0.0f, Button.w - Length) * s_ScrollValue;
	for(int i = 0; i < CServerFilterInfo::MAX_GAMETYPES; ++i)
	{
		if(FilterInfo.m_aGametype[i][0])
		{
			float CurLength = TextRender()->TextWidth(0, FontSize, FilterInfo.m_aGametype[i], -1) + 12.0f;
			Button.VSplitLeft(CurLength, &Icon, &Button);
			RenderTools()->DrawUIRect(&Icon, vec4(0.75, 0.75, 0.75, 0.25f), CUI::CORNER_ALL, 3.0f);
			UI()->DoLabelScaled(&Icon, FilterInfo.m_aGametype[i], FontSize, CUI::ALIGN_LEFT);
			Icon.VSplitRight(10.0f, 0, &Icon);
			if(DoButton_SpriteClean(IMAGE_TOOLICONS, SPRITE_TOOL_X_B, &Icon))
			{
				// remove gametype entry
				if((i == CServerFilterInfo::MAX_GAMETYPES - 1) || !FilterInfo.m_aGametype[i + 1][0])
					FilterInfo.m_aGametype[i][0] = 0;
				else
				{
					int j = i;
					for(; j < CServerFilterInfo::MAX_GAMETYPES - 1 && FilterInfo.m_aGametype[j + 1][0]; ++j)
						str_copy(FilterInfo.m_aGametype[j], FilterInfo.m_aGametype[j + 1], sizeof(FilterInfo.m_aGametype[j]));
					FilterInfo.m_aGametype[j][0] = 0;
				}
				pFilter->SetFilter(&FilterInfo);
			}
			Button.VSplitLeft(2.0f, 0, &Button);
		}
	}

	UI()->ClipDisable();

	if(NeedScrollbar)
	{
		ServerFilter.HSplitTop(LineSize, &Button, &ServerFilter);
		s_ScrollValue = DoScrollbarH(&s_ScrollValue, &Button, s_ScrollValue);
	}
	else
		ServerFilter.HSplitTop(4.f, &Button, &ServerFilter); // Leave some space in between edit boxes
	ServerFilter.HSplitTop(LineSize, &Button, &ServerFilter);

	Button.VSplitLeft(60.0f, &Button, &Icon);
	ServerFilter.HSplitTop(3.0f, 0, &ServerFilter);
	static char s_aGametype[16] = { 0 };
	static float s_Offset = 0.0f;
	static int s_EditGametype = 0;
	Button.VSplitRight(Button.h, &Label, &Button);
	DoEditBox(&s_EditGametype, &Label, s_aGametype, sizeof(s_aGametype), FontSize, &s_Offset);
	RenderTools()->DrawUIRect(&Button, vec4(1.0f, 1.0f, 1.0f, 0.25f), CUI::CORNER_R, 5.0f);
	DoIcon(IMAGE_FRIENDICONS, UI()->MouseInside(&Button) ? SPRITE_FRIEND_PLUS_A : SPRITE_FRIEND_PLUS_B, &Button);
	static CButtonContainer s_AddGametype;
	if(s_aGametype[0] && UI()->DoButtonLogic(&s_AddGametype, "", 0, &Button))
	{
		for(int i = 0; i < CServerFilterInfo::MAX_GAMETYPES; ++i)
		{
			if(!FilterInfo.m_aGametype[i][0])
			{
				str_copy(FilterInfo.m_aGametype[i], s_aGametype, sizeof(FilterInfo.m_aGametype[i]));
				pFilter->SetFilter(&FilterInfo);
				s_aGametype[0] = 0;
				break;
			}
		}
	}
	Icon.VSplitLeft(10.0f, 0, &Icon);
	Icon.VSplitLeft(40.0f, &Button, 0);
	static CButtonContainer s_ClearGametypes;
	if(DoButton_MenuTabTop(&s_ClearGametypes, Localize("Clear"), false, &Button))
	{
		for(int i = 0; i < CServerFilterInfo::MAX_GAMETYPES; ++i)
		{
			FilterInfo.m_aGametype[i][0] = 0;
		}
		pFilter->SetFilter(&FilterInfo);
	}

	if(!NeedScrollbar)
		ServerFilter.HSplitTop(LineSize - 4.f, &Button, &ServerFilter);

	{
		ServerFilter.HSplitTop(LineSize, &Button, &ServerFilter);
		CUIRect EditBox;
		Button.VSplitRight(60.0f, &Button, &EditBox);

		UI()->DoLabelScaled(&Button, Localize("Maximum ping:"), FontSize, CUI::ALIGN_LEFT);

		char aBuf[5];
		str_format(aBuf, sizeof(aBuf), "%d", FilterInfo.m_Ping);
		static float Offset = 0.0f;
		static int s_BrFilterPing = 0;
		DoEditBox(&s_BrFilterPing, &EditBox, aBuf, sizeof(aBuf), FontSize, &Offset);
		int NewPing = clamp(str_toint(aBuf), 0, 999);
		if(NewPing != FilterInfo.m_Ping)
		{
			FilterInfo.m_Ping = NewPing;
			pFilter->SetFilter(&FilterInfo);
		}
	}

	// server address
	ServerFilter.HSplitTop(3.0f, 0, &ServerFilter);
	ServerFilter.HSplitTop(LineSize, &Button, &ServerFilter);
	UI()->DoLabelScaled(&Button, Localize("Server address:"), FontSize, CUI::ALIGN_LEFT);
	Button.VSplitRight(60.0f, 0, &Button);
	static float OffsetAddr = 0.0f;
	static int s_BrFilterServerAddress = 0;
	if(DoEditBox(&s_BrFilterServerAddress, &Button, FilterInfo.m_aAddress, sizeof(FilterInfo.m_aAddress), FontSize, &OffsetAddr))
		pFilter->SetFilter(&FilterInfo);

	// player country
	{
		CUIRect Rect;
		ServerFilter.HSplitTop(3.0f, 0, &ServerFilter);
		ServerFilter.HSplitTop(LineSize, &Button, &ServerFilter);
		UI()->DoLabelScaled(&Button, Localize("Country:"), FontSize, CUI::ALIGN_LEFT);
		Button.VSplitRight(60.0f, 0, &Rect);
		Rect.VSplitLeft(16.0f, &Button, &Rect);
		static int s_BrFilterCountry = 0;
		if(DoButton_CheckBox(&s_BrFilterCountry, "", FilterInfo.m_SortHash&IServerBrowser::FILTER_COUNTRY, &Button))
		{
			FilterInfo.m_SortHash ^= IServerBrowser::FILTER_COUNTRY;
			pFilter->SetFilter(&FilterInfo);
		}
		Rect.w = Rect.h * 2;
		vec4 Color(1.0f, 1.0f, 1.0f, FilterInfo.m_SortHash&IServerBrowser::FILTER_COUNTRY ? 1.0f : 0.5f);
		m_pClient->m_pCountryFlags->Render(FilterInfo.m_Country, &Color, Rect.x, Rect.y, Rect.w, Rect.h);

		static int s_BrFilterCountryIndex = 0;
		if((FilterInfo.m_SortHash&IServerBrowser::FILTER_COUNTRY) && UI()->DoButtonLogic(&s_BrFilterCountryIndex, "", 0, &Rect))
			m_Popup = POPUP_COUNTRY;
	}

	// level
	ServerFilter.HSplitTop(5.0f, 0, &ServerFilter);

	ServerFilter.HSplitTop(LineSize + 2, &Button, &ServerFilter);
	UI()->DoLabelScaled(&Button, Localize("Difficulty"), FontSize, CUI::ALIGN_LEFT);
	Button.VSplitRight(60.0f, 0, &Button);
	Button.y -= 2.0f;
	Button.VSplitLeft(Button.h, &Icon, &Button);
	static CButtonContainer s_LevelButton1;
	if(DoButton_SpriteID(&s_LevelButton1, IMAGE_LEVELICONS, (FilterInfo.m_ServerLevel & 1) ? SPRITE_LEVEL_A_B : SPRITE_LEVEL_A_ON, false, &Icon, CUI::CORNER_L, 5.0f, true))
	{
		FilterInfo.m_ServerLevel ^= 1;
		pFilter->SetFilter(&FilterInfo);
	}
	Button.VSplitLeft(Button.h, &Icon, &Button);
	static CButtonContainer s_LevelButton2;
	if(DoButton_SpriteID(&s_LevelButton2, IMAGE_LEVELICONS, (FilterInfo.m_ServerLevel & 2) ? SPRITE_LEVEL_B_B : SPRITE_LEVEL_B_ON, false, &Icon, 0, 5.0f, true))
	{
		FilterInfo.m_ServerLevel ^= 2;
		pFilter->SetFilter(&FilterInfo);
	}
	Button.VSplitLeft(Button.h, &Icon, &Button);
	static CButtonContainer s_LevelButton3;
	if(DoButton_SpriteID(&s_LevelButton3, IMAGE_LEVELICONS, (FilterInfo.m_ServerLevel & 4) ? SPRITE_LEVEL_C_B : SPRITE_LEVEL_C_ON, false, &Icon, CUI::CORNER_R, 5.0f, true))
	{
		FilterInfo.m_ServerLevel ^= 4;
		pFilter->SetFilter(&FilterInfo);
	}

	// reset filter
	ServerFilter.HSplitBottom(LineSize, &ServerFilter, 0);
	ServerFilter.HSplitBottom(LineSize, &ServerFilter, &Button);
	Button.VMargin((Button.w - 80.0f) / 2, &Button);
	static CButtonContainer s_ResetButton;
	if(DoButton_Menu(&s_ResetButton, Localize("Reset filter"), 0, &Button))
	{
		pFilter->Reset();
	}
=======
>>>>>>> Remove sound and graphics from components
}

void CMenus::RenderServerbrowserInfoTab(CUIRect View)
{

}

void CMenus::RenderDetailInfo(CUIRect View, const CServerInfo *pInfo)
{

<<<<<<< HEAD
			LeftColumn.HSplitTop(15.0f, &Row, &LeftColumn);
			UI()->DoLabelScaled(&Row, s_aLabels[i], FontSize, CUI::ALIGN_LEFT);
		}

		// map
		RightColumn.HSplitTop(15.0f, &Row, &RightColumn);
		UI()->DoLabelScaled(&Row, pInfo->m_aMap, FontSize, CUI::ALIGN_LEFT);

		// game type
		RightColumn.HSplitTop(15.0f, &Row, &RightColumn);
		CUIRect Icon;
		Row.VSplitLeft(Row.h, &Icon, &Row);
		Icon.y -= 2.0f;
		/*if(!(pInfo->m_Flags&IServerBrowser::FLAG_PURE))
		{
			DoIcon(IMAGE_BROWSEICONS, SPRITE_BROWSE_UNPURE_A, &Icon);
		}
		else*/
		{
			DoGameIcon(pInfo->m_aGameType, &Icon, CGameIcon::GAMEICON_FULL);
		}
		UI()->DoLabelScaled(&Row, pInfo->m_aGameType, FontSize, CUI::ALIGN_LEFT);

		// version
		RightColumn.HSplitTop(15.0f, &Row, &RightColumn);
		UI()->DoLabelScaled(&Row, pInfo->m_aVersion, FontSize, CUI::ALIGN_LEFT);

		// difficulty
		RightColumn.HSplitTop(15.0f, &Row, &RightColumn);
		Row.VSplitLeft(Row.h, &Icon, &Row);
		Icon.y -= 2.0f;
		switch(pInfo->m_ServerLevel)
		{
		case 0:
			DoIcon(IMAGE_LEVELICONS, SPRITE_LEVEL_A_ON, &Icon);
			break;
		case 1:
			DoIcon(IMAGE_LEVELICONS, SPRITE_LEVEL_B_ON, &Icon);
			break;
		case 2:
			DoIcon(IMAGE_LEVELICONS, SPRITE_LEVEL_C_ON, &Icon);
		}
		UI()->DoLabelScaled(&Row, s_aDifficulty[pInfo->m_ServerLevel], FontSize, CUI::ALIGN_LEFT);
	}
=======
>>>>>>> Remove sound and graphics from components
}

void CMenus::RenderDetailScoreboard(CUIRect View, const CServerInfo *pInfo, int RowCount)
{
<<<<<<< HEAD
	// slected filter
	CBrowserFilter *pFilter = 0;
	for(int i = 0; i < m_lFilters.size(); ++i)
	{
		if(m_lFilters[i].Extended())
		{
			pFilter = &m_lFilters[i];
			m_SelectedFilter = i;
			break;
		}
	}
	if(!pFilter)
		return;
	CServerFilterInfo FilterInfo;
	pFilter->GetFilter(&FilterInfo);

	// server scoreboard
	CTextCursor Cursor;
	const float FontSize = 10.0f;
	int ActColumn = 0;
	RenderTools()->DrawUIRect(&View, vec4(0, 0, 0, 0.15f), CUI::CORNER_B, 4.0f);
	View.Margin(2.0f, &View);
	
	if(pInfo)
	{
		int Count = 0;

		CUIRect Scroll;

		UI()->ClipEnable(&View);

		float RowWidth = (RowCount == 0) ? View.w : (View.w * 0.25f);
		float LineHeight = 20.0f;

		if(RowCount == 0)
		{
			float Length = 20.0f * pInfo->m_NumClients;
			static float s_ScrollValue = 0.0f;
			int ScrollNum = (int)((Length - View.h)/20.0f)+1;
			if(ScrollNum > 0)
			{
				if(Input()->KeyPress(KEY_MOUSE_WHEEL_UP) && UI()->MouseInside(&View))
					s_ScrollValue = clamp(s_ScrollValue - 3.0f/ScrollNum, 0.0f, 1.0f);
				if(Input()->KeyPress(KEY_MOUSE_WHEEL_DOWN) && UI()->MouseInside(&View))
					s_ScrollValue = clamp(s_ScrollValue + 3.0f / ScrollNum, 0.0f, 1.0f);
			}
			if(Length > View.h)
			{
				View.VSplitRight(8.0f, &View, &Scroll);
				Scroll.HMargin(5.0f, &Scroll);
				s_ScrollValue = DoScrollbarV(&s_ScrollValue, &Scroll, s_ScrollValue);
				View.y += (View.h - Length) * s_ScrollValue;
			}
		}
		else
		{
			float Width = RowWidth * ceil(pInfo->m_NumClients / RowCount);
			static float s_ScrollValue = 0.0f;
			if(Width > View.w)
			{
				View.HSplitBottom(8.0f, &View, &Scroll);
				Scroll.VMargin(5.0f, &Scroll);
				s_ScrollValue = DoScrollbarH(&s_ScrollValue, &Scroll, s_ScrollValue);
				View.x += (View.w - Width) * s_ScrollValue;
				LineHeight = 0.25f*View.h;
			}
		}
		
		CUIRect Row = View;

		for(int i = 0; i < pInfo->m_NumClients; i++)
		{
			if((FilterInfo.m_SortHash&IServerBrowser::FILTER_BOTS) && (pInfo->m_aClients[i].m_PlayerType&CServerInfo::CClient::PLAYERFLAG_BOT))
				continue;

			CUIRect Name, Clan, Score, Flag, Icon;

			if(RowCount > 0 && Count % RowCount == 0)
			{
				View.VSplitLeft(RowWidth, &Row, &View);
				ActColumn++;
			}
	
			Row.HSplitTop(LineHeight, &Name, &Row);
			RenderTools()->DrawUIRect(&Name, vec4(1.0f, 1.0f, 1.0f, (Count % 2 + 1)*0.05f), CUI::CORNER_ALL, 4.0f);

			// friend
			if(UI()->DoButtonLogic(&pInfo->m_aClients[i], "", 0, &Name))
			{
				if(pInfo->m_aClients[i].m_FriendState == IFriends::FRIEND_PLAYER)
					m_pClient->Friends()->RemoveFriend(pInfo->m_aClients[i].m_aName, pInfo->m_aClients[i].m_aClan);
				else
					m_pClient->Friends()->AddFriend(pInfo->m_aClients[i].m_aName, pInfo->m_aClients[i].m_aClan);
				FriendlistOnUpdate();
				Client()->ServerBrowserUpdate();
			}
			Name.VSplitLeft(Name.h-8.0f, &Icon, &Name);
			Icon.HMargin(4.0f, &Icon);
			if(pInfo->m_aClients[i].m_FriendState != IFriends::FRIEND_NO)
				DoIcon(IMAGE_BROWSEICONS, SPRITE_BROWSE_HEART_A, &Icon);

			Name.VSplitLeft(2.0f, 0, &Name);
			Name.VSplitLeft(20.0f, &Score, &Name);
			Name.VSplitRight(2*(Name.h-8.0f), &Name, &Flag);
			Flag.HMargin(4.0f, &Flag);
			Name.HSplitTop(LineHeight*0.5f, &Name, &Clan);

			// score
			if(!(pInfo->m_aClients[i].m_PlayerType&CServerInfo::CClient::PLAYERFLAG_SPEC))
			{
				char aTemp[16];
				str_format(aTemp, sizeof(aTemp), "%d", pInfo->m_aClients[i].m_Score);
				TextRender()->SetCursor(&Cursor, Score.x, Score.y + (Score.h - FontSize-2) / 4.0f, FontSize-2, TEXTFLAG_RENDER | TEXTFLAG_STOP_AT_END);
				Cursor.m_LineWidth = Score.w;
				TextRender()->TextEx(&Cursor, aTemp, -1);
			}

			// name
			TextRender()->SetCursor(&Cursor, Name.x, Name.y, FontSize - 2, TEXTFLAG_RENDER | TEXTFLAG_STOP_AT_END);
			Cursor.m_LineWidth = Name.w;
			const char *pName = pInfo->m_aClients[i].m_aName;
			if(g_Config.m_BrFilterString[0])
			{
				// highlight the parts that matches
				const char *s = str_find_nocase(pName, g_Config.m_BrFilterString);
				if(s)
				{
					TextRender()->TextEx(&Cursor, pName, (int)(s - pName));
					TextRender()->TextColor(0.4f, 0.4f, 1.0f, 1.0f);
					TextRender()->TextEx(&Cursor, s, str_length(g_Config.m_BrFilterString));
					TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
					TextRender()->TextEx(&Cursor, s + str_length(g_Config.m_BrFilterString), -1);
				}
				else
					TextRender()->TextEx(&Cursor, pName, -1);
			}
			else
				TextRender()->TextEx(&Cursor, pName, -1);

			// clan
			TextRender()->SetCursor(&Cursor, Clan.x, Clan.y, FontSize - 2, TEXTFLAG_RENDER | TEXTFLAG_STOP_AT_END);
			Cursor.m_LineWidth = Clan.w;
			const char *pClan = pInfo->m_aClients[i].m_aClan;
			if(g_Config.m_BrFilterString[0])
			{
				// highlight the parts that matches
				const char *s = str_find_nocase(pClan, g_Config.m_BrFilterString);
				if(s)
				{
					TextRender()->TextEx(&Cursor, pClan, (int)(s - pClan));
					TextRender()->TextColor(0.4f, 0.4f, 1.0f, 1.0f);
					TextRender()->TextEx(&Cursor, s, str_length(g_Config.m_BrFilterString));
					TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
					TextRender()->TextEx(&Cursor, s + str_length(g_Config.m_BrFilterString), -1);
				}
				else
					TextRender()->TextEx(&Cursor, pClan, -1);
			}
			else
				TextRender()->TextEx(&Cursor, pClan, -1);

			// flag
			Flag.w = Flag.h*2;
			vec4 Color(1.0f, 1.0f, 1.0f, 0.5f);
			m_pClient->m_pCountryFlags->Render(pInfo->m_aClients[i].m_Country, &Color, Flag.x, Flag.y, Flag.w, Flag.h);

			++Count;
		}

		UI()->ClipDisable();
	}
=======

>>>>>>> Remove sound and graphics from components
}

void CMenus::RenderServerbrowserServerDetail(CUIRect View, const CServerInfo *pInfo)
{
<<<<<<< HEAD
	CUIRect ServerHeader, ServerDetails, ServerScoreboard;
	const float FontSize = 10.0f;

	// split off a piece to use for scoreboard
	//View.HMargin(2.0f, &View);
	View.HSplitTop(80.0f, &ServerDetails, &ServerScoreboard);

	// server details
	RenderDetailInfo(ServerDetails, pInfo);

	// server scoreboard
	ServerScoreboard.HSplitTop(ms_ListheaderHeight, &ServerHeader, &ServerScoreboard);
	RenderTools()->DrawUIRect(&ServerHeader, vec4(1, 1, 1, 0.25f), CUI::CORNER_T, 4.0f);
	//RenderTools()->DrawUIRect(&View, vec4(0, 0, 0, 0.15f), CUI::CORNER_B, 4.0f);
	ServerHeader.HMargin(2.0f, &ServerHeader);
	UI()->DoLabelScaled(&ServerHeader, Localize("Scoreboard"), FontSize + 2.0f, CUI::ALIGN_CENTER);
	RenderDetailScoreboard(ServerScoreboard, pInfo, 0);
=======

>>>>>>> Remove sound and graphics from components
}

void CMenus::FriendlistOnUpdate()
{
	// fill me
}

void CMenus::RenderServerbrowserBottomBox(CUIRect MainView)
{

}
void CMenus::DoGameIcon(const char *pName, const CUIRect *pRect, int Type)
{
	// get texture
	IGraphics::CTextureHandle Tex = m_GameIconDefault;
	for(int i = 0; i < m_lGameIcons.size(); ++i)
	{
		if(!str_comp_nocase(pName, m_lGameIcons[i].m_Name))
		{
			Tex = m_lGameIcons[i].m_IconTexture;
			break;
		}
	}
	Graphics()->TextureSet(Tex);
	Graphics()->QuadsBegin();

	// select sprite
	switch(Type)
	{
	case CGameIcon::GAMEICON_FULL:
		Graphics()->QuadsSetSubset(0.0f, 0.0f, 1.0f, 1.0f/3.0f);
		break;
	case CGameIcon::GAMEICON_ON:
		Graphics()->QuadsSetSubset(0.0f, 1.0f/3.0f, 1.0f, 2.0f/3.0f);
		break;
	default:	// GAMEICON_OFF
		Graphics()->QuadsSetSubset(0.0f, 2.0f/3.0f, 1.0f, 1.0f);
	}

	// draw icon
	IGraphics::CQuadItem QuadItem(pRect->x, pRect->y, pRect->w, pRect->h);
	Graphics()->QuadsDrawTL(&QuadItem, 1);
	Graphics()->QuadsEnd();
}

int CMenus::GameIconScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	CMenus *pSelf = (CMenus *)pUser;
	int l = str_length(pName);
	if(l < 5 || IsDir || str_comp(pName + l - 4, ".png") != 0)
		return 0;

	char aGameIconName[128] = { 0 };
	str_copy(aGameIconName, pName, min((int)sizeof(aGameIconName), l - 3));

	// add new game icon
	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "ui/gametypes/%s", pName);
	CImageInfo Info;
	if(!pSelf->Graphics()->LoadPNG(&Info, aBuf, DirType))
	{
		str_format(aBuf, sizeof(aBuf), "failed to load gametype icon '%s'", aGameIconName);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "game", aBuf);
		return 0;
	}
	CGameIcon GameIcon(aGameIconName);
	str_format(aBuf, sizeof(aBuf), "loaded gametype icon '%s'", aGameIconName);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "game", aBuf);

	GameIcon.m_IconTexture = pSelf->Graphics()->LoadTextureRaw(Info.m_Width, Info.m_Height, Info.m_Format, Info.m_pData, Info.m_Format, IGraphics::TEXLOAD_LINEARMIPMAPS);
	pSelf->m_lGameIcons.add(GameIcon);
	if(!str_comp_nocase(aGameIconName, "mod"))
		pSelf->m_GameIconDefault = GameIcon.m_IconTexture;
	return 0;
}

void CMenus::RenderServerbrowser(CUIRect MainView)
{
<<<<<<< HEAD
	/*
		+---------------------------+-------+
		|							|		|
		|							| side-	|
		|		server list			| bar	|
		|							| <->	|
		|---------------------------+-------+
		| back |	   | bottom box |
		+------+       +------------+
	*/
	static bool s_Init = true;
	if(s_Init)
	{
		Storage()->ListDirectory(IStorage::TYPE_ALL, "ui/gametypes", GameIconScan, this);
		s_Init = false;
	}

	CUIRect ServerList, Sidebar, BottomBox, SidebarButton;

	MainView.HSplitTop(20.0f, 0, &MainView);
	MainView.VSplitRight(20.0f, &MainView, &SidebarButton);
	MainView.HSplitBottom(80.0f, &ServerList, &MainView);
	if(m_SidebarActive)
		ServerList.VSplitRight(150.0f, &ServerList, &Sidebar);

	// server list
	RenderServerbrowserServerList(ServerList);

	// sidebar
	if(m_SidebarActive)
		RenderServerbrowserSidebar(Sidebar);

	// sidebar button
	SidebarButton.HMargin(150.0f, &SidebarButton);
	static CButtonContainer s_SidebarButton;
	if(DoButton_SpriteID(&s_SidebarButton, IMAGE_ARROWICONS, m_SidebarActive?SPRITE_ARROW_RIGHT_A:SPRITE_ARROW_LEFT_A, false, &SidebarButton, CUI::CORNER_R, 5.0f, true))
	{
		m_SidebarActive ^= 1;
	}

	float Spacing = 3.0f;
	float ButtonWidth = (MainView.w/6.0f)-(Spacing*5.0)/6.0f;

	MainView.HSplitBottom(60.0f, 0, &BottomBox);
	BottomBox.VSplitRight(ButtonWidth*2.0f+Spacing, 0, &BottomBox);

	// connect box
	RenderServerbrowserBottomBox(BottomBox);

	// back button
	RenderBackButton(MainView);
=======
>>>>>>> Remove sound and graphics from components

}

void CMenus::ConchainFriendlistUpdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	pfnCallback(pResult, pCallbackUserData);
	if(pResult->NumArguments() == 2 && ((CMenus *)pUserData)->Client()->State() == IClient::STATE_OFFLINE)
	{
		((CMenus *)pUserData)->FriendlistOnUpdate();
		((CMenus *)pUserData)->Client()->ServerBrowserUpdate();
	}
}

void CMenus::ConchainServerbrowserUpdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	pfnCallback(pResult, pCallbackUserData);
	/*if(pResult->NumArguments() && ((CMenus *)pUserData)->m_MenuPage == PAGE_FAVORITES && ((CMenus *)pUserData)->Client()->State() == IClient::STATE_OFFLINE)
		((CMenus *)pUserData)->ServerBrowser()->Refresh(IServerBrowser::TYPE_FAVORITES);*/
}
