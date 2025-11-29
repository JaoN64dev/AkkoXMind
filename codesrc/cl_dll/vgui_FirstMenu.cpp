#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"


CFirstMenu::CFirstMenu(int iTrans, int iRemoveMe, int x, int y, int wide, int tall) : CMenuPanel(iTrans, iRemoveMe, x, y, wide, tall)
{
	m_pCancelButton = new CommandButton(gHUD.m_TextMessage.BufferedLocaliseTextString("Cancel"), 5, 5, XRES(75), YRES(30));
	m_pCancelButton->setParent(this);
	m_pCancelButton->addActionSignal(new CMenuHandler_TextWindow(HIDE_TEXTWINDOW));
}