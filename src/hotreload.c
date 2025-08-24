/* header */
#ifndef HOTRELOAD_H
#define HOTRELOAD_H

#ifdef HOTRELOAD_C
#define HDEF static
#else
#define HDEF extern
#endif /* HOTRELOAD_C */

#include "game.c"

#ifdef __unix
    #include <dlfcn.h>
    HDEF const char* dllPath;
    HDEF void* LoadLib(const char* f);
    HDEF void* LoadLibProc(void* h, const char* n);
    HDEF int FreeLib(void* h);
    HDEF const char* GetLibError(void);
    typedef void* LibHandle;
    typedef void* ProcHandle;
#elif defined(_WIN32)
    #include <windef.h> /* data types */
    HDEF const char* dllPath;
    HDEF const char* tempPath;
    HDEF HMODULE LoadLib(LPCSTR f);
    HDEF FARPROC LoadLibProc(HMODULE h, LPCSTR n);
    HDEF BOOL FreeLib(HMODULE h);
    HDEF const char* GetLibError(void);
    typedef HMODULE LibHandle;
    typedef FARPROC ProcHandle;
#endif /* __unix */

typedef int (*gameStateFn)(GameState*);

HDEF struct GameDll
{
    LibHandle handle;
    gameStateFn InitGame;
    gameStateFn UpdateDrawFrame;
} gameDll;

HDEF int LoadGameDll(void);
HDEF int HandleHotReload(void);

#endif /* HOTRELOAD_H */

/* implementation */
#ifdef HOTRELOAD_C

#include "SDL.h"
#include <stdio.h>

#ifdef __unix
    #include <dlfcn.h>
    HDEF const char* dllPath = "game.so";
    HDEF void* LoadLib(const char* f) { return dlopen(f, RTLD_LAZY); }
    HDEF void* LoadLibProc(void* h, const char* n) { return dlsym(h, n); }
    HDEF int FreeLib(void* h) { return dlclose(h); }
    HDEF const char* GetLibError(void) { return dlerror(); }
/* NOTE: windows currently untested */
#elif defined(_WIN32)
    /* MSVC expects windows.h to be included in its entirety */
    /* NOTE: MSVC is currently untested */
    #ifdef _MSC_VER
        #define WIN32_LEAN_AND_MEAN
        #include <windows.h>
    /* on mingw, we can pick and choose which windows headers to include */
    #else
        #include <windef.h> /* data types */
        #include <errhandlingapi.h> /* GetLastError */
        #include <winbase.h> /* CopyFileA, GetLastError */
        #include <libloaderapi.h> /* LoadLibraryA, GetProcAddress, FreeLibrary */
    #endif /* _MSC_VER */
    #include <string.h> /* needed for string concatenation in filenames and error reporting */
    HDEF const char* dllPath = "game.dll";
    HDEF const char* tempPath = "_temp.dll";
    HDEF HMODULE LoadLib(LPCSTR f)
    {
        #define FPLEN 128
        static BOOL gotPath = FALSE;
        static char filePath[FPLEN]; /* path to game dll */

        if (!gotPath)
        {
            /* get path of executable */
            GetModuleFileNameA(NULL, filePath, FPLEN);
            /* truncate exe name */
            int i;
            for (i = FPLEN; i >= 0; --i)
            {
                if (filePath[i] == '\\')
                {
                    filePath[i + 1] = '\0';
                    break;
                }
            }
            /* add dll filename to filepath */
            strcat(filePath, dllPath);
            gotPath = TRUE;
        }
 
        BOOL err = CopyFile(filePath, tempPath, FALSE); /* make a copy of the dll file to get around locks */
        if (err != 0)
            return LoadLibrary(tempPath);
        else
            return NULL;
        #undef FPLEN
    }
    HDEF FARPROC LoadLibProc(HMODULE h, LPCSTR n) { return GetProcAddress(h, n); }
    HDEF BOOL FreeLib(HMODULE h)
    {
        BOOL err = FreeLibrary(h);
        if (err != 0)
            return FALSE; /* the rest of the API expects errors to be nonzero, so we flip the value here */
        else
            return TRUE;
    }
    HDEF const char* GetLibError(void)
    {
        static char errorCode[4];
        sprintf(errorCode, "%u", (unsigned int)(GetLastError()));
        return errorCode;
    }
#endif /* __unix */

HDEF int LoadGameDll(void)
{
    const size_t len = 128;
    char err[len];
    gameDll.handle = LoadLib(dllPath);
    if (!gameDll.handle)
    {
        sprintf(err, "%s", GetLibError());
        fprintf(stderr, "Failed to load dll, error: `%s`\n", err);
        return 1;
    }
    ProcHandle initptr = LoadLibProc(gameDll.handle, "InitGame");
    ProcHandle updateptr = LoadLibProc(gameDll.handle, "UpdateDrawFrame");
    if (!initptr || !updateptr)
    {
        sprintf(err, "%s", GetLibError());
        fprintf(stderr, "Failed to load procedure from library, error: `%s`\n", err);
        return 1;
    }
    gameDll.InitGame = (gameStateFn)initptr;
    gameDll.UpdateDrawFrame = (gameStateFn)updateptr;
    return 0;
}

HDEF int HandleHotReload(void)
{
    int err = FreeLib(gameDll.handle);
    if (err)
    {
        fprintf(stderr, "HandleHotReload: Failed to close game dll, error %d\n", err);
        return err;
    }
    err = LoadGameDll();
    if (err)
    {
        fprintf(stderr, "HandleHotReload: Failed to load game dll\n");
        return err;
    }
    else
    {
        printf("HandleHotReload: Game dll reloaded at runtime %d ms, status of LoadGameDll: %d\n", SDL_GetTicks(), err);
        return err;
    }

    return 0;
}

#endif /* HOTRELOAD_C */
