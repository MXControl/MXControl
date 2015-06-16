#include "StdAfx.h"
#include ".\bot.h"

#include "wa_ipc.h"

#define STR_VERSION "1.3.3"

const char* line_types[] = {"Unknow", "14.4K", "28.8K", "33.3K", "56K", 
                            "64K ISDN", "128K ISDN", "Cable", "DSL", "T1", "T3"};


CBot::CBot(void)
{

	m_strName = "MXControl 1.3.3";
	m_strAuthor = "Thees Ch. Winkler";
	m_strDescription = "MXControl - Simple Bot for WinMX";



	m_nBotTic = 0;
	m_strBotMessage = _T("[The host was too lazy to set this up properly.]");
	m_strBotname = _T("The mighty Chatbot :o)");
	m_strBadLan = _T("");
	m_bBadWord = TRUE;
	m_nBotTicRem = 0;
	m_nAutoKick = 3;
	m_bAutoKick = TRUE;
	m_bCostum = TRUE;
	m_bExclude = FALSE;
	m_bExtended = FALSE;
	m_nSecs = 99;
	m_nBlockTime = 0;
	m_nBlockRange = 0;
	m_nBlockMessageCount = 0;
	m_strSeperator = "._ ";
	m_bTrivia = FALSE;
}

CBot::~CBot(void)
{
}

void CBot::Init()
{

	TCHAR szBuffer[_MAX_PATH]; 
	::GetModuleFileName(AfxGetInstanceHandle(), szBuffer, _MAX_PATH);
	m_strWd.Format("%s", szBuffer);
	m_strRoboDir = m_strWd.Left(m_strWd.ReverseFind('\\'));
	m_strWd = m_strWd.Left(m_strWd.ReverseFind('\\')) + "\\Add-Ons";
	
	LoadSettings();
	ParseIniFile();
	m_bStopOut = FALSE;
	srand(GetTickCount());
	
	m_eTrivia.SetEvent();
	m_eThreadFinished.ResetEvent();
	// Fire of thread
	AfxBeginThread(OutThread, (void*)this, THREAD_PRIORITY_NORMAL);

	m_nBotStartTick = GetTickCount();
}

void CBot::Quit(void)
{

	m_bStopOut = TRUE;
	m_bTrivia = FALSE;

	WaitForSingleObject(m_eTrivia, INFINITE);
	WaitForSingleObject(m_eThreadFinished, INFINITE);
	//m_pThread = NULL;

	m_aBadWords.RemoveAll();
	ClearRandom();
}

void CBot::OnInputHook(DWORD dwID, CString* pInput)
{

	CString strCmd = *pInput;

	if(strCmd == "#LOAD_CFG")
	{

		if(m_bTrivia){

			m_bTrivia = FALSE;
			WaitForSingleObject(m_eTrivia, INFINITE);
			WriteEchoText(dwID, "MCP: Trivia stopped\n", RGB(255, 255, 0), RGB(0, 0, 120));
			ClearGame();
		}
		LoadSettings();
		ParseIniFile();
		WriteEchoText(dwID, "MXControl: Configuration file reparsed\n", RGB(255,0,0), RGB(255,255,255));
		*pInput = "";
	}

	if(strCmd == "#EDIT_CONFIG")
	{

		ShellExecute(NULL, "open", m_strWd + "\\MXC.ini", NULL, m_strWd, SW_SHOW);
		WriteEchoText(dwID, "MXControl: Type #REPARSE_CONFIG when you are done editing the config file.\n", RGB(255,0,0), RGB(255,255,255));
		*pInput = "";
	}

	if(strCmd == "#CONFIG")
	{

		ShellExecute(NULL, "open", m_strWd + "\\Configurator.exe", NULL, m_strWd, SW_SHOW);
		*pInput = "";
	}

	if(strCmd == "#RUN")
	{

		CString strOut;
		GetRoom(dwID).bIsBotActivated = !GetRoom(dwID).bIsBotActivated;
		strOut.Format((GetRoom(dwID).bIsBotActivated ? "MXControl: Bot activated.\n" : "MXControl: Bot halted.\n"));
		WriteEchoText(dwID, strOut, RGB(255,0,0), RGB(255,255,255));
		*pInput = "";
	}
	else if(strCmd == "#TRIVIA"){

		if(!GetRoom(dwID).bIsBotActivated){

			WriteEchoText(dwID, "MXControl: Bot is not running. Type '#RUN' first.\n", RGB(255, 255, 0), RGB(0, 0, 120));
		}
		else{

			if(m_bTrivia){

				m_dwTriviaID = 0;
				WriteEchoText(dwID, "MXControl: Shutting down Trivia game. Stand by...\n", RGB(255, 255, 0), RGB(0, 0, 120));
				m_bTrivia = FALSE;
				WaitForSingleObject(m_eTrivia, INFINITE);
				ClearGame();
				WriteEchoText(dwID, "MXControl: Trivia stopped\n", RGB(255, 255, 0), RGB(0, 0, 120));
			}
			else{
				
				if(m_aTrivia.GetSize() == 0){
					
					WriteEchoText(dwID, "MXControl: No questions for Trivia found... Please add questions first!\n", RGB(255, 255, 0), RGB(0, 0, 120));
					*pInput = "";
					return;
				}

				m_dwTriviaID = dwID;
				WriteEchoText(dwID, "MPC: Randomizing questions. Stand by...\n", RGB(255, 255, 0), RGB(0, 0, 120));
				InitTrivia();
				m_eTrivia.ResetEvent();
				m_bTrivia = TRUE;
				InputMessage(dwID, "Trivia Commands: SCORE - Display userscore, RANKING - Display top 3 ranking, SKIP - Skip question (Admins only)");
				AfxBeginThread(TriviaThread, (LPVOID)this, THREAD_PRIORITY_NORMAL);
				WriteEchoText(dwID, "MCP: Trivia started\n", RGB(255, 255, 0), RGB(0, 0, 120));
			}
		}
		*pInput = "";
	}
}


void CBot::OnJoinChannel(DWORD dwID, CString strChannel, CUserArray* pUserArray)
{

	AddRoom(dwID, strChannel, pUserArray);

	CString strVersion;
	strVersion.Format("This is MXControl %s\n", STR_VERSION);

	WriteEchoText(dwID, strVersion, RGB(253,171,0), RGB(255,255,255));
    WriteEchoText(dwID, "Available commands to control this BOT:\n", RGB(253,171,0), RGB(255,255,255));
	WriteEchoText(dwID, "#CONFIG \t- Start configuration tool\n", RGB(253,171,0), RGB(255,255,255));
	WriteEchoText(dwID, "#EDIT_CONFIG - Manually edit configuration file in notepad\n", RGB(253,171,0), RGB(255,255,255));
	WriteEchoText(dwID, "#LOAD_CFG \t- Reload configuration file after you editing.\n", RGB(253,171,0), RGB(255,255,255));
	WriteEchoText(dwID, "#RUN\t\t- Run/Stop the bot\n", RGB(253,171,0), RGB(255,255,255));
	WriteEchoText(dwID, "#TRIVIA\t- Run/Stop a trivia game\n\n", RGB(253,171,0), RGB(255,255,255));
}


// Called when your own username changes, either because of
// Userinteraction or if the client-id changes
// new since API Version 0x1002L
void CBot::OnSetUserName(CString strNewUsername)
{

	m_strBotname = strNewUsername;		
}

// Called in WinMX 3.53 rooms when the channel name changes
// new since API VERSION 0x1001L
void CBot::OnRenameChannel(DWORD dwID, CString strNewname)
{

	GetRoom(dwID).strRoomName = strNewname;
}


void CBot::OnLeaveChannel(DWORD dwID)
{

	RemoveRoom(dwID);
}

void CBot::OnJoin(DWORD dwID, PMX_USERINFO pUser)
{

	if(!GetRoom(dwID).bIsBotActivated) return;

	CString strNickname = pUser->strUser;

	if(WasHere(strNickname)){

		BOT_MESSAGE b;
		b.strName = strNickname;
		b.strResponse = m_strWb;
		b.strResponse.Replace("%NODE-IP%", pUser->strNodeIP);
		b.strResponse.Replace("%REAL-IP%", pUser->strRealIP);
		b.dwChannelID = dwID;
		b.strTrigger = "wb";
		m_qMessageQueue.push(b);
	}
	else{

		BOT_MESSAGE b;
		b.strName = strNickname;
		b.strResponse = m_strWelcome;
		b.strResponse.Replace("%NODE-IP%", pUser->strNodeIP);
		b.strResponse.Replace("%REAL-IP%", pUser->strRealIP);
		b.dwChannelID = dwID;
		b.strTrigger = "welcome";
		AddUser(strNickname);
		m_qMessageQueue.push(b);
	}
}

void CBot::OnRename(DWORD dwID, PMX_USERINFO pOld, PMX_USERINFO pNew)
{

	if(!m_aRooms.GetSize()) return;
	if(!GetRoom(dwID).bIsBotActivated || m_strRename.IsEmpty()) return;
	

	BOT_MESSAGE b;
	b.dwChannelID = dwID;
	b.strName = pOld->strUser;
	b.strResponse = m_strRename;
	b.strResponse.Replace("%OLD%",     GetName(pOld->strUser));
	b.strResponse.Replace("%NEW%",	   GetName(pNew->strUser));
	b.strResponse.Replace("%OLDRAW%",  pOld->strUser);
	b.strResponse.Replace("%NEWRAW%",  pNew->strUser);
	b.strResponse.Replace("%NODE-IP%",      pNew->strNodeIP);
	b.strResponse.Replace("%REAL-IP%", pNew->strRealIP);
	b.strTrigger = "rename";

	m_qMessageQueue.push(b);
}

void CBot::OnPart(DWORD dwID, PMX_USERINFO pUser)
{

	if(!m_aRooms.GetSize()) return;
	if(!GetRoom(dwID).bIsBotActivated) return;

	CString strMessage;
	Bot(dwID, pUser->strUser, "has left");
	RemoveUser(pUser->strUser);
}

void CBot::OnMessage(DWORD dwID, CString* pUser, CString* pMsg, BOOL bIsAction)
{

	if(!m_aRooms.GetSize()) return;
	if(!GetRoom(dwID).bIsBotActivated) return;
	
	Bot(dwID, *pUser, *pMsg);
	if(m_bTrivia){

		Trivia(dwID, *pUser, *pMsg);
	}
}

void CBot::OnOpMessage(DWORD dwID, CString* pUser, CString* pMsg)
{

	if(!m_aRooms.GetSize()) return;
	if(!GetRoom(dwID).bIsBotActivated) return;
	
	Bot(dwID, *pUser, *pMsg);
	if(m_bTrivia){

		Trivia(dwID, *pUser, *pMsg);
	}
}

void CBot::Configure(void)
{
	
	ShellExecute(NULL, "open", m_strWd + "\\Configurator.exe", NULL, m_strWd, SW_SHOW);
}

// Add a room
BOOL CBot::AddRoom(DWORD dwID, CString strName, CUserArray* pUserArray)
{

	for(int i = 0; i < m_aRooms.GetSize(); i++){

		if(m_aRooms[i].dwID == dwID){
			
			m_aRooms[i].strRoomName = strName;
			m_aRooms[i].pUserArray = pUserArray;
			return TRUE;
		}
	}

	ROOMDATA r;
	r.dwID = dwID;
	r.strRoomName = strName;
	r.bIsBotActivated = FALSE;
	r.pUserArray = pUserArray;
	m_aRooms.Add(r);
	
	return FALSE;
}


// Get room assigned to ID
ROOMDATA& CBot::GetRoom(DWORD dwID)
{

	for(int i = 0; i < m_aRooms.GetSize(); i++){

		if(m_aRooms[i].dwID == dwID){

			break;
		}
	}
	return m_aRooms[i];
}

// remove room assigned to ID
void CBot::RemoveRoom(DWORD dwID)
{

	for(int i = 0; i < m_aRooms.GetSize(); i++){

		if(m_aRooms[i].dwID == dwID){

			m_aRooms.RemoveAt(i);
			return;
		}
	}
}


void CBot::LoadSettings()
{

	AddLogLine("Initializing...");

	//m_Ini.SetIniFileName(m_strRoboDir + "\\RoboMX.ini");
	//m_strBotname = m_Ini.GetValue("UserInfo", "Nickname", "The mighty Chatbot :o)");

	m_Ini.SetIniFileName(m_strWd + "\\MXC.ini");
	
	m_strBadLan = m_Ini.GetValue("Messages", "BadLanWarning", "I'd watch that tone of voice, %NAME%, if I was you! (You get Autokicked!)");
	m_strBadLanRedirect = m_Ini.GetValue("Messages", "BadLanRedirectComment", "You have been warned, %NAME%. Final words are not permitted!");
	m_strBotMessage = m_Ini.GetValue("Messages", "Botmessage", "[The host was too lazy to set this up]");
	m_strHelpString = m_Ini.GetValue("Messages", "Help", "Sorry %NAME%, but the lazy host did not set up a Help command yet.");
	m_strWelcome = m_Ini.GetValue("Messages", "Welcome", "Hello, %NAME%.");
	m_strWb = m_Ini.GetValue("Messages", "WelcomeBack", "Welcome back, %NAME%");
	m_strRename = m_Ini.GetValue("Messages", "Rename", "%OLDNAME% is now know as %NEWNAME%");
	
	//m_strBotname = m_Ini.GetValue("Settings", "Botname", "The mighty Chatbot :o)");
	m_nSecs = m_Ini.GetValue("Settings", "BotTic", 1800);
	m_bAutoKick = m_Ini.GetValue("Settings", "Autokick", TRUE);
	m_nAutoKick = m_Ini.GetValue("Settings", "AutokickAfter", 3);
	m_bBadWord = m_Ini.GetValue("Settings", "BadLanPenalty", TRUE);
	m_bCostum = m_Ini.GetValue("Settings", "EnableCostum", TRUE);
	m_bExtended = m_Ini.GetValue("Settings", "EnableExtened", TRUE);
	m_bExclude = m_Ini.GetValue("Settings", "ExcludeFromPenalty", TRUE);
	m_nFlood   = m_Ini.GetValue("Settings", "FloodProtection", 2) * 1000;

	m_nBlockTime = m_Ini.GetValue("Settings", "FloodBlockTime", 240) * 1000;
	m_nBlockRange = m_Ini.GetValue("Settings", "FloodBlockRange", 10) * 1000;
	m_nBlockMessageCount = m_Ini.GetValue("Settings", "FloodBlockMaxMessages", 5);
	m_strSeperator = m_Ini.GetValue("Settings", "NickSeperators", "._ ");

	m_strTScore = m_Ini.GetValue("Trivia", "Score", "%NAME% - Your current score is %SCORE% points.");
	m_strTNoAnswer = m_Ini.GetValue("Trivia", "NoAnswer", "This question was not answered in the given time. The correct answer is %ANS%.");
	m_strTCorrect = m_Ini.GetValue("Trivia", "Correct", "%NAME% answered correctly (%ANS%)!");
	m_strTSkip = m_Ini.GetValue("Trivia", "Skip", "Question %QUES% skipped, the answer was %ANS%");
	m_strTNext = m_Ini.GetValue("Trivia", "NextQuestion", "Question: %QUES%");

	CString strTmp2;
	CString strTmp = m_Ini.GetValue("Settings", "NoPenaltyUser", "");
	
	int nStart = 0, nEnd = 0;

	while((nEnd = strTmp.Find(";", nStart)) > NULL){

		strTmp2 = strTmp.Mid(nStart, nEnd - nStart);
		nStart = nEnd+1;
		m_aNoPenaltyUsers.Add(strTmp2);
		AddLogLine(strTmp2 + " was added as a proteced user");
	}
	if(strTmp.GetLength()){
		
		strTmp2 = strTmp.Mid(nStart, strTmp.GetLength() - nStart);
		m_aNoPenaltyUsers.Add(strTmp2);
		AddLogLine(strTmp2 + " was added as a protected user");
	}

	strTmp = m_Ini.GetValue("Settings", "ExtendedUser", "");
	
	nStart = 0, nEnd = 0;

	while((nEnd = strTmp.Find(";", nStart)) > NULL){

		strTmp2 = strTmp.Mid(nStart, nEnd - nStart);
		nStart = nEnd+1;
		m_aExtendedUsers.Add(strTmp2);
		AddLogLine(strTmp2 + " was added as an extended user");
	}
	if(strTmp.GetLength()){
		
		strTmp2 = strTmp.Mid(nStart, strTmp.GetLength() - nStart);
		m_aExtendedUsers.Add(strTmp2);
		AddLogLine(strTmp2 + " was added as a extended user");
	}


	AddLogLine("Ready");
}


void CBot::ParseIniFile()
{

	CString strIniFile = m_strWd + "\\MXC.ini";

	CStdioFile ini;
	CString strBuffer;
	int nMode = -1;
	
	m_aBadWords.RemoveAll();
	ClearRandom();

	PRANDCMD rc;
	PCOMMAND cmd;
	TRY{

		ini.Open(strIniFile, CFile::modeCreate|CFile::modeNoTruncate|CFile::modeRead|CFile::typeText);

		while(ini.ReadString(strBuffer)){

			strBuffer.TrimLeft();
			strBuffer.TrimRight();
			//strBuffer.MakeUpper();

			if(strBuffer.Left(2) == "//") continue;
			if(strBuffer.Left(13) == "#TRIVIA_START"){
				
				nMode = 0;
				continue;
			}
			else if(strBuffer.Left(11) == "#TRIVIA_END"){
				
				nMode = -1;
				continue;
			}
			else if(strBuffer.Left(14) == "#BADWORD_START"){

				nMode = 1;
				continue;
			}
			else if(strBuffer.Left(12) == "#BADWORD_END"){

				nMode = -1;
				continue;
			}
			else if(strBuffer.Left(13) == "#NORMAL_START"){

				nMode = 3;
				continue;
			}
			else if(strBuffer.Left(11) == "#NORMAL_END"){

				nMode = -1;
				continue;
			}
			else if(strBuffer.Left(15) == "#EXTENDED_START"){

				nMode = 4;
				continue;
			}
			else if(strBuffer.Left(13) == "#EXTENDED_END"){

				nMode = -1;
				continue;
			}
			else if(strBuffer.Left(14) == "#RANDOM_START_"){

				nMode = 5;
			}

			if(strBuffer.Left(1) != "#"){

				switch(nMode){
					
				case 1: // badword list
					if(! strBuffer.IsEmpty()){

						strBuffer.MakeLower();
						m_aBadWords.Add(strBuffer);
					}
					continue;
				case 5: // random
					rc = new RANDCMD;
					//ini.ReadString(strBuffer);
					rc->strName = GetTriggerName(strBuffer);
					rc->strTrigger = GetTrigger(strBuffer);
					while(ini.ReadString(strBuffer)){

						if(strBuffer.IsEmpty()) continue;
						if(strBuffer.Left(11) == "#RANDOM_END") break;
						rc->strResonse.Add(strBuffer);
					}
					rc->nNum = (int)rc->strResonse.GetSize();
					m_aRandom.Add(rc);
					nMode = -1;
					continue;
				default:
					continue;
				}
			}
			else{
				
				if(strBuffer.IsEmpty() && (nMode != 0)){

					continue;
				}


				CString strEvent, strResp;
				strEvent = strBuffer.Mid(1, strBuffer.Find("#",2)-1);
				strResp = strBuffer.Mid(strBuffer.Find("\"",4)+1, strBuffer.ReverseFind('\"')-strBuffer.Find("\"", 4)-1);
				
				if(strEvent.IsEmpty()){

					continue;
				}
				switch(nMode){
				
				case 0: // trivia
					cmd = new COMMAND;
					cmd->strName = GetTriggerName(strEvent);
					cmd->strTrigger = GetTrigger(strEvent);
					cmd->strResponse = strResp;
					m_aTrivia.Add(cmd);
					break;
				case 3:
					//strEvent.MakeLower();
					cmd = new COMMAND;
					cmd->strName = GetTriggerName(strEvent);
					cmd->strTrigger = GetTrigger(strEvent);
					cmd->strResponse = strResp;
					m_aNormal.Add(cmd);
					break;
				case 4:
					cmd = new COMMAND;
					cmd->strName = GetTriggerName(strEvent);
					cmd->strTrigger = GetTrigger(strEvent);
					cmd->strResponse = strResp;
					m_aExtended.Add(cmd);
					break;
				default:
					TRACE("HELP\n");

				}

			}
		}
		ini.Close();
	}
	CATCH(CFileException, e){

		AfxMessageBox("Error during file operation!", MB_OK+MB_ICONSTOP);

	}END_CATCH;
}

UINT CBot::OutThread(PVOID pParams)
{

	CBot *pPlugin = (CBot*)pParams;
	
	while(!pPlugin->m_bStopOut){

		pPlugin->PrintNextMessage();

		if(pPlugin->m_nBotTic >= (UINT)pPlugin->m_nSecs){

			pPlugin->PrintAutoMessage();
			pPlugin->m_nBotTic = 0;
		}

		SleepEx(pPlugin->m_nFlood, FALSE);
		if(pPlugin->m_nSecs != 99){

			pPlugin->m_nBotTic++;
		}
	}
	
	pPlugin->m_eThreadFinished.SetEvent();

	return 0;
}

void CBot::AddLogLine(CString strMsg)
{

	TRACE(strMsg);
}

void CBot::PrintNextMessage()
{

	if(m_qMessageQueue.empty()){

		return;
	}

	CString strBotMessage = m_qMessageQueue.front().strResponse;
	CString strInMsg = m_qMessageQueue.front().strTrigger;
	CString strNickname = m_qMessageQueue.front().strName;
	DWORD   dwChannelID = m_qMessageQueue.front().dwChannelID;

	m_qMessageQueue.pop();
	
	// Autokick stuff
	if((strBotMessage == m_strBadLan) && m_bAutoKick){

		if(!IsUserWarned(strNickname) && !IsProtectedUser(strNickname)){

			AddLogLine("Bad Language: " + strNickname + " (first warning)");
			m_vWarnedCount[m_aWarnedNicks.Add(strNickname)] = 1;
		}
		else{

			int nWarnedCnt = GetWarnedCount(strNickname);
			if(nWarnedCnt >= m_nAutoKick){

				
				AddLogLine("Bad Language: " + strNickname + "(User has been kicked / redirected)");

				CString strWarning;
				strBotMessage = _T(m_strBadLanRedirect);
				// FIXME: redirect
				
				//strWarning.Format(m_strRCMSNick + " %s " + m_strRCMSMessage + " #Usercmd Redirect %s", m_strBotname, strNickname);
				
				BOT_MESSAGE b;
				b.dwChannelID = dwChannelID;
				b.strName     = m_strBotname;
				b.strResponse = strWarning;
				b.strTrigger  = strWarning;
				m_qMessageQueue.push(b);

				RemoveWarnedUser(strNickname);
			}
			else{

				CString strLog;
				strLog.Format("Bad Language: %s got warning #%02d", strNickname, nWarnedCnt+1);
				AddLogLine(strLog);
				IncreaseWarnedCount(strNickname);
			}
		}
	}

	
	ReplaceVars(strBotMessage, strNickname, dwChannelID);

	if(strBotMessage.GetLength() > 400){

		strBotMessage = strBotMessage.Left(400);
	}

	m_strLast = strBotMessage;
	
	InputMessage(dwChannelID, strBotMessage);
}


BOOL CBot::IsProtectedUser(CString strUser)
{

	for(int i = 0; i < m_aNoPenaltyUsers.GetSize(); i++){

		if(ContainsStringExact(strUser, m_aNoPenaltyUsers.GetAt(i)) >= 0){

			return TRUE;
		}
	}
	return FALSE;
}


CString CBot::GetInetBeats()
{

/*
    hours(in CET)*60*60 + minutes*60 + seconds. 
	Once you've calculated this (let's call the 
	total of the previous calculation calculated_total): 
	calculated_total/86.4
*/
	CString strBeats;
	int nBeats = 0, nUTCh = 0;
	SYSTEMTIME time;
	TIME_ZONE_INFORMATION tzi;
	
	GetLocalTime(&time);
	DWORD wDayLight = GetTimeZoneInformation(&tzi);

	nUTCh = time.wHour + (tzi.Bias/60) + 1;
		
	nBeats = nUTCh*60*60 + time.wMinute*60 + time.wSecond;
	nBeats /= 68.4;
	
	strBeats.Format("@%03d", nBeats);
	return strBeats;
}

CString CBot::GetMyLocalTime()
{

	CString strTime;
	SYSTEMTIME time;
		
	GetLocalTime(&time);
	

	strTime.Format("%02d:%02d.%02d", time.wHour, time.wMinute, time.wSecond);

	return strTime;
}



CString CBot::GetSystemUptime()
{

	CString strUptime;
	int nMS = GetTickCount();
	int nSec = nMS / 1000;
	int nMin = nMS / 60000;
	int nHour = nMS / 3600000;

	strUptime.Format("%02d:%02d:%02d.%03d", nHour, nMin - nHour*60, nSec - nMin*60, nMS-nSec*1000);

	return strUptime;
}

CString CBot::GetBotUptime()
{

	CString strUptime;
	int nMS = GetTickCount() - m_nBotStartTick;
	int nSec = nMS / 1000;
	int nMin = nMS / 60000;
	int nHour = nMS / 3600000;

	strUptime.Format("%02d:%02d:%02d.%03d", nHour, nMin - nHour*60, nSec - nMin*60, nMS-nSec*1000);

	return strUptime;
}

CString CBot::GetWinampSong()
{

	CString strWinamp = "Winamp is not active";
	HWND hwndWinamp = ::FindWindow("Winamp v1.x",NULL);
	if(hwndWinamp != NULL){

		TCHAR *buff = new TCHAR[250];
		::GetWindowText(hwndWinamp, buff, 250);
		strWinamp = buff;
		strWinamp = strWinamp.Mid(strWinamp.Find(" ", 0)+1, strWinamp.Find(" - Winamp") - strWinamp.Find(" ", 0)-1);
		delete buff;
		buff = NULL;
	}

	return strWinamp;
}

BOOL CBot::IsExtendedUser(CString strNickname)
{

	for(int i = 0; i < m_aExtendedUsers.GetSize(); i++){

		if(ContainsStringExact(strNickname, m_aExtendedUsers.GetAt(i)) >= 0){

			return TRUE;
		}
	}

	return FALSE;
}

// GetName returns the Name with the 9 digit userid removed
CString CBot::GetName(CString strRawname)
{
	
	CString strName;
	int nLen = strRawname.GetLength();
	// Bender's.coffee.|_|D..000_50583
	if((nLen-9) <= 2){

		strName = strRawname;
	}
	else{

		if(strRawname[nLen-6] == '_'){

			strName = strRawname.Left(nLen-9);
		}
		else{

			strName = strRawname;
		}
	}
	return strName;
}

CString CBot::GetRawName(DWORD dwID, CString strNickname)
{

	CString strRawname;

	CUserArray* pUserArray = GetRoom(dwID).pUserArray;
	
	if(pUserArray == NULL) ASSERT(FALSE);

	for(int i = 0; i < pUserArray->GetSize(); i++){

		strRawname = pUserArray->GetAt(i).strUser;
		if(strRawname.Find(strNickname, 0) >= 0) break;	
	}
	if(i >= pUserArray->GetSize()){

		strRawname = strNickname;
	}
	return strRawname;
}

void CBot::RemoveWarnedUser(CString strUser)
{

	if(IsProtectedUser(strUser)){

		return;
	}

	for(int i = 0; i < m_aWarnedNicks.GetSize(); i++){

		if(strUser == m_aWarnedNicks.GetAt(i)){

			m_aWarnedNicks.RemoveAt(i, 1);
			return;
		}
	}
}

void CBot::IncreaseWarnedCount(CString strUser)
{

	if(IsProtectedUser(strUser)){

		return;
	}

	for(int i = 0; i < m_aWarnedNicks.GetSize(); i++){

		if(strUser == m_aWarnedNicks.GetAt(i)){

			if(i >= 255){
				return;
			}
			int nValue = m_vWarnedCount[i];
			m_vWarnedCount[i] = ++nValue;
			
			return; 
		}
	}
}

int CBot::GetWarnedCount(CString strUser)
{

	if(IsProtectedUser(strUser)){

		return 0;
	}
	for(int i = 0; i < m_aWarnedNicks.GetSize(); i++){

		if(strUser == m_aWarnedNicks.GetAt(i)){

			if(i >= 255){
				return 0;
			}

			int nCnt = m_vWarnedCount[i];
			
			return nCnt ;
		}
	}

	return 0;
}

BOOL CBot::IsUserWarned(CString strUser)
{

	for(int i = 0; i < m_aWarnedNicks.GetSize(); i++){

		if(strUser == m_aWarnedNicks.GetAt(i)){

			return TRUE;
		}
	}

	return FALSE;
}

void CBot::Bot(DWORD dwID, CString strNickname, CString strMessage)
{

	// Nickname is emtpy, ignore input :-P
	if(strNickname.IsEmpty()){
	
		return;
	}
	// Message is emtpy, ignore input :-P
	if(strMessage.IsEmpty()){
		
		return;
	}
	
	// if this is a message printed by the bot, ignore it still :-P
	if((GetRawName(dwID, strNickname) == m_strBotname) || (m_strBotname.Find(GetName(strNickname), 0) == 0)){
		
		return;
	}

	// If this is a message printed by the bot via #nickname .... ignore it too :-P
	if((m_strLast.Find(strMessage, 0) >= NULL) && (m_strLast.Find(strNickname, 0) >= 0) && (m_strLast.Find("#private", 0) < 0)){
		
		return;
	}	
	
	SetWb(strNickname);

	CString strBadword = strMessage;
	strBadword.MakeLower();

	// Badword
	for(int i = 0; (i < m_aBadWords.GetSize()) && m_bBadWord; i++){

		if(ContainsStringExact(strBadword, m_aBadWords.GetAt(i)) >= 0){
		
			BOT_MESSAGE b;
			b.dwChannelID = dwID;
			b.strName = strNickname;
			b.strTrigger = strMessage;
			b.strResponse = m_strBadLan;
			m_qMessageQueue.push(b);
			break;
		}
	}

	// Normal command
	for(i = 0; (i < m_aNormal.GetSize()) && m_bCostum; i++){

		PCOMMAND pcmd = m_aNormal.GetAt(i);
		
		if(ContainsStringExact(strNickname, pcmd->strName) < 0) continue;

		CString strTmp = pcmd->strTrigger;

		int nPosT = strTmp.Find("$PARAMETER$", 0);
		strTmp.Replace("$parameter$", "*");
		strTmp.Replace("$PARAMETER$", "*");

		if(ContainsStringExact(strMessage, strTmp) >= 0){
			
			if(IsUserBlocked(strNickname)) break;

			strTmp = pcmd->strResponse;
			int nPos = 0;
			if((nPos = strTmp.Find("$PARAMETER$")) >= NULL){

				strTmp.Replace("$PARAMETER$", strMessage.Mid(nPosT >= 0 ? nPosT : 0));
			}

			BOT_MESSAGE b;
			b.dwChannelID = dwID;
			b.strName = strNickname;
			b.strTrigger = strMessage;
			b.strResponse = strTmp;
			m_qMessageQueue.push(b);
			break;
		}
	}

	// Extended command
	if(IsExtendedUser(strNickname)){

		for(i = 0; (i < m_aExtended.GetSize()) && m_bExtended; i++){

			PCOMMAND pcmd = m_aExtended.GetAt(i);
			
			if(ContainsStringExact(strNickname, pcmd->strName) < 0) continue;

			CString strTmp = pcmd->strTrigger;

			int nPosT = strTmp.Find("$PARAMETER$", 0);
			strTmp.Replace("$parameter$", "*");
			strTmp.Replace("$PARAMETER$", "*");

			if(ContainsStringExact(strMessage, strTmp) >= 0){
				
				strTmp = pcmd->strResponse;
				int nPos = 0;
				if((nPos = strTmp.Find("$PARAMETER$")) >= NULL){

					strTmp.Replace("$PARAMETER$", strMessage.Mid(nPosT >= 0 ? nPosT : 0));
				}

				BOT_MESSAGE b;
				b.dwChannelID = dwID;
				b.strName = strNickname;
				b.strTrigger = strMessage;
				b.strResponse = strTmp;
				m_qMessageQueue.push(b);
				break;
			}
		}
	}

	for(i = 0; (i < m_aRandom.GetSize()) && m_bCostum; i++){


		PRANDCMD r = m_aRandom.GetAt(i);
		CString strTmp = r->strTrigger;
		if(ContainsStringExact(strNickname, r->strName) < 0) continue;

		int nPosT = strTmp.Find("$PARAMETER$", 0);
		strTmp.Replace("$parameter$", "*");
		strTmp.Replace("$PARAMETER$", "*");

		if(ContainsStringExact(strMessage, strTmp) >= 0){
			
			int nRes = rand() % r->nNum;
			strTmp = r->strResonse[nRes];
			int nPos = 0;
			if((nPos = strTmp.Find("$PARAMETER$")) >= NULL){

				strTmp.Replace("$PARAMETER$", strMessage.Mid(nPosT >= 0 ? nPosT : 0));
			}

			BOT_MESSAGE b;
			b.dwChannelID = dwID;
			b.strName = strNickname;
			b.strTrigger = strMessage;
			b.strResponse = strTmp;
			m_qMessageQueue.push(b);
			break;
		}

	}
}

void CBot::PrintAutoMessage()
{

	for(int i = 0; i < m_aRooms.GetSize(); i++){

		BOT_MESSAGE b;
		b.dwChannelID = m_aRooms[i].dwID;
		b.strName = m_strBotname;
		b.strResponse = m_strBotMessage;
		b.strTrigger  = "Automessage";

		m_qMessageQueue.push(b);
	}
}


BOOL CBot::ContainsStringExact(CString strLine, CString strSubString, BOOL bCase)
{
	
    if(strLine.IsEmpty()) return FALSE;
	if(strSubString.IsEmpty()) return TRUE;
	int nReturn = -1;
	BOOL bFound = FALSE;
	if(!bCase){

		strLine.MakeLower();
		strSubString.MakeLower();
	}

	CString strSearch = strSubString;
	strSearch.Remove('*');
	
	int nIndex = -1;
	BOOL bFront, bEnd;

	if((nIndex = strLine.Find(strSearch, 0)) >= 0){

		//Check if there is a space or ( at front and end of searched item
		bEnd = (nIndex + strSearch.GetLength() >= strLine.GetLength() ? TRUE : strLine.GetAt(strSearch.GetLength() + nIndex) == ' ');
		bFront = (nIndex > 0 ? strLine.GetAt(nIndex-1) == ' ' : TRUE);

		if((strSubString.GetAt(0) == '*') && (strSubString.GetAt(strSubString.GetLength()-1) == '*')){

			bFound = TRUE;
		}
		else if((strSubString.GetAt(0) == '*') && (strSubString.GetAt(strSubString.GetLength()-1) != '*')){

			bFound = bEnd;
		}
		else if((strSubString.GetAt(0) != '*') && (strSubString.GetAt(strSubString.GetLength()-1) == '*')){

			bFound = bFront;
		}
		else{

			bFound = bFront && bEnd;
		}
		if(bFound){
			
			nReturn = nIndex;
		}
	}

	return nReturn;
}


BOOL CBot::IsUserBlocked(CString strNickname)
{
	BOOL bReturn = FALSE;
	BOOL bFound = FALSE;
	if(strNickname == m_strBotname) return TRUE;

	for(int i = 0; i < m_vUserInfo.GetSize(); i++){

		if(m_vUserInfo.GetAt(i).strName == strNickname){

			bFound = TRUE;
			break;
		} // end ... == nickname
	}// end for


	// User has not been posting yet
	if(!bFound){

		USER_INFO u;
		u.nCount      = 1;
		u.nFirst      = GetTickCount();
		u.strName     = strNickname;
		u.nBlockStart = -1;
		m_vUserInfo.Add(u);
	}
	else{

		// User is already blocked
		if(m_vUserInfo.GetAt(i).nBlockStart != -1){

			if((GetTickCount() - m_vUserInfo.GetAt(i).nBlockStart) <= (DWORD)m_nBlockTime){
				
				// Blocked and still in blocktime
				TRACE(strNickname + " is blocked\n");
				bReturn = TRUE;
			}
			else{

				// Blocked but exceeded blocktime so release him
				TRACE(strNickname + " is now unblocked\n");
				USER_INFO u = m_vUserInfo.GetAt(i);
				u.nCount = 1;
				u.nFirst = GetTickCount();
				u.nBlockStart = -1;
				m_vUserInfo.SetAt(i, u);
				bReturn = FALSE;
			}
		}
		// User is not yet blocked
		else{
			
			if((GetTickCount() - m_vUserInfo.GetAt(i).nFirst) <= m_nBlockRange){

				// User has not exceeded blockrange so push him one up
				TRACE(strNickname + " pushed a notch\n");
				USER_INFO u = m_vUserInfo.GetAt(i);
				u.nCount++;
				m_vUserInfo.SetAt(i, u);
				bReturn = FALSE;
			}
			else{

				// enough time between posts
				TRACE(strNickname + " was clever enough\n");
				USER_INFO u = m_vUserInfo.GetAt(i);
				u.nBlockStart = -1;
				u.nCount = 1;
				u.nFirst = GetTickCount();
				m_vUserInfo.SetAt(i, u);
				bReturn = FALSE;
			}
			if(m_vUserInfo.GetAt(i).nCount >= m_nBlockMessageCount){

				TRACE(strNickname + " now blocked\n");
				USER_INFO u = m_vUserInfo.GetAt(i);
				u.nBlockStart = GetTickCount();
				m_vUserInfo.SetAt(i, u);
				bReturn = TRUE;
			}
		}
	}

	return bReturn;
}

CString CBot::GetTriggerName(CString strText)
{

	CString strName;
	int nStart = 0, nEnd = 0;
	nStart = strText.Find("{", 0);
	if(nStart >= NULL){

		nEnd = strText.Find("}", nStart+1);
		if(nEnd > NULL){

			strName = strText.Mid(nStart+1, nEnd-nStart-1);
		}
	}
	if(strName.IsEmpty()) strName = "*";

	return strName;
}

CString CBot::GetTrigger(CString strText)
{

	CString strTrigger;
	int nStart = 0;

	nStart = strText.ReverseFind('}');
	if(nStart >= NULL){

		strTrigger = strText.Mid(nStart+1);
	}
	else{

		strTrigger = strText;
	}

	return strTrigger;
}

void CBot::ClearRandom()
{

	int j = 0;
	while((j = (int)m_aRandom.GetSize()) > 0){

		PRANDCMD r = m_aRandom.GetAt(j-1);
		m_aRandom.RemoveAt(j-1);
		r->strResonse.RemoveAll();
		r->nNum = 0;
		r->strName.Empty();
		r->strTrigger.Empty();
		delete r;
		r = NULL;
	}

	while((j = (int)m_aNormal.GetSize()) > 0){

		PCOMMAND c = m_aNormal.GetAt(j-1);
		m_aNormal.RemoveAt(j-1);
		c->strName.Empty();
		c->strResponse.Empty();
		c->strTrigger.Empty();
		delete c;
		c = NULL;
	}

	while((j = (int)m_aExtended.GetSize()) > 0){

		PCOMMAND c = m_aExtended.GetAt(j-1);
		m_aExtended.RemoveAt(j-1);
		c->strName.Empty();
		c->strResponse.Empty();
		c->strTrigger.Empty();
		delete c;
		c = NULL;
	}

	m_aGame.SetSize(0);
	m_aExtendedUsers.SetSize(0);
}

CString CBot::GetNick(CString strName)
{

	int nEnd = 0;

	for(int i = 0; i < m_strSeperator.GetLength(); i++){

		nEnd = strName.Find(m_strSeperator[i]);
		if(nEnd > NULL){

			return strName.Left(nEnd);
		}
	}
	return strName;
}

BOOL CBot::IsAdmin(CString& strNickname)
{

	BOOL bAdmin = FALSE;
	for(int i = 0; i < m_aExtendedUsers.GetSize(); i++){

		if(strNickname.Find(m_aExtendedUsers[i], 0) == 0){

			bAdmin = TRUE;
			break;
		}
	}
	return bAdmin;
}

BOOL CBot::WasHere(CString &strUser)
{
	
	BOOL bWasHere = FALSE;

	for(int i = 0; i < m_aUserInfo.GetSize(); i++){

		if(strUser.Find(m_aUserInfo[i].strName, 0) == 0){

			bWasHere = m_aUserInfo[i].bWasHere;
			break;
		}
	}

	return bWasHere;

}

void CBot::AddUser(CString strUser)
{

	for(int i = 0; i < m_aUserInfo.GetSize(); i++){

		if(strUser.Find(m_aUserInfo[i].strName, 0) == 0){

			return; // is already added
		}
	}
	
	USER_INFO u;
	u.strName = strUser;
	u.bWasHere = FALSE; // was only here if he talked ;-)
	u.nTriviaScore = 0;

	m_aUserInfo.Add(u);
}

void CBot::RemoveUser(CString strUser)
{

	for(int i = 0; i < m_aUserInfo.GetSize(); i++){

		if(strUser.Find(m_aUserInfo[i].strName, 0) == 0){

			if(!m_aUserInfo[i].bWasHere){

				//remove only if he didnt talk
				m_aUserInfo.RemoveAt(i);
			}
			return; // is already added
		}
	}
}

void CBot::SetWb(CString strUser)
{

	for(int i = 0; i < m_aUserInfo.GetSize(); i++){

		if(m_aUserInfo[i].strName.Find(strUser, 0) == 0){

			m_aUserInfo[i].bWasHere = TRUE;
			return; // was found and set to be welcomed back
		}
	}

	// not in array but he said something ;)
	USER_INFO u;
	u.strName = strUser;
	u.bWasHere = TRUE;
	u.nTriviaScore = 0;
	
	m_aUserInfo.Add(u);
}

// Init trivia questions...
void CBot::InitTrivia()
{

   	CArray<PCOMMAND, PCOMMAND> aTmp;

	for(int i = 0; i < m_aTrivia.GetSize(); i++){

		aTmp.Add(m_aTrivia.GetAt(i));
	}

	int nIndex = 0;
	//randomize order...
	while(aTmp.GetSize()){

		nIndex = rand() % aTmp.GetSize();
		m_aGame.Add(aTmp.GetAt(nIndex));
		aTmp.RemoveAt(nIndex);
	}
	m_nCurrent = 0;
}

int CBot::GetScore(CString &strUser)
{

	for(int i = 0; i < m_aUserInfo.GetSize(); i++){

		if(m_aUserInfo[i].strName.Find(strUser, 0) == 0){

			return m_aUserInfo[i].nTriviaScore;
		}
	}
	return 0;
}

int CBot::IncreaseScore(CString &strUser)
{

	for(int i = 0; i < m_aUserInfo.GetSize(); i++){

		if(m_aUserInfo[i].strName.Find(strUser, 0) == 0){

			m_aUserInfo[i].nTriviaScore++;
			return m_aUserInfo[i].nTriviaScore;
		}
	}

	USER_INFO u;
	u.bWasHere = TRUE;
	u.nTriviaScore = 1;
	u.strName = strUser;
	m_aUserInfo.Add(u);
	
	return u.nTriviaScore;
}

// Thread for trivia game :-)
UINT CBot::TriviaThread(PVOID pParam)
{

	CBot *pPlugin = (CBot*)pParam;
	
	int nStartTic = 0;

	while(pPlugin->m_bTrivia){

		// No current question so pop ask the next one :-)
		if(pPlugin->m_pCurrent == NULL){

			// take care it does not loop out of scope!
			if(pPlugin->m_nCurrent >= pPlugin->m_aGame.GetSize()){

				pPlugin->m_nCurrent = 0;
			}
			// Get next question
			pPlugin->m_pCurrent = pPlugin->m_aGame.GetAt(pPlugin->m_nCurrent++);

			BOT_MESSAGE b;
			b.dwChannelID = pPlugin->m_dwTriviaID;
			b.strTrigger = "trivia";
			b.strName = pPlugin->m_strBotname;
			b.strResponse = pPlugin->m_strTNext;
			b.strResponse.Replace("%QUES%", pPlugin->m_pCurrent->strTrigger);
			pPlugin->m_qMessageQueue.push(b);

			nStartTic = GetTickCount();
		}
		else if((GetTickCount() - nStartTic) > 120000){


			//ran out of time...
			BOT_MESSAGE b;
			b.strResponse = pPlugin->m_strTNoAnswer;
			b.strResponse.Replace("%ANS%", pPlugin->m_pCurrent->strResponse);
			b.strResponse.Replace("%QUES%", pPlugin->m_pCurrent->strTrigger);
			
			pPlugin->m_pCurrent = NULL;

			b.dwChannelID = pPlugin->m_dwTriviaID;
			b.strTrigger = "trivia";
			b.strName = pPlugin->m_strBotname;

			pPlugin->m_qMessageQueue.push(b);

		}
		Sleep(100);
	}
	pPlugin->m_eTrivia.SetEvent();
	return 0;
}

// Check for trivia answers
void CBot::Trivia(DWORD dwID, CString strNickname, CString strMessage)
{

	// Nickname is emtpy, ignore input :-P
	if(strNickname.IsEmpty()){
	
		return;
	}
	// Message is emtpy, ignore input :-P
	if(strMessage.IsEmpty()){
		
		return;
	}
	
	// if this is a message printed by the bot, ignore it still :-P
	if((GetRawName(dwID, strNickname) == m_strBotname) || (m_strBotname.Find(GetName(strNickname), 0) == 0)){
		
		return;
	}

	// If this is a message printed by the bot via #nickname .... ignore it too :-P
	if((m_strLast.Find(strMessage, 0) >= NULL) && (m_strLast.Find(strNickname, 0) >= 0) && (m_strLast.Find("#private", 0) < 0)){
		
		return;
	}	

	CString strTmp;

	if(strMessage == "SCORE"){

		strTmp.Format("%d", GetScore(strNickname));

		BOT_MESSAGE b;
		b.dwChannelID = dwID;
		b.strName = strNickname;
		b.strTrigger = "SCORE";
		b.strResponse = m_strTScore;
		b.strResponse.Replace("%SCORE%", strTmp);
		m_qMessageQueue.push(b);
		
	}
	
	if(strMessage == "RANKING"){

		if(!m_aUserInfo.GetSize()) return;

		USER_INFO u1, u2, u3;

		u1.nTriviaScore = -1;
		u1.strName = "";
		for(int i = 0; i < m_aUserInfo.GetSize(); i++){

			if(m_aUserInfo[i].nTriviaScore > u1.nTriviaScore){

				TRACE("1> %s %d\n", m_aUserInfo[i].strName, m_aUserInfo[i].nTriviaScore);
				u1 = m_aUserInfo[i];
			}
		}

		if(m_aUserInfo.GetSize() > 1){

			u2.nTriviaScore = -1;
			u2.strName = "";
			for(i = 0; i < m_aUserInfo.GetSize(); i++){

				if((m_aUserInfo[i].nTriviaScore > u2.nTriviaScore) &&
					(m_aUserInfo[i].nTriviaScore <= u1.nTriviaScore) &&
					(m_aUserInfo[i].strName != u1.strName)){

					TRACE("2> %s %d\n", m_aUserInfo[i].strName, m_aUserInfo[i].nTriviaScore);
					u2 = m_aUserInfo[i];
				}
			}
		}
		if(m_aUserInfo.GetSize() > 2){


			u3.nTriviaScore = -1;
			u3.strName = "";
			for(i = 0; i < m_aUserInfo.GetSize(); i++){

				if((m_aUserInfo[i].nTriviaScore > u3.nTriviaScore) &&
					(m_aUserInfo[i].nTriviaScore <= u2.nTriviaScore) &&
					(m_aUserInfo[i].strName != u2.strName) &&
					(m_aUserInfo[i].strName != u1.strName)){

					TRACE("3> %s %d\n", m_aUserInfo[i].strName, m_aUserInfo[i].nTriviaScore);
					u3 = m_aUserInfo[i];
				}
			}
		}

		BOT_MESSAGE b;
		b.strName = strNickname;
		b.dwChannelID = dwID;
		b.strTrigger = "RANKING";

		b.strResponse = "Trivia UserRanking:";
		m_qMessageQueue.push(b);

		b.strResponse.Format("  Place 1: %s (%02d points)", u1.strName, u1.nTriviaScore);
		m_qMessageQueue.push(b);

		if(m_aUserInfo.GetSize() > 1){

			b.strResponse.Format("  Place 2: %s (%02d points)", u2.strName, u2.nTriviaScore);
			m_qMessageQueue.push(b);
		}

		if(m_aUserInfo.GetSize() > 2){

			b.strResponse.Format("  Place 3: %s (%02d points)", u3.strName, u3.nTriviaScore);
			m_qMessageQueue.push(b);
		}
	}

	if((strMessage == "SKIP") && IsAdmin(strNickname)){


		BOT_MESSAGE b;
		b.strResponse = m_strTSkip;
		b.strResponse.Replace("%QUES%", m_pCurrent->strTrigger);
		b.strResponse.Replace("%ANS$%", m_pCurrent->strResponse);
		b.dwChannelID = dwID;
		b.strName = strNickname;
		b.strTrigger = "SCORE";
		m_qMessageQueue.push(b);

		m_pCurrent = NULL;
	}

	if(m_pCurrent == NULL){


		return;
	}

	if(ContainsStringExact(strMessage, m_pCurrent->strResponse, FALSE) >= 0){

		
		BOT_MESSAGE b;
		b.dwChannelID = dwID;
		b.strName = strNickname;
		b.strTrigger = "SCORE";
		b.strResponse = m_strTCorrect;
		strTmp.Format("%d", IncreaseScore(strNickname));
		b.strResponse.Replace("%SCORE%", strTmp);
		b.strResponse.Replace("%ANS%", m_pCurrent->strResponse);
		b.strResponse.Replace("%QUES%", m_pCurrent->strTrigger);
		m_qMessageQueue.push(b);
		m_pCurrent = NULL;
	}
}

void CBot::ClearGame()
{

	m_aGame.SetSize(0);
	for(int i = 0; i <  m_aUserInfo.GetSize(); i++){

		m_aUserInfo[i].nTriviaScore = 0;
	}
	m_bTrivia = FALSE;
	m_pCurrent = NULL;
	m_nCurrent = 0;

}
// Get Connectión type of user in channel ID
CString CBot::GetLineType(DWORD dwID, CString strName)
{

	CString strRawname, strLine = "Unknown";

	CUserArray* pUserArray = GetRoom(dwID).pUserArray;
	
	if(pUserArray == NULL) ASSERT(FALSE);

	for(int i = 0; i < pUserArray->GetSize(); i++){

		strRawname = pUserArray->GetAt(i).strUser;
		strLine = line_types[pUserArray->GetAt(i).wLineType];
		if(strRawname.Find(strName, 0) >= 0) break;	
	}
	
	return strLine;
}

// Get room name assigned to ID
CString CBot::GetRoomName(DWORD dwID)
{

	for(int i = 0; i < m_aRooms.GetSize(); i++){

		if(m_aRooms[i].dwID == dwID) return m_aRooms[i].strRoomName;
	}
	return "";
}

// Get Number of files shared of user
CString CBot::GetFiles(DWORD dwID, CString strName)
{

	CString strRawname, strFiles = "0";

	CUserArray* pUserArray = GetRoom(dwID).pUserArray;
	
	if(pUserArray == NULL) ASSERT(FALSE);

	for(int i = 0; i < pUserArray->GetSize(); i++){

		strRawname = pUserArray->GetAt(i).strUser;
		strFiles.Format("%d", pUserArray->GetAt(i).dwNumFiles);
		if(strRawname.Find(strName, 0) >= 0) break;	
	}

	return strFiles;
}

CString CBot::GetIP(DWORD dwID, CString strName)
{

	CString strIP;

	CUserArray* pUserArray = GetRoom(dwID).pUserArray;
	
	if(pUserArray == NULL) ASSERT(FALSE);

	for(int i = 0; i < pUserArray->GetSize(); i++){

		if(pUserArray->GetAt(i).strUser.Find(strName, 0) == 0){
			
			strIP = pUserArray->GetAt(i).strRealIP;
			break;			
		}
	}
	if(i >= pUserArray->GetSize()){

		strIP = "0.0.0.0";
	}
	return strIP;
}
    
CString CBot::GetHostname(DWORD dwID, CString strName)
{

	CString strHost;

	CUserArray* pUserArray = GetRoom(dwID).pUserArray;
	
	if(pUserArray == NULL) ASSERT(FALSE);

	for(int i = 0; i < pUserArray->GetSize(); i++){

		if(pUserArray->GetAt(i).strUser.Find(strName, 0) == 0){
			
			strHost = pUserArray->GetAt(i).strHostname;
			break;			
		}
	}
	if(i >= pUserArray->GetSize()){

		strHost = "unknown host";
	}
	return strHost;
}

// Replace all variables
void CBot::ReplaceVars(CString &strString, CString strName, DWORD dwID)
{

	if(strString.Find("%",0) < 0) return;

	CString strNum;
	

	strNum.Format("%d",  GetRoom(dwID).pUserArray->GetSize());
	strString.Replace(_T("%NUMUSERS%"), strNum);

	// Winamp stuff:
	if(strString.Find("%WA-", 0) >= 0){

		CString strArtist, strSong, strIsVideo, strVersion, strPlayTime, strTotalTime, strRemTime, strSampleRate, strBitrate, strNumChannels, strStatus = "not running";


		strArtist = GetWinampSong();
		
		if(strArtist != "Winamp is not active"){

			int nMid = strArtist.Find(" - ", 0);
			if(nMid > 0){

				strSong = strArtist.Mid(nMid+3);
				strArtist = strArtist.Left(nMid);
			}
			else{

				//strArtist = "error";
				strSong = "";
			}
		}
		else{

			strArtist = "Winamp";
			strSong  = "not running";		
		}

		HWND hwndWinamp = ::FindWindow("Winamp v1.x",NULL);

		if(hwndWinamp != NULL){


			strVersion.Format("%x", ::SendMessage(hwndWinamp, WM_WA_IPC, 0, IPC_GETVERSION));
			strVersion.SetAt(1, '.');

			int nTotal = 0, nRem = 0, nEla = 0;
			nTotal = ::SendMessage(hwndWinamp, WM_WA_IPC, 1, IPC_GETOUTPUTTIME);
			strTotalTime.Format("%02d:%02d", nTotal/60, nTotal%60);

			nEla= ::SendMessage(hwndWinamp, WM_WA_IPC, 0, IPC_GETOUTPUTTIME) / 1000;
			strPlayTime.Format("%02d:%02d", nEla/60, nEla%60);
			
			nRem = nTotal - nEla;
			strRemTime.Format("%02d:%02d", nRem/60, nRem%60);

			strSampleRate.Format("%d", ::SendMessage(hwndWinamp, WM_WA_IPC, 0, IPC_GETINFO));
			strBitrate.Format("%d", ::SendMessage(hwndWinamp, WM_WA_IPC, 1, IPC_GETINFO));
			strNumChannels.Format("%d", ::SendMessage(hwndWinamp, WM_WA_IPC, 2, IPC_GETINFO));

			switch(::SendMessage(hwndWinamp, WM_WA_IPC, 0, IPC_ISPLAYING)){

			case 1: strStatus = "playing";
				break;
			case 3: strStatus = "paused";
				break;
			default: strStatus = "stopped";
			}

			switch(SendMessage(hwndWinamp, WM_WA_IPC, 0, IPC_IS_PLAYING_VIDEO)){

			case 0: strIsVideo.Format("%d", 0);
				break;
			case 1: strIsVideo.Format("%d", 1);
				break;
			default : strIsVideo.Format("%d", 2);
			}

		}
		strString.Replace(_T("%WA-ARTIST%"), strArtist);
		strString.Replace(_T("%WA-SONG%"), strSong);
		strString.Replace(_T("%WA-VERSION%"), strVersion);
		strString.Replace(_T("%WA-ELATIME%"), strPlayTime);
		strString.Replace(_T("%WA-REMTIME%"), strRemTime);
		strString.Replace(_T("%WA-TOTALTIME%"), strTotalTime);
		strString.Replace(_T("%WA-SAMPLERATE%"), strSampleRate);
		strString.Replace(_T("%WA-BITRATE%"), strBitrate);
		strString.Replace(_T("%WA-CHANNELS%"), strNumChannels);
		strString.Replace(_T("%WA-STATUS%"), strStatus);
		strString.Replace(_T("%WA-ISVIDEO%"), strIsVideo);
	}

	strString.Replace(_T("%FILES%"), GetFiles(dwID, strName));
	strString.Replace(_T("%LINE%"), GetLineType(dwID, strName));
	strString.Replace(_T("%VERSION%"), STR_VERSION);
	strString.Replace(_T("%TIME%"), GetMyLocalTime());
	strString.Replace(_T("%INETBEATS%"), GetInetBeats());
	strString.Replace(_T("%UPTIME%"), GetSystemUptime());
	strString.Replace(_T("%BOTUPTIME%"), GetBotUptime());
	strString.Replace(_T("%BOTNAME%"), m_strBotname);
	strString.Replace(_T("%BOTRAWNAME%"), GetRawName(dwID, m_strBotname));
	strString.Replace(_T("%ROOMNAME%"), GetRoomName(dwID));
	strString.Replace("%IP%", GetIP(dwID, strName));
	strString.Replace("%HOSTNAME%", GetHostname(dwID, strName));
	
	strString.Replace(_T("%NAME%"), GetName(strName));
	
	strString.Replace(_T("%RAWNAME%"), GetRawName(dwID, strName));
	strString.Replace(_T("%NICK%"), GetNick(strName));

}