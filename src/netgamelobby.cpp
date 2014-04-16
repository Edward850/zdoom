/*
** netgamelobby.cpp
** The menu used to display an active netgame lobby
**
**---------------------------------------------------------------------------
** Copyright 2001-2010 Randy Heit
** Copyright 2010 Christoph Oelckers
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
*/

#include "menu/menu.h"
#include "v_text.h"
#include "v_palette.h"
#include "gi.h"
#include "d_gui.h"
#include "i_system.h"
#include "version.h"
#include "g_game.h"
#include "m_png.h"
#include "w_wad.h"
#include "d_event.h"
#include "gstrings.h"
#include "doomstat.h"
#include "d_gui.h"
#include "d_net.h"

class DNetgamelobby : public DListMenu
{
	DECLARE_CLASS(DNetgamelobby, DListMenu)
	friend void ClearPlayers();

protected:
	static TArray<FPlayerNode*> Players;

	int Selected;
	int TopItem;

	int rowHeight;
	int listboxLeft;
	int listboxTop;
	int listboxWidth;

	int listboxRows;
	int listboxHeight;
	int listboxRight;
	int listboxBottom;

	int commentLeft;
	int commentTop;
	int commentWidth;
	int commentHeight;
	int commentRight;
	int commentBottom;

	static int InsertPlayerNode(FPlayerNode *node);
	static void ReadPlayerStrings();

	DNetgamelobby(DMenu *parent = NULL, FListMenuDescriptor *desc = NULL);
	void Destroy();

	void Drawer();
	bool MenuEvent(int mkey, bool fromcontroller);
	bool MouseEvent(int type, int x, int y);
	bool Responder(event_t *ev);

public:
	static void NotifyNewPlayer();

};

IMPLEMENT_CLASS(DNetgamelobby)

TArray<FPlayerNode*> DNetgamelobby::Players;

void ClearPlayers()
{
	for (unsigned i = 0; i<DNetgamelobby::Players.Size(); i++)
	{
		delete DNetgamelobby::Players[i];
	}
	DNetgamelobby::Players.Clear();
}

void DNetgamelobby::Drawer()
{
	Super::Drawer();

	FPlayerNode *node;
	int i;
	unsigned j;
	bool didSeeSelected = false;

	// Draw comment area
	V_DrawFrame(commentLeft, commentTop, commentWidth, commentHeight);
	screen->Clear(commentLeft, commentTop, commentRight, commentBottom, 0, 0);

	// Draw file area
	V_DrawFrame(listboxLeft, listboxTop, listboxWidth, listboxHeight);
	screen->Clear(listboxLeft, listboxTop, listboxRight, listboxBottom, 0, 0);

	if (Players.Size() == 0)
	{
		const char * text = GStrings("MNU_NOPLAYERS");
		const int textlen = SmallFont->StringWidth(text)*CleanXfac;

		screen->DrawText(SmallFont, CR_GOLD, listboxLeft + (listboxWidth - textlen) / 2,
			listboxTop + (listboxHeight - rowHeight) / 2, text,
			DTA_CleanNoMove, true, TAG_DONE);
		return;
	}

	for (i = 0, j = TopItem; i < listboxRows && j < Players.Size(); i++, j++)
	{
		int color;
		node = Players[j];
		if (node->host)
		{
			color = CR_CREAM;
		}
		/*else if (node->bMissingWads)
		{
			color = CR_ORANGE;
		}*/
		else if ((int)j == Selected)
		{
			color = CR_WHITE;
		}
		else
		{
			color = CR_GREEN;
		}

		if ((int)j == Selected)
		{
			screen->Clear(listboxLeft, listboxTop + rowHeight*i,
				listboxRight, listboxTop + rowHeight*(i + 1), -1,
				MAKEARGB(255, 0, 0, 255));
			didSeeSelected = true;

			screen->DrawText(SmallFont, color,
				listboxLeft + 1, listboxTop + rowHeight*i + CleanYfac, node->name,
				DTA_CleanNoMove, true, TAG_DONE);
			
		}
		else
		{
			screen->DrawText(SmallFont, color,
				listboxLeft + 1, listboxTop + rowHeight*i + CleanYfac, node->name,
				DTA_CleanNoMove, true, TAG_DONE);
		}
	}
}

DNetgamelobby::DNetgamelobby(DMenu *parent, FListMenuDescriptor *desc)
: DListMenu(parent, desc)
{
	ReadPlayerStrings();
	
	Selected = TopItem = 0;

	int savepicLeft = 10;
	int savepicTop = 54 * CleanYfac;
	int savepicWidth = 216 * screen->GetWidth() / 640;
	int savepicHeight = 135 * screen->GetHeight() / 400;

	rowHeight = (SmallFont->GetHeight() + 1) * CleanYfac;
	listboxLeft = savepicLeft + savepicWidth + 14;
	listboxTop = savepicTop;
	listboxWidth = screen->GetWidth() - listboxLeft - 10;
	int listboxHeight1 = screen->GetHeight() - listboxTop - 10;
	listboxRows = (listboxHeight1 - 1) / rowHeight;
	listboxHeight = listboxRows * rowHeight + 1;
	listboxRight = listboxLeft + listboxWidth;
	listboxBottom = listboxTop + listboxHeight;

	commentLeft = savepicLeft;
	commentTop = savepicTop + savepicHeight + 16;
	commentWidth = savepicWidth;
	commentHeight = (51 + (screen->GetHeight()>200 ? 10 : 0))*CleanYfac;
	commentRight = commentLeft + commentWidth;
	commentBottom = commentTop + commentHeight;
}

void DNetgamelobby::Destroy()
{
	ClearPlayers();
}

bool DNetgamelobby::MenuEvent(int mkey, bool fromcontroller)
{
	return Super::MenuEvent(mkey, fromcontroller);
}

bool DNetgamelobby::MouseEvent(int type, int x, int y)
{
	return Super::MouseEvent(type, x, y);
}

bool DNetgamelobby::Responder(event_t *ev)
{
	return Super::Responder(ev);
}

void DNetgamelobby::ReadPlayerStrings()
{
	int i;
	//char buff[MAXPLAYERNAME];
	if (Players.Size() != 0) ClearPlayers();

	for (i = 0; i < MAXNETNODES; i++)
	{
		if (i < doomcom.numnodes)
		{
			FPlayerNode *node = new FPlayerNode;
			//sprintf(buff, "node %d", i);
			node->name = netHandshake.names[i];
			node->host = (i == 0) ? true : false;
			InsertPlayerNode(node);
		}
	}
}

int DNetgamelobby::InsertPlayerNode(FPlayerNode *node)
{
	return Players.Push(node);
}

void DNetgamelobby::NotifyNewPlayer()
{
	ReadPlayerStrings();
}

void M_NotifyNewPlayer()
{
	DNetgamelobby::NotifyNewPlayer();
}