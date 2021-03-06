/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <base/color.h>
#include <base/system.h>

#include <engine/shared/datafile.h>
#include <engine/shared/config.h>
#include <engine/client.h>
#include <engine/console.h>
#include <engine/input.h>
#include <engine/keys.h>
#include <engine/storage.h>

#include <game/gamecore.h>
#include <game/client/lineinput.h>
#include <game/client/localization.h>
#include <generated/client_data.h>

#include "auto_map.h"
#include "editor.h"

const void* CEditor::ms_pUiGotContext;

enum
{
	BUTTON_CONTEXT=1,
};

CEditorImage::~CEditorImage()
{

}

CLayerGroup::CLayerGroup()
{
	m_aName[0] = 0;
	m_Visible = true;
	m_SaveToMap = true;
	m_Collapse = false;
	m_GameGroup = false;
	m_OffsetX = 0;
	m_OffsetY = 0;
	m_ParallaxX = 100;
	m_ParallaxY = 100;

	m_UseClipping = 0;
	m_ClipX = 0;
	m_ClipY = 0;
	m_ClipW = 0;
	m_ClipH = 0;
}

CLayerGroup::~CLayerGroup()
{
	Clear();
}

void CLayerGroup::Mapping(float *pPoints)
{

}

void CLayerGroup::MapScreen()
{

}

void CLayerGroup::Render()
{

}

void CLayerGroup::AddLayer(CLayer *l)
{
	m_pMap->m_Modified = true;
	m_lLayers.add(l);
}

void CLayerGroup::DeleteLayer(int Index)
{
	if(Index < 0 || Index >= m_lLayers.size()) return;
	delete m_lLayers[Index];
	m_lLayers.remove_index(Index);
	m_pMap->m_Modified = true;
}

void CLayerGroup::GetSize(float *w, float *h) const
{
	*w = 0; *h = 0;
	for(int i = 0; i < m_lLayers.size(); i++)
	{
		float lw, lh;
		m_lLayers[i]->GetSize(&lw, &lh);
		*w = max(*w, lw);
		*h = max(*h, lh);
	}
}


int CLayerGroup::SwapLayers(int Index0, int Index1)
{
	if(Index0 < 0 || Index0 >= m_lLayers.size()) return Index0;
	if(Index1 < 0 || Index1 >= m_lLayers.size()) return Index0;
	if(Index0 == Index1) return Index0;
	m_pMap->m_Modified = true;
	swap(m_lLayers[Index0], m_lLayers[Index1]);
	return Index1;
}

void CEditorImage::AnalyseTileFlags()
{

}

void CEditorImage::LoadAutoMapper()
{
	if(m_pAutoMapper)
		return;

	// read file data into buffer
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "editor/automap/%s.json", m_aName);
	IOHANDLE File = m_pEditor->Storage()->OpenFile(aBuf, IOFLAG_READ, IStorage::TYPE_ALL);
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
		m_pEditor->Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, aBuf, aError);
		return;
	}

	// generate configurations
	const json_value &rTileset = (*pJsonData)[(const char *)IAutoMapper::GetTypeName(IAutoMapper::TYPE_TILESET)];
	if(rTileset.type == json_array)
	{
		m_pAutoMapper = new CTilesetMapper(m_pEditor);
		m_pAutoMapper->Load(rTileset);
	}
	else
	{
		const json_value &rDoodads = (*pJsonData)[(const char *)IAutoMapper::GetTypeName(IAutoMapper::TYPE_DOODADS)];
		if(rDoodads.type == json_array)
		{
			m_pAutoMapper = new CDoodadsMapper(m_pEditor);
			m_pAutoMapper->Load(rDoodads);
		}
	}

	// clean up
	json_value_free(pJsonData);
	if(m_pAutoMapper && g_Config.m_Debug)
	{
		str_format(aBuf, sizeof(aBuf),"loaded %s.json (%s)", m_aName, IAutoMapper::GetTypeName(m_pAutoMapper->GetType()));
		m_pEditor->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "editor", aBuf);
	}
}

void CEditor::EnvelopeEval(float TimeOffset, int Env, float *pChannels, void *pUser)
{
	CEditor *pThis = (CEditor *)pUser;
	if(Env < 0 || Env >= pThis->m_Map.m_lEnvelopes.size())
	{
		pChannels[0] = 0;
		pChannels[1] = 0;
		pChannels[2] = 0;
		pChannels[3] = 0;
		return;
	}

	CEnvelope *e = pThis->m_Map.m_lEnvelopes[Env];
	float t = pThis->m_AnimateTime+TimeOffset;
	t *= pThis->m_AnimateSpeed;
	e->Eval(t, pChannels);
}

/********************************************************
 OTHER
*********************************************************/

vec4 CEditor::ButtonColorMul(const void *pID)
{
	return vec4(1,1,1,1);
}

vec4 CEditor::GetButtonColor(const void *pID, int Checked)
{
	return vec4(1,1,1,0.5f);
}

void CEditor::RenderGrid(CLayerGroup *pGroup)
{

}

CLayerGroup *CEditor::GetSelectedGroup()
{
	return 0x0;
}

CLayer *CEditor::GetSelectedLayer(int Index)
{
	return 0x0;
}

CLayer *CEditor::GetSelectedLayerType(int Index, int Type)
{
	return 0x0;
}

CQuad *CEditor::GetSelectedQuad()
{
	return 0;
}

void CEditor::CallbackOpenMap(const char *pFileName, int StorageType, void *pUser)
{

}

void CEditor::CallbackAppendMap(const char *pFileName, int StorageType, void *pUser)
{

}

void CEditor::CallbackSaveMap(const char *pFileName, int StorageType, void *pUser)
{

}

void CEditor::DoQuad(CQuad *q, int Index)
{

}

void CEditor::DoQuadPoint(CQuad *pQuad, int QuadIndex, int V)
{

}

void CEditor::ReplaceImage(const char *pFileName, int StorageType, void *pUser)
{

}

void CEditor::AddImage(const char *pFileName, int StorageType, void *pUser)
{

}

static int CompareImageName(const void *pObject1, const void *pObject2)
{
	CEditorImage *pImage1 = *(CEditorImage**)pObject1;
	CEditorImage *pImage2 = *(CEditorImage**)pObject2;

	return str_comp(pImage1->m_aName, pImage2->m_aName);
}

static int *gs_pSortedIndex = 0;
static void ModifySortedIndex(int *pIndex)
{
	if(*pIndex > -1)
		*pIndex = gs_pSortedIndex[*pIndex];
}

void CEditor::SortImages()
{
	bool Sorted = true;
	for(int i = 1; i < m_Map.m_lImages.size(); i++)
		if( str_comp(m_Map.m_lImages[i]->m_aName, m_Map.m_lImages[i-1]->m_aName) < 0 )
		{
			Sorted = false;
			break;
		}

	if(!Sorted)
	{
		array<CEditorImage*> lTemp = array<CEditorImage*>(m_Map.m_lImages);
		gs_pSortedIndex = new int[lTemp.size()];

		qsort(m_Map.m_lImages.base_ptr(), m_Map.m_lImages.size(), sizeof(CEditorImage*), CompareImageName);

		for(int OldIndex = 0; OldIndex < lTemp.size(); OldIndex++)
			for(int NewIndex = 0; NewIndex < m_Map.m_lImages.size(); NewIndex++)
				if(lTemp[OldIndex] == m_Map.m_lImages[NewIndex])
					gs_pSortedIndex[OldIndex] = NewIndex;

		m_Map.ModifyImageIndex(ModifySortedIndex);

		delete [] gs_pSortedIndex;
		gs_pSortedIndex = 0;
	}
}


static int EditorListdirCallback(const char *pName, int IsDir, int StorageType, void *pUser)
{
	CEditor *pEditor = (CEditor*)pUser;
	int Length = str_length(pName);
	if((pName[0] == '.' && (pName[1] == 0 ||
		(pName[1] == '.' && pName[2] == 0 && (!str_comp(pEditor->m_pFileDialogPath, "maps") || !str_comp(pEditor->m_pFileDialogPath, "mapres"))))) ||
		(!IsDir && ((pEditor->m_FileDialogFileType == CEditor::FILETYPE_MAP && (Length < 4 || str_comp(pName+Length-4, ".map"))) ||
		(pEditor->m_FileDialogFileType == CEditor::FILETYPE_IMG && (Length < 4 || str_comp(pName+Length-4, ".png"))))))
		return 0;

	CEditor::CFilelistItem Item;
	str_copy(Item.m_aFilename, pName, sizeof(Item.m_aFilename));
	if(IsDir)
		str_format(Item.m_aName, sizeof(Item.m_aName), "%s/", pName);
	else
		str_copy(Item.m_aName, pName, min(static_cast<int>(sizeof(Item.m_aName)), Length-3));
	Item.m_IsDir = IsDir != 0;
	Item.m_IsLink = false;
	Item.m_StorageType = StorageType;
	pEditor->m_FileList.add(Item);

	return 0;
}

void CEditor::RenderFileDialog()
{

}

void CEditor::FilelistPopulate(int StorageType)
{
	m_FileList.clear();
	if(m_FileDialogStorageType != IStorage::TYPE_SAVE && !str_comp(m_pFileDialogPath, "maps"))
	{
		CFilelistItem Item;
		str_copy(Item.m_aFilename, "downloadedmaps", sizeof(Item.m_aFilename));
		str_copy(Item.m_aName, "downloadedmaps/", sizeof(Item.m_aName));
		Item.m_IsDir = true;
		Item.m_IsLink = true;
		Item.m_StorageType = IStorage::TYPE_SAVE;
		m_FileList.add(Item);
	}
	Storage()->ListDirectory(StorageType, m_pFileDialogPath, EditorListdirCallback, this);
	m_FilesSelectedIndex = m_FileList.size() ? 0 : -1;
	m_aFileDialogActivate = false;
}

void CEditor::InvokeFileDialog(int StorageType, int FileType, const char *pTitle, const char *pButtonText,
	const char *pBasePath, const char *pDefaultName,
	void (*pfnFunc)(const char *pFileName, int StorageType, void *pUser), void *pUser)
{
	m_FileDialogStorageType = StorageType;
	m_pFileDialogTitle = pTitle;
	m_pFileDialogButtonText = pButtonText;
	m_pfnFileDialogFunc = pfnFunc;
	m_pFileDialogUser = pUser;
	m_aFileDialogFileName[0] = 0;
	m_aFileDialogCurrentFolder[0] = 0;
	m_aFileDialogCurrentLink[0] = 0;
	m_pFileDialogPath = m_aFileDialogCurrentFolder;
	m_FileDialogFileType = FileType;
	m_FileDialogScrollValue = 0.0f;

	if(pDefaultName)
		str_copy(m_aFileDialogFileName, pDefaultName, sizeof(m_aFileDialogFileName));
	if(pBasePath)
		str_copy(m_aFileDialogCurrentFolder, pBasePath, sizeof(m_aFileDialogCurrentFolder));

	FilelistPopulate(m_FileDialogStorageType);

	m_Dialog = DIALOG_FILE;
}


void CEditor::Render()
{

}

void CEditor::Reset(bool CreateDefault)
{
	m_Map.Clean();

	// create default layers
	if(CreateDefault)
		m_Map.CreateDefault();

	m_SelectedLayer = 0;
	m_SelectedGroup = 0;
	m_SelectedQuad = -1;
	m_SelectedPoints = 0;
	m_SelectedEnvelope = 0;
	m_SelectedImage = 0;

	m_WorldOffsetX = 0;
	m_WorldOffsetY = 0;
	m_EditorOffsetX = 0.0f;
	m_EditorOffsetY = 0.0f;

	m_WorldZoom = 1.0f;
	m_ZoomLevel = 200;

	m_MouseDeltaX = 0;
	m_MouseDeltaY = 0;
	m_MouseDeltaWx = 0;
	m_MouseDeltaWy = 0;

	m_Map.m_Modified = false;

	m_ShowEnvelopePreview = SHOWENV_NONE;
}

int CEditor::GetLineDistance() const
{
	int LineDistance = 512;

	if(m_ZoomLevel <= 100)
		LineDistance = 16;
	else if(m_ZoomLevel <= 250)
		LineDistance = 32;
	else if(m_ZoomLevel <= 450)
		LineDistance = 64;
	else if(m_ZoomLevel <= 850)
		LineDistance = 128;
	else if(m_ZoomLevel <= 1550)
		LineDistance = 256;

	return LineDistance;
}

void CEditor::ZoomMouseTarget(float ZoomFactor)
{
	// zoom to the current mouse position
	// get absolute mouse position
	float aPoints[4];

	float WorldWidth = aPoints[2]-aPoints[0];
	float WorldHeight = aPoints[3]-aPoints[1];

	float Mwx = 0.0f;
	float Mwy = 0.0f;

	// adjust camera
	m_WorldOffsetX += (Mwx-m_WorldOffsetX) * (1-ZoomFactor);
	m_WorldOffsetY += (Mwy-m_WorldOffsetY) * (1-ZoomFactor);
}

void CEditorMap::DeleteEnvelope(int Index)
{
	if(Index < 0 || Index >= m_lEnvelopes.size())
		return;

	m_Modified = true;

	// fix links between envelopes and quads
	for(int i = 0; i < m_lGroups.size(); ++i)
		for(int j = 0; j < m_lGroups[i]->m_lLayers.size(); ++j)
			if(m_lGroups[i]->m_lLayers[j]->m_Type == LAYERTYPE_QUADS)
			{
				CLayerQuads *pLayer = static_cast<CLayerQuads *>(m_lGroups[i]->m_lLayers[j]);
				for(int k = 0; k < pLayer->m_lQuads.size(); ++k)
				{
					if(pLayer->m_lQuads[k].m_PosEnv == Index)
						pLayer->m_lQuads[k].m_PosEnv = -1;
					else if(pLayer->m_lQuads[k].m_PosEnv > Index)
						pLayer->m_lQuads[k].m_PosEnv--;
					if(pLayer->m_lQuads[k].m_ColorEnv == Index)
						pLayer->m_lQuads[k].m_ColorEnv = -1;
					else if(pLayer->m_lQuads[k].m_ColorEnv > Index)
						pLayer->m_lQuads[k].m_ColorEnv--;
				}
			}
			else if(m_lGroups[i]->m_lLayers[j]->m_Type == LAYERTYPE_TILES)
			{
				CLayerTiles *pLayer = static_cast<CLayerTiles *>(m_lGroups[i]->m_lLayers[j]);
				if(pLayer->m_ColorEnv == Index)
					pLayer->m_ColorEnv = -1;
				if(pLayer->m_ColorEnv > Index)
					pLayer->m_ColorEnv--;
			}

	m_lEnvelopes.remove_index(Index);
}

void CEditorMap::MakeGameLayer(CLayer *pLayer)
{
	m_pGameLayer = (CLayerGame *)pLayer;
	m_pGameLayer->m_pEditor = m_pEditor;
}

void CEditorMap::MakeGameGroup(CLayerGroup *pGroup)
{
	m_pGameGroup = pGroup;
	m_pGameGroup->m_GameGroup = true;
	str_copy(m_pGameGroup->m_aName, "Game", sizeof(m_pGameGroup->m_aName));
}



void CEditorMap::Clean()
{
	m_lGroups.delete_all();
	m_lEnvelopes.delete_all();
	m_lImages.delete_all();

	m_MapInfo.Reset();

	m_pGameLayer = 0x0;
	m_pGameGroup = 0x0;

	m_Modified = false;
}

void CEditorMap::CreateDefault()
{
	// add background
	CLayerGroup *pGroup = NewGroup();
	pGroup->m_ParallaxX = 0;
	pGroup->m_ParallaxY = 0;
	CLayerQuads *pLayer = new CLayerQuads;
	pLayer->m_pEditor = m_pEditor;
	CQuad *pQuad = pLayer->NewQuad();
	const int Width = i2fx(800);
	const int Height = i2fx(600);
	pQuad->m_aPoints[0].x = pQuad->m_aPoints[2].x = -Width;
	pQuad->m_aPoints[1].x = pQuad->m_aPoints[3].x = Width;
	pQuad->m_aPoints[0].y = pQuad->m_aPoints[1].y = -Height;
	pQuad->m_aPoints[2].y = pQuad->m_aPoints[3].y = Height;
	pQuad->m_aPoints[4].x = pQuad->m_aPoints[4].y = 0;
	pQuad->m_aColors[0].r = pQuad->m_aColors[1].r = 94;
	pQuad->m_aColors[0].g = pQuad->m_aColors[1].g = 132;
	pQuad->m_aColors[0].b = pQuad->m_aColors[1].b = 174;
	pQuad->m_aColors[2].r = pQuad->m_aColors[3].r = 204;
	pQuad->m_aColors[2].g = pQuad->m_aColors[3].g = 232;
	pQuad->m_aColors[2].b = pQuad->m_aColors[3].b = 255;
	pGroup->AddLayer(pLayer);

	// add game layer
	MakeGameGroup(NewGroup());
	MakeGameLayer(new CLayerGame(50, 50));
	m_pGameGroup->AddLayer(m_pGameLayer);
}

void CEditor::Init()
{
	m_pInput = Kernel()->RequestInterface<IInput>();
	m_pClient = Kernel()->RequestInterface<IClient>();
	m_pConsole = Kernel()->RequestInterface<IConsole>();
	m_pStorage = Kernel()->RequestInterface<IStorage>();
	m_Map.m_pEditor = this;


	m_TilesetPicker.m_pEditor = this;
	m_TilesetPicker.MakePalette();
	m_TilesetPicker.m_Readonly = true;

	m_Brush.m_pMap = &m_Map;

	Reset();
	m_Map.m_Modified = false;

#ifdef CONF_DEBUG
	m_pConsole->Register("map_magic", "i", CFGFLAG_CLIENT, ConMapMagic, this, "1-grass_doodads, 2-winter_main, 3-both");
#endif
}

static const char *s_aMaps[] = {"ctf1", "ctf2", "ctf3", "ctf4", "ctf5", "ctf6", "ctf7", "ctf8", "dm1", "dm2", "dm3", "dm6", "dm7", "dm8", "dm9", "lms1"};
static const char *s_aImageName[] = { "grass_doodads", "winter_main" };

static int s_GrassDoodadsIndicesOld[] = { 42, 43, 44, 58, 59, 60, 74, 75, 76, 132, 133, 148, 149, 163, 164, 165, 169, 170, 185, 186 };
static int s_GrassDoodadsIndicesNew[] = { 217, 218, 219, 233, 234, 235, 249, 250, 251, 182, 183, 198, 199, 213, 214, 215, 184, 185, 200, 201 };
static int s_WinterMainIndicesOld[] = { 166, 167, 168, 169, 170, 171, 182, 183, 184, 185, 186, 187, 198, 199, 200, 201, 202, 203, 174, 177, 178, 179, 180, 193, 194, 195, 196, 197, 209, 210, 211, 212, 215, 216, 231, 232, 248, 249, 250, 251, 252, 253, 254, 255, 229, 230, 224, 225, 226, 227, 228 };
static int s_WinterMainIndicesNew[] = { 218, 219, 220, 221, 222, 223, 234, 235, 236, 237, 238, 239, 250, 251, 252, 253, 254, 255, 95, 160, 161, 162, 163, 192, 193, 194, 195, 196, 176, 177, 178, 179, 232, 233, 248, 249, 240, 241, 242, 243, 244, 245, 246, 247, 224, 225, 226, 227, 228, 229, 230 };

void CEditor::ConMapMagic(IConsole::IResult *pResult, void *pUserData)
{
	CEditor *pSelf = static_cast<CEditor *>(pUserData);
	int Flag = pResult->GetInteger(0);

	for(unsigned m = 0; m < sizeof(s_aMaps) / sizeof(s_aMaps[0]); ++m)
	{
		char aBuf[64] = { 0 };
		str_format(aBuf, sizeof(aBuf), "maps/%s.map", s_aMaps[m]);
		dbg_msg("map magic", "processing map '%s'", s_aMaps[m]);
		CallbackOpenMap(aBuf, IStorage::TYPE_ALL, pSelf);
		bool Edited = false;

		// find image
		for(int i = 0; i < pSelf->m_Map.m_lImages.size(); ++i)
		{
			for(unsigned SrcIndex = 0; SrcIndex < sizeof(s_aImageName) / sizeof(s_aImageName[0]); ++SrcIndex)
			{
				if(((1 << SrcIndex)&Flag) && !str_comp(pSelf->m_Map.m_lImages[i]->m_aName, s_aImageName[SrcIndex]))
				{
					dbg_msg("map magic", "found image '%s'. doing magic", s_aImageName[SrcIndex]);
					pSelf->DoMapMagic(i, SrcIndex);
					Edited = true;
				}
			}
		}

		if(Edited)
		{
			str_format(aBuf, sizeof(aBuf), "maps/%s_mapmagic.map", s_aMaps[m]);
			dbg_msg("map magic", "saving map '%s_mapmagic'", s_aMaps[m]);
			CallbackSaveMap(aBuf, IStorage::TYPE_SAVE, pSelf);
		}
	}
}

void CEditor::DoMapMagic(int ImageID, int SrcIndex)
{
	for(int g = 0; g < m_Map.m_lGroups.size(); g++)
	{
		for(int i = 0; i < m_Map.m_lGroups[g]->m_lLayers.size(); i++)
		{
			if(m_Map.m_lGroups[g]->m_lLayers[i]->m_Type == LAYERTYPE_TILES)
			{
				CLayerTiles *pLayer = static_cast<CLayerTiles *>(m_Map.m_lGroups[g]->m_lLayers[i]);
				if(pLayer->m_Image == ImageID)
				{
					for(int Count = 0; Count < pLayer->m_Height*pLayer->m_Width; ++Count)
					{
						if(SrcIndex == 0)	// grass_doodads
						{
							for(unsigned TileIndex = 0; TileIndex < sizeof(s_GrassDoodadsIndicesOld) / sizeof(s_GrassDoodadsIndicesOld[0]); ++TileIndex)
							{
								if(pLayer->m_pTiles[Count].m_Index == s_GrassDoodadsIndicesOld[TileIndex])
								{
									pLayer->m_pTiles[Count].m_Index = s_GrassDoodadsIndicesNew[TileIndex];
									break;
								}
							}
						}
						else if(SrcIndex == 1)	// winter_main
						{
							for(unsigned TileIndex = 0; TileIndex < sizeof(s_WinterMainIndicesOld) / sizeof(s_WinterMainIndicesOld[0]); ++TileIndex)
							{
								if(pLayer->m_pTiles[Count].m_Index == s_WinterMainIndicesOld[TileIndex])
								{
									pLayer->m_pTiles[Count].m_Index = s_WinterMainIndicesNew[TileIndex];
									break;
								}
							}
						}
					}
				}
			}
		}
	}
}

void CEditor::DoMapBorder()
{
	CLayerTiles *pT = (CLayerTiles *)GetSelectedLayerType(0, LAYERTYPE_TILES);

	for(int i = 0; i < pT->m_Width*2; ++i)
		pT->m_pTiles[i].m_Index = 1;

	for(int i = 0; i < pT->m_Width*pT->m_Height; ++i)
	{
		if(i%pT->m_Width < 2 || i%pT->m_Width > pT->m_Width-3)
			pT->m_pTiles[i].m_Index = 1;
	}

	for(int i = (pT->m_Width*(pT->m_Height-2)); i < pT->m_Width*pT->m_Height; ++i)
		pT->m_pTiles[i].m_Index = 1;
}

void CEditor::UpdateAndRender()
{

}

IEditor *CreateEditor() { return new CEditor; }
