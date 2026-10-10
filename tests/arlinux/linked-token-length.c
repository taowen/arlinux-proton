/* Compile with LLVM-MinGW; link -ladvapi32. Compare with native Windows. */
#include <windows.h>
#include <stdio.h>
int main(void)
{
    HANDLE token;
    TOKEN_LINKED_TOKEN info;
    DWORD size = 0, error;
    BOOL ok;
    FILE *result = fopen("C:\\linked-token-length-result.txt", "w");
    int status = 1;
    if (!result) return 2;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) return 3;
    SetLastError(0);
    ok = GetTokenInformation(token, TokenLinkedToken, NULL, 0, &size);
    error = GetLastError();
    fprintf(result,"empty ok=%d error=%lu size=%lu\n",ok,(unsigned long)error,(unsigned long)size);
    if (ok || error != ERROR_BAD_LENGTH || size != sizeof(info)) goto done;
    SetLastError(0);
    ok = GetTokenInformation(token, TokenLinkedToken, &info, sizeof(info)-1, &size);
    error = GetLastError();
    fprintf(result,"short ok=%d error=%lu size=%lu\n",ok,(unsigned long)error,(unsigned long)size);
    if (ok || error != ERROR_BAD_LENGTH || size != sizeof(info)) goto done;
    /* Variable-sized information keeps ERROR_INSUFFICIENT_BUFFER semantics. */
    ok = GetTokenInformation(token, TokenUser, NULL, 0, &size);
    if (ok || GetLastError() != ERROR_INSUFFICIENT_BUFFER) goto done;
    status = 0;
done:
    fprintf(result,"status=%d\n",status);
    CloseHandle(token);
    fclose(result);
    return status;
}
