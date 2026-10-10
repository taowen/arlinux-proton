/* Compile with LLVM-MinGW for aarch64, x86_64 or i686; link -lruntimeobject -lole32.
 * Exercises the WinRT ABI without needing generated SDK interface headers.
 */
#include <windows.h>
#include <roapi.h>
#include <winstring.h>
#include <stdio.h>

typedef struct history history;
typedef struct manager manager;
struct manager_vtbl
{
    HRESULT (WINAPI *query)(manager *, REFIID, void **);
    ULONG (WINAPI *addref)(manager *);
    ULONG (WINAPI *release)(manager *);
    void *inspectable[3];
    HRESULT (WINAPI *get_history)(manager *, history **);
};
struct manager { const struct manager_vtbl *vtbl; };
struct history_vtbl
{
    HRESULT (WINAPI *query)(history *, REFIID, void **);
    ULONG (WINAPI *addref)(history *);
    ULONG (WINAPI *release)(history *);
    void *inspectable[3];
    void *remove[5];
    HRESULT (WINAPI *clear)(history *);
    HRESULT (WINAPI *clear_with_id)(history *, HSTRING);
};
struct history { const struct history_vtbl *vtbl; };

int main(void)
{
    static const WCHAR name[] = L"Windows.UI.Notifications.ToastNotificationManager";
    static const GUID iid = {0x7ab93c52,0x0e48,0x4750,{0xba,0x9d,0x1a,0x41,0x13,0x98,0x18,0x47}};
    HSTRING str = NULL;
    manager *object = NULL;
    history *list = NULL;
    HRESULT hr;
    FILE *result = fopen("C:\\toast-history-result.txt", "w");
    int status = 1;
    if (!result) return 2;
    hr = RoInitialize(RO_INIT_MULTITHREADED);
    if (FAILED(hr)) goto done;
    hr = WindowsCreateString(name, ARRAYSIZE(name)-1, &str);
    if (FAILED(hr)) goto done;
    hr = RoGetActivationFactory(str, &iid, (void **)&object);
    if (FAILED(hr) || !object) goto done;
    hr = object->vtbl->get_history(object, &list);
    if (FAILED(hr) || !list) goto done;
    hr = list->vtbl->clear(list);
    if (FAILED(hr)) goto done;
    hr = list->vtbl->clear_with_id(list, str);
    if (FAILED(hr)) goto done;
    hr = object->vtbl->get_history(object, NULL);
    if (hr != E_POINTER) goto done;
    status = 0;
done:
    fprintf(result,"status=%d hr=%08lx manager=%d history=%d\n",status,(unsigned long)hr,!!object,!!list);
    fclose(result);
    if (list) list->vtbl->release(list);
    if (object) object->vtbl->release(object);
    WindowsDeleteString(str);
    RoUninitialize();
    return status;
}
