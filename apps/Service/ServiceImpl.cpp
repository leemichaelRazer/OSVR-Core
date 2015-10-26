
#include "stdafx.h"
#include "ServiceImpl.h"

VOID ServiceReportEvent(WORD EventType, LPTSTR szMessage);

CServiceImpl::CServiceImpl()
{
	configName = osvr::server::getDefaultConfigFilename();
	{
		CString csMessage;
		csMessage.Format(_T("CServiceImpl::CServiceImpl"));
		ServiceReportEvent(EVENTLOG_INFORMATION_TYPE, csMessage.GetBuffer());
	}
}

CServiceImpl::~CServiceImpl()
{
}

BOOL CServiceImpl::Init()
{
	configName = osvr::server::getDefaultConfigFilename();
	{
		CString csMessage;
		csMessage.Format(_T("CServiceImpl::Init"));
		ServiceReportEvent(EVENTLOG_INFORMATION_TYPE, csMessage.GetBuffer());
	}


    return TRUE;
}

BOOL CServiceImpl::UnInit()
{
    return TRUE;
}

void handleShutdown() {
	//out << "Received shutdown signal..." << endl;
	//server->signalStop();
}

BOOL CServiceImpl::Start()
{
	o_filestr.open("OSVR/o_osvr.log");
	o_backup = std::cout.rdbuf();
	o_psbuf = o_filestr.rdbuf();
	std::cout.rdbuf(o_psbuf);

	e_filestr.open("OSVR/e_osvr.log");
	e_backup = std::cerr.rdbuf();
	e_psbuf = e_filestr.rdbuf();
	std::cerr.rdbuf(e_psbuf);


	//char path[512];
	//DWORD path_length = 512;

	char *path = "c:/Windows/System32/OSVR/osvr_server_config.json";

	//HKEY key;

	//int theError = RegOpenKey(HKEY_CURRENT_USER, TEXT("Software\\OSVR\\Server"), &key);
	//if ( theError != ERROR_SUCCESS)
	//{
	//	CString csMessage;
	//	csMessage.Format(_T("Cannot retreive registry key for config file location: %x", theError));
	//	ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());
	//	RegCloseKey(key);
	//	return false;
	//}

	//RegQueryValueEx(key, TEXT("server_config"), NULL, (LPDWORD)REG_SZ, (LPBYTE)&path, &path_length);
	//{
	//	CString csMessage;
	//	csMessage.Format(_T("Config file location: %s", value));
	//	ServiceReportEvent(EVENTLOG_INFORMATION_TYPE, csMessage.GetBuffer());
	//}

	//RegCloseKey(key);


	server = osvr::server::configureServerFromFile("C:/ProgramData/OSVR/osvr_server_config.json");
	//server = osvr::server::configureServerFromFile("C:/Users/OSVR/Documents/GitHub/OSVR-Core/Build/bin/Release/osvr_server_config.json");

	if (!server)
	{
		CString csMessage;
		csMessage.Format(_T("CServiceImpl::Start - Server did not instantiate or start"));
		ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());
		return FALSE;
	}

	osvr::server::registerShutdownHandler<&handleShutdown>(); 
	
	//server->startAndAwaitShutdown();
	server->start();

	return TRUE;
}

BOOL CServiceImpl::Stop()
{
	server->stop();
	server->~Server();

	std::cout.rdbuf(o_backup);
	o_filestr.close();
	std::cerr.rdbuf(e_backup);
	e_filestr.close();

	return TRUE;
}

BOOL CServiceImpl::Pause()
{
    return TRUE;
}

BOOL CServiceImpl::Continue()
{
    return TRUE;
}

void CServiceImpl::OnDeviceEvent(DWORD dwEvent, LPVOID lpData)
{
}
