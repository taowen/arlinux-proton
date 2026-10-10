/* Use an existing font file; no proprietary font is distributed with this test. */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct { UINT32 version; void *callback; BOOL thread, codecs; } StartupInput;
__declspec(dllimport) int WINAPI GdiplusStartup(ULONG_PTR *, const StartupInput *, void *);
__declspec(dllimport) void WINAPI GdiplusShutdown(ULONG_PTR);
__declspec(dllimport) int WINAPI GdipNewPrivateFontCollection(void **);
__declspec(dllimport) int WINAPI GdipDeletePrivateFontCollection(void **);
__declspec(dllimport) int WINAPI GdipPrivateAddFontFile(void *, const WCHAR *);
__declspec(dllimport) int WINAPI GdipGetFontCollectionFamilyCount(void *, int *);
__declspec(dllimport) int WINAPI GdipGetFontCollectionFamilyList(void *, int, void **, int *);
__declspec(dllimport) int WINAPI GdipGetFamilyName(void *, WCHAR *, WORD);
__declspec(dllimport) int WINAPI GdipCreateFontFamilyFromName(const WCHAR *, void *, void **);
__declspec(dllimport) int WINAPI GdipDeleteFontFamily(void *);

int wmain(int argc, WCHAR **argv)
{
    StartupInput input = {1, NULL, FALSE, FALSE};
    ULONG_PTR token;
    void *collection = NULL, *family = NULL, **families;
    int count = 0, found = 0, failures = 0, status, i;
    FILE *result;
    if (argc != 2) return 2;
    result = fopen("C:\\gdiplus-private-family-result.txt", "w");
    if (!result) return 2;
    if (GdiplusStartup(&token, &input, NULL)) { fclose(result); return 2; }
    if (GdipNewPrivateFontCollection(&collection)) { GdiplusShutdown(token); fclose(result); return 2; }
    status = GdipCreateFontFamilyFromName(L"missing family 194b09", collection, &family);
    if (status != 14) failures++;
    status = GdipPrivateAddFontFile(collection, argv[1]);
    fprintf(result, "add=%d\n", status);
    if (status || GdipGetFontCollectionFamilyCount(collection, &count) || count < 1 || count > 256)
    {
        GdipDeletePrivateFontCollection(&collection);
        GdiplusShutdown(token);
        fclose(result);
        return 2;
    }
    families = calloc(count, sizeof(*families));
    if (!families) { GdipDeletePrivateFontCollection(&collection); GdiplusShutdown(token); fclose(result); return 2; }
    if (GdipGetFontCollectionFamilyList(collection, count, families, &found) || found != count) failures++;
    for (i = 0; i < found; i++)
    {
        WCHAR name[LF_FACESIZE];
        char utf8[128];
        if (GdipGetFamilyName(families[i], name, 0)) { failures++; continue; }
        WideCharToMultiByte(CP_UTF8, 0, name, -1, utf8, sizeof(utf8), NULL, NULL);
        status = GdipCreateFontFamilyFromName(name, collection, &family);
        fprintf(result, "family=%s lookup=%d\n", utf8, status);
        if (status) failures++;
        else GdipDeleteFontFamily(family);
        CharLowerBuffW(name, lstrlenW(name));
        status = GdipCreateFontFamilyFromName(name, collection, &family);
        if (status) failures++;
        else GdipDeleteFontFamily(family);
    }
    status = GdipCreateFontFamilyFromName(L"missing family 194b09", collection, &family);
    if (status != 14) failures++;
    status = GdipCreateFontFamilyFromName(NULL, collection, &family);
    if (status != 2) failures++;
    free(families);
    GdipDeletePrivateFontCollection(&collection);
    GdiplusShutdown(token);
    fprintf(result, "failures=%d\n", failures);
    fclose(result);
    return failures ? 1 : 0;
}
