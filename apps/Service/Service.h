
#ifndef _SERVICE_H_
#define _SERVICE_H_

#pragma once

class CService
{
public:
    CService() {};
    ~CService() {};

    virtual BOOL Start() { return TRUE; };
    virtual BOOL Stop() { return TRUE; };
    virtual BOOL Pause() { return TRUE; };
    virtual BOOL Continue() { return TRUE; };
    virtual void OnDeviceEvent(DWORD dwEvent, LPVOID lpData) { UNREFERENCED_PARAMETER(dwEvent); UNREFERENCED_PARAMETER(lpData); };
};

#endif
