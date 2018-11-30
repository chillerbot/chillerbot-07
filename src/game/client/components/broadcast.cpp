/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <generated/protocol.h>
#include <generated/client_data.h>

#include <game/client/gameclient.h>
#include <stdio.h>

#include "broadcast.h"
#include "chat.h"
#include "scoreboard.h"
#include "motd.h"

#define BROADCAST_FONTSIZE_BIG 11.0f
#define BROADCAST_FONTSIZE_SMALL 6.5f

inline bool IsCharANum(char c)
{
	return c >= '0' && c <= '9';
}

inline int WordLengthBack(const char *pText, int MaxChars)
{
	int s = 0;
	while(MaxChars--)
	{
		if((*pText == '\n' || *pText == '\t' || *pText == ' '))
			return s;
		pText--;
		s++;
	}
	return 0;
}

inline bool IsCharWhitespace(char c)
{
	return c == '\n' || c == '\t' || c == ' ';
}

void CBroadcast::RenderServerBroadcast()
{
	if(!g_Config.m_ClShowServerBroadcast || m_pClient->m_MuteServerBroadcast)
		return;

	if(m_aSrvBroadcastMsg[0] == 0)
		return;

	if(m_pClient->m_pChat->IsActive())
		return;

	// draw lines
	const char* pBroadcastMsg = m_aSrvBroadcastMsg;

	// printf("broadcast: %s", pBroadcastMsg);
}

CBroadcast::CBroadcast()
{
	OnReset();
}

void CBroadcast::DoBroadcast(const char *pText)
{
	str_copy(m_aBroadcastText, pText, sizeof(m_aBroadcastText));
	m_BroadcastTime = Client()->LocalTime() + 10.0f;
}

void CBroadcast::OnReset()
{
	m_BroadcastTime = 0;
}

void CBroadcast::OnMessage(int MsgType, void* pRawMsg)
{
	// process server broadcast message
	if(MsgType == NETMSGTYPE_SV_BROADCAST && g_Config.m_ClShowServerBroadcast &&
	   !m_pClient->m_MuteServerBroadcast)
	{
		CNetMsg_Sv_Broadcast *pMsg = (CNetMsg_Sv_Broadcast *)pRawMsg;

		// new broadcast message
		int RcvMsgLen = str_length(pMsg->m_pMessage);
		mem_zero(m_aSrvBroadcastMsg, sizeof(m_aSrvBroadcastMsg));
		m_aSrvBroadcastMsgLen = 0;
		m_SrvBroadcastReceivedTime = Client()->LocalTime();

		const CBcColor White = { 255, 255, 255, 0 };
		m_aSrvBroadcastColorList[0] = White;
		m_SrvBroadcastColorCount = 1;

		CBcLineInfo UserLines[MAX_BROADCAST_LINES];
		int UserLineCount = 0;
		int LastUserLineStartPoint = 0;

		// parse colors
		for(int i = 0; i < RcvMsgLen; i++)
		{
			const char* c = pMsg->m_pMessage + i;
			const char* pTmp = c;
			int CharUtf8 = str_utf8_decode(&pTmp);
			const int Utf8Len = pTmp-c;

			if(*c == CharUtf8 && *c == '^')
			{
				if(i+3 < RcvMsgLen && IsCharANum(c[1]) && IsCharANum(c[2])  && IsCharANum(c[3]))
				{
					u8 r = (c[1] - '0') * 24 + 39;
					u8 g = (c[2] - '0') * 24 + 39;
					u8 b = (c[3] - '0') * 24 + 39;
					CBcColor Color = { r, g, b, m_aSrvBroadcastMsgLen };
					if(m_SrvBroadcastColorCount < MAX_BROADCAST_COLORS)
						m_aSrvBroadcastColorList[m_SrvBroadcastColorCount++] = Color;
					i += 3;
					continue;
				}
			}

			if(*c == CharUtf8 && *c == '\\')
			{
				if(i+1 < RcvMsgLen && c[1] == 'n' && UserLineCount < MAX_BROADCAST_LINES)
				{
					CBcLineInfo Line = { m_aSrvBroadcastMsg+LastUserLineStartPoint,
										 m_aSrvBroadcastMsgLen-LastUserLineStartPoint, 0 };
					if(Line.m_StrLen > 0)
						UserLines[UserLineCount++] = Line;
					LastUserLineStartPoint = m_aSrvBroadcastMsgLen;
					i++;
					continue;
				}
			}

			if(m_aSrvBroadcastMsgLen+Utf8Len < MAX_BROADCAST_MSG_LENGTH)
				m_aSrvBroadcastMsg[m_aSrvBroadcastMsgLen++] = *c;
		}

		// process boradcast message
		const char* pBroadcastMsg = m_aSrvBroadcastMsg;
		// const int MsgLen = m_aSrvBroadcastMsgLen;

		printf("broadcastMSG: %s\n", pBroadcastMsg);
	}
}

void CBroadcast::OnRender()
{
	if(m_pClient->m_pScoreboard->Active() || m_pClient->m_pMotd->IsActive())
		return;

	// client broadcast
	if(Client()->LocalTime() < m_BroadcastTime)
	{
		printf("client_broadcast: %s", m_aBroadcastText);
	}

	// server broadcast
	RenderServerBroadcast();
}

