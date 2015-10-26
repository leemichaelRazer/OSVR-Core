// Service.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Strsafe.h>
#include <Dbt.h>
#include "ServiceErrors.h"
#include "ServiceImpl.h"

//#define SVC_NAME            L"My Service"

#define SVC_DESCRIPTION     _T("OSVR Services")


// https://msdn.microsoft.com/en-us/library/windows/hardware/ff545860(v=vs.85).aspx
// {4D1E55B2-F16F-11CF-88CB-001111000030}
static const GUID GUID_DEVINTERFACE_HID = 
{ 0x4D1E55B2, 0xF16F, 0x11CF, { 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } };

CServiceImpl            g_Service;

SERVICE_STATUS          gServiceStatus; 
SERVICE_STATUS_HANDLE   gServiceStatusHandle; 
HANDLE                  ghServiceStopEvent = NULL;
HDEVNOTIFY              ghDevNotify = NULL;

VOID InstallService(void);
VOID UninstallService(void);
VOID StartService(void);
VOID StopService(void);
DWORD WINAPI ServiceControlHandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);
VOID WINAPI ServiceMain( DWORD dwArgc, LPTSTR *lpszArgv ); 

VOID ReportServiceStatus( DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint );
VOID ServiceInit( DWORD dwArgc, LPTSTR *lpszArgv ); 
VOID ServiceUnInit(void);
VOID ServiceStart(void);
VOID ServiceStop(void);
VOID ServicePause(void);
VOID ServiceContinue(void);
VOID ServiceReportEvent( LPTSTR szFunction );
VOID ServiceReportEvent(WORD EventType, LPTSTR szMessage);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(nCmdShow);

    CString csCmdLine(lpCmdLine);
    if(csCmdLine == _T("-install"))
    {
        InstallService();
		std::cout << "hey";
        return 0;
    }
    else if(csCmdLine == _T("-uninstall"))
    {
        UninstallService();
        return 0;
    }
    else if(csCmdLine == _T("-start"))
    {
        StartService();
        return 0;
    }
    else if(csCmdLine == _T("-stop"))
    {
        StopService();
        return 0;
    }

		{
			CString csMessage;
			csMessage.Format(_T("OSVRService: Post Command line procesing"));
			ServiceReportEvent(EVENTLOG_INFORMATION_TYPE, csMessage.GetBuffer());
		}

    // TO_DO: Add any additional services for the process to this table.
    SERVICE_TABLE_ENTRY DispatchTable[] = 
    { 
        { SVC_NAME, (LPSERVICE_MAIN_FUNCTION) ServiceMain }, 
        { NULL, NULL } 
    }; 
 
    // This call returns when the service has stopped. 
    // The process should simply terminate when the call returns.

    if (!StartServiceCtrlDispatcher( DispatchTable )) 
    { 
        CString csMessage;
        csMessage.Format(_T("StartServiceCtrlDispatcher failed with error %d"), GetLastError());
        ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());
    }

    return 0;
}

VOID InstallService(void)
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    TCHAR szPath[MAX_PATH];

    if( !GetModuleFileName(NULL, szPath, MAX_PATH ) )
    {
        CString csMessage;
        csMessage.Format(_T("GetModuleFileName failed with error %d"), GetLastError());
        ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());
        return;
    }

    // Get a handle to the SCM database. 
 
    schSCManager = OpenSCManager( 
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 
 
    if (NULL == schSCManager) 
    {
        CString csMessage;
        csMessage.Format(_T("OpenSCManager failed with error %d"), GetLastError());
        ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());
        return;
    }

    // Create the service

    schService = CreateService( 
        schSCManager,              // SCM database 
        SVC_NAME,                  // name of service 
        SVC_DISPLAY_NAME,          // service name to display 
        SERVICE_ALL_ACCESS,        // desired access 
        SERVICE_WIN32_OWN_PROCESS, // service type 
        SERVICE_AUTO_START,        // start type 
        SERVICE_ERROR_NORMAL,      // error control type 
        szPath,                    // path to service's binary 
        NULL,                      // no load ordering group 
        NULL,                      // no tag identifier 
        NULL,                      // no dependencies 
        NULL,                      // LocalSystem account 
        NULL);                     // no password 
 
    if (schService == NULL) 
    {
        CString csMessage;
        csMessage.Format(_T("OpenSCManager failed with error %d"), GetLastError());
        ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());
        CloseServiceHandle(schSCManager);
        return;
    }

	SERVICE_FAILURE_ACTIONS sfa;
	SC_ACTION actions;

	sfa.dwResetPeriod = INFINITE;
	sfa.lpCommand = NULL;
	sfa.lpRebootMsg = NULL;
	sfa.cActions = 1;
	sfa.lpsaActions = &actions;

	sfa.lpsaActions[0].Type = SC_ACTION_RESTART;
	sfa.lpsaActions[0].Delay = 0;

	if (ChangeServiceConfig2(schService, SERVICE_CONFIG_FAILURE_ACTIONS, &sfa) == FALSE)
	{
		CString csMessage;
		csMessage.Format(_T("ChangeServiceConfig2: set recovery failed with error %d"), GetLastError());
		ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return;
	}

		
	SERVICE_DESCRIPTION Description = { SVC_DESCRIPTION };
	if (ChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, &Description)==FALSE)
    {
        CString csMessage;
        csMessage.Format(_T("ChangeServiceConfig2 failed with error %d %s"), GetLastError(),Description);
        ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());
    }

    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);
}

VOID UninstallService(void)
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    // Get a handle to the SCM database. 
 
    schSCManager = OpenSCManager( 
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 
 
    if (NULL == schSCManager) 
    {
        CString csMessage;
        csMessage.Format(_T("OpenSCManager failed with error %d"), GetLastError());
        ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());
        return;
    }

    // Get a handle to the service.

    schService = OpenService( 
        schSCManager,       // SCM database 
        SVC_NAME,          // name of service 
        DELETE);            // need delete access 
 
    if (schService == NULL)
    { 
        CString csMessage;
        csMessage.Format(_T("OpenService failed with error %d"), GetLastError());
        ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());
        CloseServiceHandle(schSCManager);
        return;
    }

    // Delete the service.
 
    if (! DeleteService(schService) ) 
    {
        CString csMessage;
        csMessage.Format(_T("DeleteService failed with error %d"), GetLastError());
        ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());
    }
 
    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);
}

VOID StartService(void)
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    SERVICE_STATUS_PROCESS ssStatus; 
    DWORD dwOldCheckPoint; 
    DWORD dwStartTickCount;
    DWORD dwWaitTime;
    DWORD dwBytesNeeded;

	{
		CString csMessage;
		csMessage.Format(_T("OSVRService: StartService"));
		ServiceReportEvent(EVENTLOG_INFORMATION_TYPE, csMessage.GetBuffer());
	}


    // Get a handle to the SCM database. 
    schSCManager = OpenSCManager( 
        NULL,                    // local computer
        NULL,                    // servicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 
 
    if (NULL == schSCManager) 
    {
        CString csMessage;
        csMessage.Format(_T("OpenSCManager failed with error %d"), GetLastError());
        ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());
        return;
    }

    // Get a handle to the service.
    schService = OpenService( 
        schSCManager,           // SCM database 
        SVC_NAME,               // name of service 
        SERVICE_ALL_ACCESS);    // full access 
 
    if (schService == NULL)
    { 
        CString csMessage;
        csMessage.Format(_T("OpenService failed with error %d"), GetLastError());
        ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());
        CloseServiceHandle(schSCManager);
        return;
    }    

    // Check the status in case the service is not stopped. 

    if (!QueryServiceStatusEx( 
            schService,                     // handle to service 
            SC_STATUS_PROCESS_INFO,         // information level
            (LPBYTE) &ssStatus,             // address of structure
            sizeof(SERVICE_STATUS_PROCESS), // size of structure
            &dwBytesNeeded ) )              // size needed if buffer is too small
    {
        CString csMessage;
        csMessage.Format(_T("QueryServiceStatusEx failed with error %d"), GetLastError());
        ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
        return; 
    }

    // Check if the service is already running. It would be possible 
    // to stop the service here, but for simplicity this example just returns. 

    if(ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING)
    {
        CString csMessage(_T("Service already running"));
        ServiceReportEvent(EVENTLOG_INFORMATION_TYPE, csMessage.GetBuffer());
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
        return; 
    }

    // Save the tick count and initial checkpoint.

    dwStartTickCount = GetTickCount();
    dwOldCheckPoint = ssStatus.dwCheckPoint;

    // Wait for the service to stop before attempting to start it.

    while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
    {
        // Do not wait longer than the wait hint. A good interval is 
        // one-tenth of the wait hint but not less than 1 second  
        // and not more than 10 seconds. 
 
        dwWaitTime = ssStatus.dwWaitHint / 10;

        if( dwWaitTime < 1000 )
            dwWaitTime = 1000;
        else if ( dwWaitTime > 10000 )
            dwWaitTime = 10000;

        Sleep( dwWaitTime );

        // Check the status until the service is no longer stop pending. 
 
        if (!QueryServiceStatusEx( 
                schService,                     // handle to service 
                SC_STATUS_PROCESS_INFO,         // information level
                (LPBYTE) &ssStatus,             // address of structure
                sizeof(SERVICE_STATUS_PROCESS), // size of structure
                &dwBytesNeeded ) )              // size needed if buffer is too small
        {
            CString csMessage;
            csMessage.Format(_T("QueryServiceStatusEx failed with error %d"), GetLastError());
            ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());
            CloseServiceHandle(schService); 
            CloseServiceHandle(schSCManager);
            return; 
        }

        if ( ssStatus.dwCheckPoint > dwOldCheckPoint )
        {
            // Continue to wait and check.

            dwStartTickCount = GetTickCount();
            dwOldCheckPoint = ssStatus.dwCheckPoint;
        }
        else
        {
            if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint)
            {
                CloseServiceHandle(schService); 
                CloseServiceHandle(schSCManager);
                return; 
            }
        }
    }

    // Attempt to start the service.

    if (!StartService(
            schService,  // handle to service 
            0,           // number of arguments 
            NULL) )      // no arguments 
    {
        CString csMessage;
        csMessage.Format(_T("StartService failed with error %d"), GetLastError());
        ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
        return; 
    }

    // Check the status until the service is no longer start pending. 
 
    if (!QueryServiceStatusEx( 
            schService,                     // handle to service 
            SC_STATUS_PROCESS_INFO,         // info level
            (LPBYTE) &ssStatus,             // address of structure
            sizeof(SERVICE_STATUS_PROCESS), // size of structure
            &dwBytesNeeded ) )              // if buffer too small
    {
        CString csMessage;
        csMessage.Format(_T("QueryServiceStatusEx failed with error %d"), GetLastError());
        ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
        return; 
    }
 
    // Save the tick count and initial checkpoint.

    dwStartTickCount = GetTickCount();
    dwOldCheckPoint = ssStatus.dwCheckPoint;

    while (ssStatus.dwCurrentState == SERVICE_START_PENDING) 
    { 
        // Do not wait longer than the wait hint. A good interval is 
        // one-tenth the wait hint, but no less than 1 second and no 
        // more than 10 seconds. 
 
        dwWaitTime = ssStatus.dwWaitHint / 10;

        if( dwWaitTime < 1000 )
            dwWaitTime = 1000;
        else if ( dwWaitTime > 10000 )
            dwWaitTime = 10000;

        Sleep( dwWaitTime );

        // Check the status again. 
 
        if (!QueryServiceStatusEx( 
            schService,             // handle to service 
            SC_STATUS_PROCESS_INFO, // info level
            (LPBYTE) &ssStatus,             // address of structure
            sizeof(SERVICE_STATUS_PROCESS), // size of structure
            &dwBytesNeeded ) )              // if buffer too small
        {
            CString csMessage;
            csMessage.Format(_T("QueryServiceStatusEx failed with error %d"), GetLastError());
            ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());
            break; 
        }
 
        if ( ssStatus.dwCheckPoint > dwOldCheckPoint )
        {
            // Continue to wait and check.

            dwStartTickCount = GetTickCount();
            dwOldCheckPoint = ssStatus.dwCheckPoint;
        }
        else
        {
            if(GetTickCount()-dwStartTickCount > ssStatus.dwWaitHint)
            {
                // No progress made within the wait hint.
                break;
            }
        }
    } 

    // Determine whether the service is running.

    if (ssStatus.dwCurrentState == SERVICE_RUNNING) 
    {
        CString csMessage(_T("Service already running"));
        ServiceReportEvent(EVENTLOG_INFORMATION_TYPE, csMessage.GetBuffer());
    }

    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);
}

VOID StopService(void)
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    SERVICE_STATUS_PROCESS ssp;
    DWORD dwStartTime = GetTickCount();
    DWORD dwBytesNeeded;
    DWORD dwTimeout = 30000; // 30-second time-out
    DWORD dwWaitTime;

    // Get a handle to the SCM database. 
 
    schSCManager = OpenSCManager( 
        NULL,                    // local computer
        NULL,                    // ServicesActive database 
        SC_MANAGER_ALL_ACCESS);  // full access rights 
 
    if (NULL == schSCManager) 
    {
        CString csMessage;
        csMessage.Format(_T("OpenSCManager failed with error %d"), GetLastError());
        ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());

        return;
    }

    // Get a handle to the service.

    schService = OpenService( 
                schSCManager,         // SCM database 
                SVC_NAME,             // name of service 
                SERVICE_STOP | 
                SERVICE_QUERY_STATUS | 
                SERVICE_ENUMERATE_DEPENDENTS);
 
    if (schService == NULL)
    {
        CString csMessage;
        csMessage.Format(_T("OpenService failed with error %d"), GetLastError());
        ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());
        CloseServiceHandle(schSCManager);
        return;
    }    

    // Make sure the service is not already stopped.

    if ( !QueryServiceStatusEx( 
            schService, 
            SC_STATUS_PROCESS_INFO,
            (LPBYTE)&ssp, 
            sizeof(SERVICE_STATUS_PROCESS),
            &dwBytesNeeded ) )
    {
        CString csMessage;
        csMessage.Format(_T("QueryServiceStatusEx failed with error %d"), GetLastError());
        ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());
        goto stop_cleanup;
    }

    if ( ssp.dwCurrentState == SERVICE_STOPPED )
    {
        CString csMessage(_T("Service has stopped"));
        ServiceReportEvent(EVENTLOG_INFORMATION_TYPE, csMessage.GetBuffer());
        goto stop_cleanup;
    }

    // If a stop is pending, wait for it.

    while ( ssp.dwCurrentState == SERVICE_STOP_PENDING ) 
    {
        // Do not wait longer than the wait hint. A good interval is 
        // one-tenth of the wait hint but not less than 1 second  
        // and not more than 10 seconds. 
 
        dwWaitTime = ssp.dwWaitHint / 10;

        if( dwWaitTime < 1000 )
            dwWaitTime = 1000;
        else if ( dwWaitTime > 10000 )
            dwWaitTime = 10000;

        Sleep( dwWaitTime );

        if ( !QueryServiceStatusEx( 
                 schService, 
                 SC_STATUS_PROCESS_INFO,
                 (LPBYTE)&ssp, 
                 sizeof(SERVICE_STATUS_PROCESS),
                 &dwBytesNeeded ) )
        {
            CString csMessage;
            csMessage.Format(_T("QueryServiceStatusEx failed with error %d"), GetLastError());
            ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());
            goto stop_cleanup;
        }

        if ( ssp.dwCurrentState == SERVICE_STOPPED )
        {
            CString csMessage(_T("Service has stopped"));
            ServiceReportEvent(EVENTLOG_INFORMATION_TYPE, csMessage.GetBuffer());
            goto stop_cleanup;
        }

        if ( GetTickCount() - dwStartTime > dwTimeout )
        {
            goto stop_cleanup;
        }
    }

    // Send a stop code to the service.

    if ( !ControlService( 
            schService, 
            SERVICE_CONTROL_STOP, 
            (LPSERVICE_STATUS) &ssp ) )
    {
        CString csMessage;
        csMessage.Format(_T("ControlService failed with error %d"), GetLastError());
        ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());
        goto stop_cleanup;
    }

    // Wait for the service to stop.

    while ( ssp.dwCurrentState != SERVICE_STOPPED ) 
    {
        Sleep( ssp.dwWaitHint );
        if ( !QueryServiceStatusEx( 
                schService, 
                SC_STATUS_PROCESS_INFO,
                (LPBYTE)&ssp, 
                sizeof(SERVICE_STATUS_PROCESS),
                &dwBytesNeeded ) )
        {
            CString csMessage;
            csMessage.Format(_T("QueryServiceStatusEx failed with error %d"), GetLastError());
            ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());
            goto stop_cleanup;
        }

        if ( ssp.dwCurrentState == SERVICE_STOPPED )
        {
            break;
        }

        if ( GetTickCount() - dwStartTime > dwTimeout )
        {
            goto stop_cleanup;
        }
    }

stop_cleanup:
    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);
}

VOID WINAPI ServiceMain( DWORD dwArgc, LPTSTR *lpszArgv )
{
	{
		CString csMessage;
		csMessage.Format(_T("OSVRService: ServiceMain"));
		ServiceReportEvent(EVENTLOG_INFORMATION_TYPE, csMessage.GetBuffer());
	}
	
	// Register the handler function for the service

    gServiceStatusHandle = RegisterServiceCtrlHandlerEx(SVC_NAME, ServiceControlHandlerEx, nullptr);

    if( !gServiceStatusHandle )
    { 
        CString csMessage;
        csMessage.Format(_T("RegisterServiceCtrlHandler failed with error %d"), GetLastError());
        ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());
        return; 
    } 

    // These SERVICE_STATUS members remain as set here

    gServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS; 
    gServiceStatus.dwServiceSpecificExitCode = 0;

    // Report initial status to the SCM

    ReportServiceStatus( SERVICE_START_PENDING, NO_ERROR, 3000 );

    // Perform service-specific initialization and work.

    ServiceInit( dwArgc, lpszArgv );

    ServiceStart();

    // Report running status when initialization is complete.

    ReportServiceStatus( SERVICE_RUNNING, NO_ERROR, 3000 );

    // Check whether to stop the service.
    if(WaitForSingleObject(ghServiceStopEvent, INFINITE) == WAIT_OBJECT_0)
    {
        ReportServiceStatus( SERVICE_STOPPED, NO_ERROR, 0 );
    }

    ServiceUnInit();
}

VOID ServiceInit( DWORD dwArgc, LPTSTR *lpszArgv)
{
    UNREFERENCED_PARAMETER(dwArgc);
    UNREFERENCED_PARAMETER(lpszArgv);

    // TO_DO: Declare and set any required variables.
    //   Be sure to periodically call ReportServiceStatus() with 
    //   SERVICE_START_PENDING. If initialization fails, call
    //   ReportServiceStatus with SERVICE_STOPPED.

	{
		CString csMessage;
		csMessage.Format(_T("OSVRService: INIT"));
		ServiceReportEvent(EVENTLOG_INFORMATION_TYPE, csMessage.GetBuffer());
	}

	g_Service.Init();
	ReportServiceStatus(SERVICE_START_PENDING, NO_ERROR, 0);

    // Create an event. The control handler function, ServiceControlHandlerEx,
    // signals this event when it receives the stop control code.

    ghServiceStopEvent = CreateEvent(
                         NULL,    // default security attributes
                         TRUE,    // manual reset event
                         FALSE,   // not signaled
                         NULL);   // no name

    if ( ghServiceStopEvent == NULL)
    {
        ReportServiceStatus( SERVICE_STOPPED, NO_ERROR, 0 );
        return;
    }
}

VOID ServiceUnInit()
{
    g_Service.UnInit();
}

VOID ServiceStart(void)
{
	{
		CString csMessage;
		csMessage.Format(_T("OSVRService: Start"));
		ServiceReportEvent(EVENTLOG_INFORMATION_TYPE, csMessage.GetBuffer());
	}
	
	DEV_BROADCAST_DEVICEINTERFACE NotificationFilter = {};
    NotificationFilter.dbcc_size = sizeof(NotificationFilter);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_classguid = GUID_DEVINTERFACE_HID;

    if(gServiceStatusHandle)
    {
        ghDevNotify = RegisterDeviceNotification(gServiceStatusHandle, 
                                                 &NotificationFilter,
                                                 DEVICE_NOTIFY_SERVICE_HANDLE | DEVICE_NOTIFY_ALL_INTERFACE_CLASSES);
        if(!ghDevNotify)
        {
            CString csMessage;
            csMessage.Format(_T("RegisterDeviceNotification failed with error %d"), GetLastError());
            ServiceReportEvent(EVENTLOG_ERROR_TYPE, csMessage.GetBuffer());
        }
    }

	g_Service.Start();

	ReportServiceStatus(SERVICE_RUNNING, NO_ERROR, 0);
}

VOID ServiceStop(void)
{
	{
		CString csMessage;
		csMessage.Format(_T("OSVRService: Stop"));
		ServiceReportEvent(EVENTLOG_INFORMATION_TYPE, csMessage.GetBuffer());
	}
	g_Service.Stop();

    if(ghDevNotify)
    {
        UnregisterDeviceNotification(ghDevNotify);
    }

    SetEvent(ghServiceStopEvent);
}

VOID ServicePause(void)
{
    g_Service.Pause();

    ReportServiceStatus(SERVICE_PAUSED, NO_ERROR, 0);
}

VOID ServiceContinue(void)
{
    g_Service.Continue();

    ReportServiceStatus(SERVICE_RUNNING, NO_ERROR, 0);
}

VOID ReportServiceStatus( DWORD dwCurrentState,
                          DWORD dwWin32ExitCode,
                          DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.

    gServiceStatus.dwCurrentState = dwCurrentState;
    gServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
    gServiceStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
    {
        gServiceStatus.dwControlsAccepted = 0;
    }
    else
    {
        gServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    }

    if ( (dwCurrentState == SERVICE_RUNNING) ||
           (dwCurrentState == SERVICE_STOPPED) )
    {
        gServiceStatus.dwCheckPoint = 0;
    }
    else
    {
        gServiceStatus.dwCheckPoint = dwCheckPoint++;
    }

    // Report the status of the service to the SCM.
    SetServiceStatus( gServiceStatusHandle, &gServiceStatus );
}

DWORD WINAPI ServiceControlHandlerEx( DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext )
{
    UNREFERENCED_PARAMETER(dwEventType);
    UNREFERENCED_PARAMETER(lpEventData);
    UNREFERENCED_PARAMETER(lpContext);

    // Handle the requested control code. 

    switch(dwControl) 
    {
    case SERVICE_CONTROL_STOP: 
         ReportServiceStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

         // Signal the service to stop.
         ServiceStop();
         return NO_ERROR;
    case SERVICE_CONTROL_PAUSE:
        ReportServiceStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
        return NO_ERROR;
    case SERVICE_CONTROL_CONTINUE:
        ReportServiceStatus(SERVICE_CONTINUE_PENDING, NO_ERROR, 0);
        return NO_ERROR;
      //case SERVICE_CONTROL_SHUTDOWN:
      //    return NO_ERROR;
      case SERVICE_CONTROL_DEVICEEVENT:
          g_Service.OnDeviceEvent(dwEventType, lpEventData);
          return NO_ERROR;
      default: 
         break;
    }

    return NO_ERROR;
}

VOID ServiceReportEvent(LPTSTR szFunction) 
{ 
    HANDLE hEventSource;
    LPCTSTR lpszStrings[2];
    TCHAR Buffer[80];

    hEventSource = RegisterEventSource(NULL, SVC_NAME);

    if( NULL != hEventSource )
    {
        StringCchPrintf(Buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());

        lpszStrings[0] = SVC_NAME;
        lpszStrings[1] = Buffer;

        ReportEvent(hEventSource,        // event log handle
                    EVENTLOG_ERROR_TYPE, // event type
                    0,                   // event category
                    SVC_ERROR,           // event identifier
                    NULL,                // no security identifier
                    2,                   // size of lpszStrings array
                    0,                   // no binary data
                    lpszStrings,         // array of strings
                    NULL);               // no binary data

        DeregisterEventSource(hEventSource);
    }
}

VOID ServiceReportEvent(WORD EventType, LPTSTR szMessage)
{
    HANDLE hEventSource;
    LPCTSTR lpszStrings[2];
    TCHAR Buffer[80];

    hEventSource = RegisterEventSource(NULL, SVC_NAME);

    if( NULL != hEventSource )
    {
        StringCchPrintf(Buffer, 80, TEXT("%s"), szMessage);

        lpszStrings[0] = SVC_NAME;
        lpszStrings[1] = Buffer;

        DWORD EventId = 0;
        switch(EventType)
        {
        case EVENTLOG_SUCCESS:
            EventId = SVC_SUCCESS;
            break;
        case EVENTLOG_ERROR_TYPE:
            EventId = SVC_ERROR;
            break;
        case EVENTLOG_WARNING_TYPE:
            EventId = SVC_SUCCESS;
            break;
        case EVENTLOG_INFORMATION_TYPE:
            EventId = SVC_SUCCESS;
            break;
        case EVENTLOG_AUDIT_SUCCESS:
            EventId = SVC_SUCCESS;
            break;
        case EVENTLOG_AUDIT_FAILURE:
            EventId = SVC_ERROR;
            break;
        }

        ReportEvent(hEventSource,        // event log handle
                    EventType,           // event type
                    0,                   // event category
                    EventId,             // event identifier
                    NULL,                // no security identifier
                    2,                   // size of lpszStrings array
                    0,                   // no binary data
                    lpszStrings,         // array of strings
                    NULL);               // no binary data

        DeregisterEventSource(hEventSource);
    }
}
