//https://docs.microsoft.com/de-de/windows/win32/wmisdk/example--getting-wmi-data-from-the-local-computer-asynchronously
#include "querysink.h"

ULONG QuerySink::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

ULONG QuerySink::Release()
{
    LONG lRef = InterlockedDecrement(&m_lRef);
    if(lRef == 0) { delete this; }
    return lRef;
}

HRESULT QuerySink::QueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IUnknown || riid == IID_IWbemObjectSink) {
        *ppv = (IWbemObjectSink *) this;
        AddRef();
        return WBEM_S_NO_ERROR;
    }
    else return E_NOINTERFACE;
}

HRESULT QuerySink::Indicate(long lObjectCount, IWbemClassObject **apObjArray)
{
    HRESULT hres = S_OK;
    std::cout << "Indicate called with " << lObjectCount << " results" << std::endl;

    for (int i = 0; i < lObjectCount; i++) {
        VARIANT varName;
        hres = apObjArray[i]->Get(_bstr_t(L"Name"),
                                  0, &varName, 0, 0);

        if (FAILED(hres)) {
            std::cout << "Failed to get the data from the query" << " Error code = 0x" << std::hex << hres << std::endl; return WBEM_E_FAILED;
        }

        printf("Name: %ls\n", V_BSTR(&varName));
    }

    return WBEM_S_NO_ERROR;
}

HRESULT QuerySink::SetStatus(LONG lFlags, HRESULT hResult, BSTR strParam, IWbemClassObject __RPC_FAR *pObjParam)
{
    if(lFlags == WBEM_STATUS_COMPLETE) {
        printf("Call complete.\n");

        EnterCriticalSection(&threadLock);
        bDone = true;
        LeaveCriticalSection(&threadLock);
    }
    else if(lFlags == WBEM_STATUS_PROGRESS) {
        printf("Call in progress.\n");
    }

    return WBEM_S_NO_ERROR;
}

bool QuerySink::IsDone()
{
    bool done = true;

    EnterCriticalSection(&threadLock);
    done = bDone;
    LeaveCriticalSection(&threadLock);

    return done;
}    // end of QuerySink.cpp
