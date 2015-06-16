#pragma once
#include "roboex.h"

#include "Ini.h"
#include <queue>
#include <vector>
#include <Afxmt.h>
#include <Afxtempl.h>

typedef struct TAG_USER_INFO{

	CString strName;
	int nCount;
	int nFirst;
	int nBlockStart;
	BOOL bWasHere;
	int nTriviaScore;
} USER_INFO, *PUSER_INFO;


typedef struct TAG_BOT_MESSAGE{

	CString strName;
	CString strTrigger;
	CString strResponse;
	DWORD   dwChannelID;
} BOT_MESSAGE, *PBOT_MESSAGE;

typedef struct TAG_COMMAND{

	CString strName;
	CString strTrigger;
	CString strResponse;
} COMMAND, *PCOMMAND;

typedef struct TAG_RANDCMD{

	CString strName;
	CString strTrigger;
	CStringArray strResonse;
	int  nNum;
} RANDCMD, *PRANDCMD, &RRANDCMD;

typedef struct TAG_ROOMDATA{

	CString strRoomName;
	DWORD   dwID;
	BOOL    bIsBotActivated;
	CUserArray* pUserArray;

} ROOMDATA, *PROOMDATA;

class CBot :
	public CRoboEx
{
public: // interface
	CBot(void);
	~CBot(void);

	virtual void Init();

	virtual void Quit(void);
	
	virtual void OnJoinChannel(DWORD dwID, CString strChannel, CUserArray* pUserArray);

	// Called when your own username changes, either because of
	// Userinteraction or if the client-id changes
	// new since API Version 0x1002L
	virtual void OnSetUserName(CString strNewUsername);

	// Called in WinMX 3.53 rooms when the channel name changes
	// new since API VERSION 0x1001L
	virtual void OnRenameChannel(DWORD dwID, CString strNewname);

	virtual void OnLeaveChannel(DWORD dwID);

	virtual void OnJoin(DWORD dwID, PMX_USERINFO pUser);

	virtual void OnRename(DWORD dwID, PMX_USERINFO pOld, PMX_USERINFO pNew);

	virtual void OnPart(DWORD dwID, PMX_USERINFO pUser);

	virtual void OnMessage(DWORD dwID, CString* pUser, CString* pMsg, BOOL bIsAction);

	virtual void OnOpMessage(DWORD dwID, CString* pUser, CString* pMsg);

	virtual void Configure(void);

	virtual void OnInputHook(DWORD dwID, CString* pInput);

public: // implementation and members


	CString m_strLast;
	CString m_strSeperator;
	int m_nFlood;
	
	CString GetFiles(DWORD dwID, CString strName);
	CString GetLineType(DWORD dwID, CString strName);
	void ReplaceVars(CString &strString, CString strName, DWORD dwID);
	void Bot(DWORD dwID, CString strNickname, CString strMessage);
	CString GetNick(CString strName);
	BOOL IsUserBlocked(CString strNickname);
	CString GetIP(DWORD dwID, CString strName);
	CString GetHostname(DWORD dwID, CString strName);
	CString GetBotUptime();
	CString GetSystemUptime();
	CString GetInetBeats();
	BOOL ContainsStringExact(CString strLine, CString strSubString, BOOL bCase = TRUE);
	void PrintAutoMessage();
	void ClearRandom();
	CString GetTrigger(CString strText);
	CString GetTriggerName(CString strText);

	CString m_strWelcome;
	CString m_strWb;
	CString m_strRename;
	CString	m_strBotMessage;
	CString	m_strBotname;
	CString	m_strBadLan;
	BOOL	m_bBadWord;
	UINT	m_nBotTicRem;
	int		m_nAutoKick;
	BOOL	m_bAutoKick;
	BOOL	m_bCostum;
	BOOL	m_bExclude;
	BOOL	m_bExtended;
	int		m_nSecs;
	int		m_nBlockTime;
	int		m_nBlockRange;
	int		m_nBlockMessageCount;

	CArray<ROOMDATA, ROOMDATA> m_aRooms;    // Array holding all rooms the bot is in
	std::queue<BOT_MESSAGE> m_qMessageQueue;
	CArray<USER_INFO, USER_INFO> m_vUserInfo;
	static UINT OutThread(PVOID pParams);
	BOOL m_bStopOut;
	CEvent m_eThreadFinished;
	CWinThread *m_pThread;
	// Events
	CArray<PRANDCMD, PRANDCMD> m_aRandom;
	CStringArray m_aBadWords;
	//Normal commands
	CArray<PCOMMAND, PCOMMAND> m_aNormal;
	//Extended commands
	CArray<PCOMMAND, PCOMMAND> m_aExtended;
	
	// Room management
	void		RemoveRoom(DWORD dwID);
	BOOL		AddRoom(DWORD dwID, CString strName, CUserArray* pUserArray);
	ROOMDATA&	GetRoom(DWORD dwID);
	CString		GetRoomName(DWORD dwID);
	DWORD		GetID(HWND hChat);

	// Users
	CStringArray m_aNoPenaltyUsers;
	CStringArray m_aExtendedUsers;
	CArray<USER_INFO, USER_INFO> m_aUserInfo; // Array holding wb names

	CString m_strBadLanRedirect;
	CString m_strRCMSMessage;
	CString m_strHelpString;
	CString m_strRCMSRedirect;
	
	CStringArray m_aWarnedNicks;
	int			 m_vWarnedCount[255];

	int		m_nBotStartTick;
	UINT	m_nBotTic;
	CString m_strWd;
	CString m_strRoboDir;
	CIni   m_Ini;

	// Members
	void AddLogLine(CString strMsg);
	void LoadSettings();
	void PrintNextMessage();
	void ParseIniFile();
	void HandleMessageSound(CString strUser, CString strEvent);

	BOOL IsProtectedUser(CString strUser);
	CString GetMyLocalTime();
	BOOL IsExtendedUser(CString strNickname);
	CString GetRawName(DWORD dwID, CString strNickname);
	CString GetName(CString strRawname);
	CString GetWinampSong();
	void RemoveWarnedUser(CString strUser);
	void IncreaseWarnedCount(CString strUser);
	int GetWarnedCount(CString strUser);
	BOOL IsUserWarned(CString strUser);


	// Trivia
	int IncreaseScore(CString& strUser);
	int GetScore(CString& strUser);
	void InitTrivia();
	void ClearGame();
	void SetWb(CString strUser);
	void RemoveUser(CString strUser);
	void AddUser(CString strUser);
	BOOL WasHere(CString &strUser);
	BOOL IsAdmin(CString& strNickname);
	void Trivia(DWORD dwID, CString strName, CString strMessage);
	
	// TRIVIA
	CString m_strTScore;
	CString m_strTNoAnswer;
	CString m_strTCorrect;
	CString m_strTSkip;
	CString m_strTNext;
	CArray<PCOMMAND, PCOMMAND> m_aTrivia;
	CArray<PCOMMAND, PCOMMAND> m_aGame;
	PCOMMAND m_pCurrent;
	BOOL   m_bTrivia;
	static UINT TriviaThread(PVOID pVoid);
	CWinThread* m_pTrivia;
	CEvent m_eTrivia;
	int m_nCurrent;
	DWORD m_dwTriviaID;
};
