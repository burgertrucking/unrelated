/* Single header file hot reloading module that wraps syscalls. Use only in one object file (eg. main) or else it won't work */
#ifndef HOTRELOAD_H
#define HOTRELOAD_H

#include "SDL/SDL.h"
#include "game.h"
#include <stdio.h>

#ifdef __unix
    #include <dlfcn.h>
    static const char* dllPath = "libgame.so";
    static void* LoadLib(const char* f) { return dlopen(f, RTLD_LAZY); }
    static void* LoadLibProc(void* h, const char* n) { return dlsym(h, n); }
    static int FreeLib(void* h) { return dlclose(h); }
    static const char* GetLibError(void) { return dlerror(); }
    typedef void* LibHandle;
    typedef void* ProcHandle;
/* NOTE: windows currently untested */
#elif defined(_WIN32)
    /* MSVC expects windows.h to be included in its entirety */
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
    static const char* dllPath = "game.dll";
    static const char* tempPath = "_temp.dll";
    static HMODULE LoadLib(LPCSTR f)
    {
        const size_t len = 128;
        static bool gotPath = false;
        static char filePath[len] = { 0 }; /* Exact path to game.dll */
        if (!gotPath)
        {
            strcat(filePath, GetApplicationDirectory());
            strcat(filePath, dllPath);
            gotPath = true;
        }
 
        /* Make a copy of the dll to get around file locks when you recompile; */
        /* will be in working directory (typically project root) */
        BOOL err = CopyFile(filePath, tempPath, FALSE);
        if (err != 0) /* success values are nonzero for some reason */
            return LoadLibrary(tempPath);
        else
            return NULL;
    }
    static FARPROC LoadLibProc(HMODULE h, LPCSTR n) { return GetProcAddress(h, n); }
    static BOOL FreeLib(HMODULE h)
    {
        BOOL err = FreeLibrary(h);
        if (err != 0)
            return FALSE; /* the rest of the API expects errors to be nonzero, so we flip the value here */
        else
            return TRUE;
    }
    static const char* GetLibError(void)
    {
        static char errorCode[4];
        sprintf(errorCode, "%u", (unsigned int)(GetLastError()));
        return errorCode;
    }
    typedef HMODULE LibHandle;
    typedef FARPROC ProcHandle;
#endif

typedef void (*gameStateFn)(GameState*);

static struct GameDll
{
    LibHandle handle;
    gameStateFn InitGame;
    gameStateFn UpdateDrawFrame;
} gameDll;

static int LoadGameDll(void)
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

static int HandleHotReload(void)
{
    int err = FreeLib(gameDll.handle);
    if (err)
    {
        fprintf(stderr, "HandleHotReload: Failed to close game dll, error %d", err);
        return err;
    }
    err = LoadGameDll();
    if (err)
    {
        fprintf(stderr, "HandleHotReload: Failed to load game dll");
        return err;
    }
    else
    {
        fprintf(stderr, "HandleHotReload: Game dll reloaded at runtime %d ms, status of LoadGameDll: %d", SDL_GetTicks(), err);
        return err;
    }

    return 0;
}

#endif /* HOTRELOAD_H */
