//https://docs.microsoft.com/de-de/windows/win32/wmisdk/example--getting-wmi-data-from-the-local-computer-asynchronously
#ifndef QUERYSINK_H
#define QUERYSINK_H

#define _WIN32_DCOM
#include <iostream>

#include <comdef.h>
#include <Wbemidl.h>

//#pragma comment(lib, "wbemuuid.lib")

class QuerySink : public IWbemObjectSink
{
    LONG m_lRef;
    bool bDone;
    CRITICAL_SECTION threadLock; // for thread safety

public:
    QuerySink() {
        m_lRef = 0;
        bDone = false;
        InitializeCriticalSection(&threadLock);
    }

    ~QuerySink() {
        bDone = true;
        DeleteCriticalSection(&threadLock);
    }

    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);

    virtual HRESULT STDMETHODCALLTYPE Indicate(LONG lObjectCount, IWbemClassObject __RPC_FAR *__RPC_FAR *apObjArray);

    virtual HRESULT STDMETHODCALLTYPE SetStatus(LONG lFlags, HRESULT hResult, BSTR strParam, IWbemClassObject __RPC_FAR *pObjParam);

    bool IsDone();
};

#endif // QUERYSINK_H
