
#ifndef _SERVICEIMPL_H
#define _SERVICEIMPL_H

#pragma once

#include "Service.h"
#include <osvr/Server/ConfigureServerFromFile.h>
#include <osvr/Server/RegisterShutdownHandler.h>

class CServiceImpl : public CService
{
public:
    CServiceImpl();
    ~CServiceImpl();

    BOOL Init();
    BOOL UnInit();

    // CRzService methods
    BOOL Start();
    BOOL Stop();
    BOOL Pause();
    BOOL Continue();

    void OnDeviceEvent(DWORD dwEvent, LPVOID lpData);

private:

	osvr::server::ServerPtr server;
	std::string configName;

	// Redirects used for std::cout and std:cerr
	std::streambuf *o_psbuf, *o_backup;
	std::ofstream o_filestr;
	std::streambuf *e_psbuf, *e_backup;
	std::ofstream e_filestr;

    // Disable copy constructor.
	CServiceImpl(const CServiceImpl&);
	CServiceImpl& operator=(const CServiceImpl);

};

#endif
