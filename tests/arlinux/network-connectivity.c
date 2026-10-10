/* Compile with LLVM-MinGW; link -lole32 -luuid. No network traffic is sent. */
#define COBJMACROS
#include <windows.h>
#include <objbase.h>
#include <netlistmgr.h>
#include <stdio.h>

int main(void)
{
    INetworkListManager *manager = NULL;
    VARIANT_BOOL internet = VARIANT_FALSE, connected = VARIANT_FALSE;
    NLM_CONNECTIVITY flags = NLM_CONNECTIVITY_DISCONNECTED;
    HRESULT hr;
    FILE *result = fopen("C:\\network-connectivity-result.txt", "w");
    int status = 1;
    if (!result) return 2;
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) goto done;
    hr = CoCreateInstance(&CLSID_NetworkListManager, NULL, CLSCTX_INPROC_SERVER,
                         &IID_INetworkListManager, (void **)&manager);
    if (FAILED(hr)) goto done;
    hr = INetworkListManager_IsConnectedToInternet(manager, &internet);
    if (FAILED(hr)) goto done;
    hr = INetworkListManager_IsConnected(manager, &connected);
    if (FAILED(hr)) goto done;
    hr = INetworkListManager_GetConnectivity(manager, &flags);
    if (FAILED(hr)) goto done;
    if (!!internet != !!(flags & (NLM_CONNECTIVITY_IPV4_INTERNET | NLM_CONNECTIVITY_IPV6_INTERNET))) goto done;
    if (internet && !connected) goto done;
    status = 0;
done:
    fprintf(result,"status=%d hr=%08lx internet=%d connected=%d flags=%x\n",
            status,(unsigned long)hr,!!internet,!!connected,(unsigned int)flags);
    fclose(result);
    if (manager) INetworkListManager_Release(manager);
    CoUninitialize();
    return status;
}
