/* Compile against Wine's dxcore.h; load DXCore dynamically. */
#define COBJMACROS
#include <windows.h>
#include <initguid.h>
#include <dxcore.h>
#include <stdio.h>

int main(void)
{
    HRESULT (WINAPI *create_factory)(REFIID, void **);
    IDXCoreAdapterFactory *factory = NULL;
    IDXCoreAdapterList *list = NULL;
    IDXCoreAdapter *adapter = NULL;
    DXCoreAdapterProperty properties[] = {DedicatedAdapterMemory, DedicatedSystemMemory, SharedSystemMemory};
    HMODULE module = LoadLibraryW(L"dxcore.dll");
    FILE *output = fopen("C:\\dxcore-properties-result.txt", "w");
    UINT64 value;
    size_t size;
    BYTE detachable;
    HRESULT hr;
    unsigned int i;
    int status = 1;
    if (!output) return 2;
    if (!module || !(create_factory = (void *)GetProcAddress(module, "DXCoreCreateAdapterFactory"))) goto done;
    if (FAILED(create_factory(&IID_IDXCoreAdapterFactory, (void **)&factory))) goto done;
    hr = IDXCoreAdapterFactory_CreateAdapterList(factory, 1, &DXCORE_ADAPTER_ATTRIBUTE_D3D12_CORE_COMPUTE,
            &IID_IDXCoreAdapterList, (void **)&list);
    if (FAILED(hr) || !IDXCoreAdapterList_GetAdapterCount(list)) goto done;
    if (FAILED(IDXCoreAdapterList_GetAdapter(list, 0, &IID_IDXCoreAdapter, (void **)&adapter))) goto done;
    for (i = 0; i < sizeof(properties)/sizeof(properties[0]); ++i)
    {
        if (!IDXCoreAdapter_IsPropertySupported(adapter, properties[i])) goto done;
        if (FAILED(IDXCoreAdapter_GetPropertySize(adapter, properties[i], &size)) || size != sizeof(value)) goto done;
        if (IDXCoreAdapter_GetProperty(adapter, properties[i], sizeof(value)-1, &value) != E_INVALIDARG) goto done;
        if (IDXCoreAdapter_GetProperty(adapter, properties[i], sizeof(value), NULL) != E_POINTER) goto done;
        if (FAILED(IDXCoreAdapter_GetProperty(adapter, properties[i], sizeof(value), &value))) goto done;
        fprintf(output, "property=%u bytes=%llu\n", properties[i], (unsigned long long)value);
    }
    if (!IDXCoreAdapter_IsPropertySupported(adapter, IsDetachable)) goto done;
    if (FAILED(IDXCoreAdapter_GetPropertySize(adapter, IsDetachable, &size)) || size != sizeof(detachable)) goto done;
    if (FAILED(IDXCoreAdapter_GetProperty(adapter, IsDetachable, sizeof(detachable), &detachable))) goto done;
    fprintf(output, "detachable=%u\n", detachable);
    if (detachable) goto done;
    if (IDXCoreAdapter_IsPropertySupported(adapter, (DXCoreAdapterProperty)0xffffffff)) goto done;
    status = 0;
done:
    fprintf(output, "status=%d\n", status);
    if (adapter) IDXCoreAdapter_Release(adapter);
    if (list) IDXCoreAdapterList_Release(list);
    if (factory) IDXCoreAdapterFactory_Release(factory);
    if (module) FreeLibrary(module);
    fclose(output);
    return status;
}
