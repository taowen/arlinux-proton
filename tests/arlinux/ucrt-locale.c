/* Exercise UCRT locale names with code-page suffixes, independent of the host CRT. */
#include <windows.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    char *(__cdecl *set_locale)(int, const char *);
    void *(__cdecl *create_locale)(int, const char *);
    void (__cdecl *free_locale)(void *);
    size_t (__cdecl *to_wide)(wchar_t *, const char *, size_t);
    size_t (__cdecl *to_narrow)(char *, const wchar_t *, size_t);
    const char *names[] = {"en-US.UTF-8", "zh-CN.utf8", "en-US.65001", ".utf8"};
    const char input[] = "\xe4\xb8\xad\xe6\x96\x87";
    wchar_t wide[8];
    char narrow[16];
    FILE *output = fopen("C:\\ucrt-locale-result.txt", "w");
    HMODULE module = LoadLibraryW(L"ucrtbase.dll");
    void *locale;
    char *name;
    unsigned int i;
    int status = 1;
    if (!output) return 2;
    if (!module) goto done;
    set_locale = (void *)GetProcAddress(module, "setlocale");
    create_locale = (void *)GetProcAddress(module, "_create_locale");
    free_locale = (void *)GetProcAddress(module, "_free_locale");
    to_wide = (void *)GetProcAddress(module, "mbstowcs");
    to_narrow = (void *)GetProcAddress(module, "wcstombs");
    if (!set_locale || !create_locale || !free_locale || !to_wide || !to_narrow) goto done;
    for (i = 0; i < sizeof(names)/sizeof(names[0]); ++i)
    {
        name = set_locale(0, names[i]);
        fprintf(output, "locale=%s result=%s\n", names[i], name ? name : "NULL");
        if (!name) goto done;
        if (to_wide(wide, input, 8) != 2 || wide[0] != 0x4e2d || wide[1] != 0x6587) goto done;
        if (to_narrow(narrow, wide, 16) != 6 || strcmp(narrow, input)) goto done;
        locale = create_locale(0, names[i]);
        if (!locale) goto done;
        free_locale(locale);
    }
    if (set_locale(0, "invalid-locale.UTF-8")) goto done;
    if (set_locale(0, "en-US.99999")) goto done;
    if (!set_locale(0, "C")) goto done;
    status = 0;
done:
    fprintf(output, "status=%d\n", status);
    fclose(output);
    if (module) FreeLibrary(module);
    return status;
}
