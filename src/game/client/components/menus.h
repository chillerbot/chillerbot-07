/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_MENUS_H
#define GAME_CLIENT_COMPONENTS_MENUS_H

#include <base/vmath.h>
#include <base/tl/sorted_array.h>
#include <engine/demo.h>
#include <engine/friends.h>
#include <game/voting.h>
#include <game/client/component.h>
#include <game/client/localization.h>

#include "skins.h"


// component to fetch keypresses, override all other input
class CMenusKeyBinder : public CComponent
{
public:
	bool m_TakeKey;
	bool m_GotKey;
	IInput::CEvent m_Key;
	CMenusKeyBinder();
	virtual bool OnInput(IInput::CEvent Event);
};

class CMenus : public CComponent
{
public:
	class CButtonContainer
	{
	public:
		float m_FadeStartTime;

		const void *GetID() const { return &m_FadeStartTime; }
	};

private:
	float ButtonFade(CButtonContainer *pBC, float Seconds, int Checked=0);


	struct CListboxItem
	{
		int m_Visible;
		int m_Selected;
	};

	struct CListBoxState
	{
		float m_ListBoxRowHeight;
		int m_ListBoxItemIndex;
		int m_ListBoxSelectedIndex;
		int m_ListBoxNewSelected;
		int m_ListBoxDoneEvents;
		int m_ListBoxNumItems;
		int m_ListBoxItemsPerRow;
		float m_ListBoxScrollValue;
		bool m_ListBoxItemActivated;

		CListBoxState()
		{
			m_ListBoxScrollValue = 0;
		}
	};

	int UiDoListboxEnd(CListBoxState* pState, bool *pItemActivated);

	enum
	{
		POPUP_NONE=0,
		POPUP_FIRST_LAUNCH,
		POPUP_CONNECTING,
		POPUP_MESSAGE,
		POPUP_DISCONNECTED,
		POPUP_PURE,
		POPUP_LANGUAGE,
		POPUP_COUNTRY,
		POPUP_DELETE_DEMO,
		POPUP_RENAME_DEMO,
		POPUP_REMOVE_FRIEND,
		POPUP_SAVE_SKIN,
		POPUP_DELETE_SKIN,
		POPUP_SOUNDERROR,
		POPUP_PASSWORD,
		POPUP_QUIT,
	};

	enum
	{
		PAGE_NEWS=0,
		PAGE_GAME,
		PAGE_PLAYERS,
		PAGE_SERVER_INFO,
		PAGE_CALLVOTE,
		PAGE_INTERNET,
		PAGE_LAN,
		PAGE_DEMOS,
		PAGE_SETTINGS,
		PAGE_SYSTEM,
		PAGE_START,

		SETTINGS_GENERAL=0,
		SETTINGS_PLAYER,
		SETTINGS_TEE,
		SETTINGS_CONTROLS,
		SETTINGS_GRAPHICS,
		SETTINGS_SOUND,

		ACTLB_LANG=0,
		ACTLB_THEME,
	};

	int m_GamePage;
	int m_Popup;
	int m_ActivePage;
	int m_MenuPage;
	bool m_MenuActive;
	int m_SidebarTab;
	bool m_SidebarActive;
	bool m_ShowServerDetails;
	bool m_UseMouseButtons;
	vec2 m_MousePos;
	vec2 m_PrevMousePos;
	bool m_PopupActive;
	int m_ActiveListBox;

	// images
	struct CMenuImage
	{
		char m_aName[64];
		IGraphics::CTextureHandle m_OrgTexture;
		IGraphics::CTextureHandle m_GreyTexture;
	};
	array<CMenuImage> m_lMenuImages;

	static int MenuImageScan(const char *pName, int IsDir, int DirType, void *pUser);

	const CMenuImage *FindMenuImage(const char* pName);

	// themes
	class CTheme
	{
	public:
		CTheme() {}
		CTheme(const char *n, bool HasDay, bool HasNight) : m_Name(n), m_HasDay(HasDay), m_HasNight(HasNight) {}

		string m_Name;
		bool m_HasDay;
		bool m_HasNight;
		IGraphics::CTextureHandle m_IconTexture;
		bool operator<(const CTheme &Other) { return m_Name < Other.m_Name; }
	};
	sorted_array<CTheme> m_lThemes;

	static int ThemeScan(const char *pName, int IsDir, int DirType, void *pUser);
	static int ThemeIconScan(const char *pName, int IsDir, int DirType, void *pUser);

	int64 m_LastInput;

	// loading
	int m_LoadCurrent;
	int m_LoadTotal;

	//
	char m_aMessageTopic[512];
	char m_aMessageBody[512];
	char m_aMessageButton[512];
	int m_NextPopup;

	void PopupMessage(const char *pTopic, const char *pBody, const char *pButton, int Next=POPUP_NONE);

	// some settings
	static float ms_ButtonHeight;
	static float ms_ListheaderHeight;
	static float ms_FontmodHeight;
	static float ms_BackgroundAlpha;

	// for settings
	bool m_NeedRestartGraphics;
	bool m_NeedRestartSound;
	int m_TeePartSelected;
	char m_aSaveSkinName[24];

	void SaveSkinfile();
	bool m_RefreshSkinSelector;
	const CSkins::CSkin *m_pSelectedSkin;

	//
	bool m_EscapePressed;
	bool m_EnterPressed;
	bool m_TabPressed;
	bool m_DeletePressed;
	bool m_UpArrowPressed;
	bool m_DownArrowPressed;

	// for map download popup
	int64 m_DownloadLastCheckTime;
	int m_DownloadLastCheckSize;
	float m_DownloadSpeed;

	// for call vote
	int m_CallvoteSelectedOption;
	int m_CallvoteSelectedPlayer;
	char m_aCallvoteReason[VOTE_REASON_LENGTH];

	// for callbacks
	int *m_pActiveDropdown;

	// demo
	struct CDemoItem
	{
		char m_aFilename[128];
		char m_aName[128];
		bool m_IsDir;
		int m_StorageType;

		bool m_InfosLoaded;
		bool m_Valid;
		CDemoHeader m_Info;

		bool operator<(const CDemoItem &Other) { return !str_comp(m_aFilename, "..") ? true : !str_comp(Other.m_aFilename, "..") ? false :
														m_IsDir && !Other.m_IsDir ? true : !m_IsDir && Other.m_IsDir ? false :
														str_comp_filenames(m_aFilename, Other.m_aFilename) < 0; }
	};

	sorted_array<CDemoItem> m_lDemos;
	char m_aCurrentDemoFolder[256];
	char m_aCurrentDemoFile[64];
	int m_DemolistSelectedIndex;
	bool m_DemolistSelectedIsDir;
	int m_DemolistStorageType;
	int64 m_SeekBarActivatedTime;
	bool m_SeekBarActive;

	void DemolistOnUpdate(bool Reset);
	void DemolistPopulate();
	static int DemolistFetchCallback(const char *pName, int IsDir, int StorageType, void *pUser);

	// friends
	class CFriendItem
	{
	public:
		const CFriendInfo *m_pFriendInfo;
		const CServerInfo *m_pServerInfo;
		char m_aName[MAX_NAME_LENGTH];
		char m_aClan[MAX_CLAN_LENGTH];
		bool m_IsPlayer;

		CFriendItem()
		{
			m_pFriendInfo = 0;
			m_pServerInfo = 0;
		}

		bool operator<(const CFriendItem &Other)
		{
			if(m_aName[0] && !Other.m_aName[0])
				return true;
			if(!m_aName[0] && Other.m_aName[0])
				return false;
			int Result = str_comp_nocase(m_aName, Other.m_aName);
			if(Result < 0 || (Result == 0 && str_comp_nocase(m_aClan, Other.m_aClan) < 0))
				return true;
			
			return false;
		}
	};

	enum
	{
		FRIEND_PLAYER_ON = 0,
		FRIEND_CLAN_ON,
		FRIEND_OFF,
		NUM_FRIEND_TYPES
	};
	sorted_array<CFriendItem> m_lFriendList[NUM_FRIEND_TYPES];
	const CFriendItem *m_pDeleteFriend;

	void FriendlistOnUpdate();

	class CBrowserFilter
	{
		bool m_Extended;
		int m_Custom;
		char m_aName[64];
		int m_Filter;
		class IServerBrowser *m_pServerBrowser;

		static class CServerFilterInfo ms_FilterStandard;
		static class CServerFilterInfo ms_FilterFavorites;
		static class CServerFilterInfo ms_FilterAll;

	public:

		enum
		{
			FILTER_CUSTOM=0,
			FILTER_ALL,
			FILTER_STANDARD,
			FILTER_FAVORITES,
		};
		// buttons var
		int m_SwitchButton;
		int m_aButtonID[3];

		CBrowserFilter() {}
		CBrowserFilter(int Custom, const char* pName, IServerBrowser *pServerBrowser);
		void Switch();
		bool Extended() const;
		int Custom() const;
		int Filter() const;
		const char* Name() const;

		void SetFilterNum(int Num);

		int NumSortedServers() const;
		int NumPlayers() const;
		const CServerInfo *SortedGet(int Index) const;
		const void *ID(int Index) const;

		void Reset();
		void GetFilter(class CServerFilterInfo *pFilterInfo) const;
		void SetFilter(const class CServerFilterInfo *pFilterInfo);
	};

	array<CBrowserFilter> m_lFilters;

	int m_SelectedFilter;

	void LoadFilters();
	void SaveFilters();
	void RemoveFilter(int FilterIndex);
	void Move(bool Up, int Filter);

	class CInfoOverlay
	{
	public:
		enum
		{
			OVERLAY_SERVERINFO=0,
			OVERLAY_HEADERINFO,
			OVERLAY_PLAYERSINFO,
		};

		int m_Type;
		const void *m_pData;
		float m_X;
		float m_Y;
		bool m_Reset;
	};

	CInfoOverlay m_InfoOverlay;
	bool m_InfoOverlayActive;

	class CServerEntry
	{
	public:
		int m_Filter;
		int m_Index;
	};

	CServerEntry m_SelectedServer;

	enum
	{
		FIXED=1,
		SPACER=2,

		COL_BROWSER_FLAG=0,
		COL_BROWSER_NAME,
		COL_BROWSER_GAMETYPE,
		COL_BROWSER_MAP,
		COL_BROWSER_PLAYERS,
		COL_BROWSER_PING,
		NUM_BROWSER_COLS,
	};

	struct CColumn
	{
		int m_ID;
		int m_Sort;
		CLocConstString m_Caption;
		int m_Direction;
		float m_Width;
		int m_Flags;
	};

	static CColumn ms_aBrowserCols[NUM_BROWSER_COLS];

	enum
	{
		MAX_RESOLUTIONS=256,
	};

	CVideoMode m_aModes[MAX_RESOLUTIONS];
	int m_NumModes;

	struct CVideoFormat
	{
		int m_WidthValue;
		int m_HeightValue;
	};

	CVideoFormat m_aVideoFormats[MAX_RESOLUTIONS];
	sorted_array<CVideoMode> m_lRecommendedVideoModes;
	sorted_array<CVideoMode> m_lOtherVideoModes;
	int m_NumVideoFormats;
	int m_CurrentVideoFormat;
	void UpdateVideoFormats();
	void UpdatedFilteredVideoModes();
	void UpdateVideoModeSettings();

	// found in menus.cpp
	int Render();
	//void render_background();
	//void render_loading(float percent);
	void RenderServerbrowserOverlay();
	static void ConchainFriendlistUpdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainServerbrowserUpdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainToggleMusic(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	void SetOverlay(int Type, float x, float y, const void *pData);
	void UpdateFriendCounter(const CServerInfo *pEntry);
	void UpdateFriends();

	// found in menus_settings.cpp

	void SetActive(bool Active);

	void DoPopupMenu();

	IGraphics::CTextureHandle m_TextureBlob;

	void ToggleMusic();

	void SetMenuPage(int NewPage);
public:
	struct CSwitchTeamInfo
	{
		char m_aNotification[128];
		bool m_AllowSpec;
		int m_TimeLeft;
	};
	void GetSwitchTeamInfo(CSwitchTeamInfo *pInfo);
	void RenderBackground();

	void UseMouseButtons(bool Use) { m_UseMouseButtons = Use; }

	static CMenusKeyBinder m_Binder;

	CMenus();

	void RenderLoading();

	bool IsActive() const { return m_MenuActive; }

	virtual void OnInit();

	virtual void OnConsoleInit();
	virtual void OnStateChange(int NewState, int OldState);
	virtual void OnReset();
	virtual void OnRender();
	virtual bool OnInput(IInput::CEvent Event);
	virtual bool OnMouseMove(float x, float y);
};
#endif
