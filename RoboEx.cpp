// RoboEx.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "RoboEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////
////
//    IMPORTANT: DO NOT CHANGE ANYTHING HERE
//      THIS FILE ONLY CONTAINS THE BASECLASS API DEFINITION
//      FOR YOUR PLUGIN. MAKE YOUR MODIFICATIONS IN
//      plugin.cpp. OVERRIDE THE MEMBERS YOU WANT TO USE
//      THERE!
////
//////////////////////////////////////////////////////////////

#define WM_ECHOTEXT		WM_APP+1
#define WM_ECHOSYSTEXT	WM_APP+2
#define WM_INPUT		WM_APP+3
#define WM_TOOLTIP		WM_APP+4
#define WM_PMSG			WM_APP+5

// CRoboEx

CRoboEx::CRoboEx()
{

	m_strName = "Default Name";
	m_strAuthor = "Default Author";
	m_strDescription = "Default Description";
}

CRoboEx::~CRoboEx()
{
}

// CRoboEx member functions

void CRoboEx::Init()
{

}

void CRoboEx::Quit(void)
{

}

// Called when client joins a channel
void CRoboEx::OnJoinChannel(DWORD dwID, CString strChannel, CUserArray* pUserArray)
{
}

// Called in WinMX 3.53 rooms when the channel name changes
// new since API VERSION 0x1001L
void CRoboEx::OnRenameChannel(DWORD dwID, CString strNewname)
{
}

// Called when your own username changes, either because of
// Userinteraction or if the client-id changes
// new since API Version 0x1002L
void CRoboEx::OnSetUserName(CString strNewUsername)
{
}

// Called when client leaves a channel
void CRoboEx::OnLeaveChannel(DWORD dwID)
{
}

void CRoboEx::OnJoin(DWORD dwID, PMX_USERINFO pUser)
{
}

// Called when a user in the channel changes an attribute
void CRoboEx::OnRename(DWORD dwID, PMX_USERINFO pOld, PMX_USERINFO pNew)
{
}

void CRoboEx::OnPart(DWORD dwID, PMX_USERINFO pUser)
{
}

void CRoboEx::OnMessage(DWORD dwID, CString* pUser, CString* pMsg, BOOL bIsAction)
{
}

// Called when a operator writes a message in the channel and the user has permission to see it
void CRoboEx::OnOpMessage(DWORD dwID, CString* pUser, CString* pMsg)
{
}

void CRoboEx::OnInputHook(DWORD dwID, CString* pInput)
{
}

// Called when a new PM arrives. This is for later use and wil never be called at the moment
void CRoboEx::OnPrivateMsg(CString* pSender, CString* pMessage, DWORD dwNode, WORD wPort)
{
}

void CRoboEx::Configure(void)
{

	AfxMessageBox("Nothing to configure for this plugin", MB_ICONINFORMATION);
}

// Called when a user selects an item from a menu. nCmd is the command identifer, 
//dwID is the ID of the channel. this is -1 if the command is not channel related
void CRoboEx::OnMenuCommand(UINT nCmd, DWORD dwID)
{
}

// Call this to write text on the chat window. Do not override this method
void CRoboEx::WriteEchoText(DWORD dwID, CString strText, COLORREF crText, COLORREF crBg)
{

	AFX_MANAGE_STATE(AfxGetModuleState());
	
	EXT_MSG_STRUCT msg;
	msg.lpszMsg = strText;
	msg.wParam  = (WPARAM)crText;
	msg.lParam  = (LPARAM)crBg;
	AfxGetApp()->m_pMainWnd->SendMessage(WM_ECHOTEXT, (WPARAM)dwID, (LPARAM)&msg);
}

// Call this method to write text on the system console of the chat. Do not override this method!
void CRoboEx::WriteSystemEchoText(DWORD dwID, CString strMsg, COLORREF crText, COLORREF crBg)
{

	AFX_MANAGE_STATE(AfxGetModuleState());

	EXT_MSG_STRUCT msg;
	msg.lpszMsg = strMsg;
	msg.wParam  = (WPARAM)crText;
	msg.lParam  = (LPARAM)crBg;
	AfxGetApp()->m_pMainWnd->SendMessage(WM_ECHOSYSTEXT, (WPARAM)dwID, (LPARAM)&msg);
}

// Call this method to Input text into the chat. Do not override this method
void CRoboEx::InputMessage(DWORD dwID, CString strMsg)
{

	AFX_MANAGE_STATE(AfxGetModuleState());

	EXT_MSG_STRUCT msg;
	msg.lpszMsg = strMsg;
	msg.wParam  = 0;
	msg.lParam  = 0;
	AfxGetApp()->m_pMainWnd->SendMessage(WM_INPUT, (WPARAM)dwID, (LPARAM)&msg);
}

void CRoboEx::DisplayToolTip(CString strMessage, UINT uTimeout, DWORD dwIcon)
{

	AFX_MANAGE_STATE(AfxGetModuleState());

	EXT_MSG_STRUCT msg;
	msg.lpszMsg = strMessage;
	msg.wParam  = (WPARAM)uTimeout;
	msg.lParam  = (LPARAM)dwIcon;
	AfxGetApp()->m_pMainWnd->SendMessage(WM_TOOLTIP, 0, (LPARAM)&msg);
}

// Call this methot to send a PM to a user <strName>
// dwTargetNode and wTargetPort must be the node of the recipient
// (get from the userarray ;)
void CRoboEx::SendPrivateMsg(DWORD dwTargetNode, WORD wTargetPort, CString strName, CString strMsg)
{

	AFX_MANAGE_STATE(AfxGetModuleState());

	EXT_MSG_STRUCT msg;
	msg.lpszParam = strName;
	msg.lpszMsg = strMsg;
	msg.wParam  = (WPARAM)dwTargetNode;
	msg.lParam  = (LPARAM)wTargetPort;
	AfxGetApp()->m_pMainWnd->SendMessage(WM_PMSG, 0, (LPARAM)&msg);
}

