#include "modules/ffi/ffiConfig.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef FFI_PLATFORM_WINDOWS
#include <windows.h>
#elif defined(FFI_PLATFORM_LINUX) || defined(FFI_PLATFORM_MACOS)
#include <dlfcn.h>
#endif

typedef struct {
    char *name;
    void *handle;
    bool loaded;
} DynamicLibrary;

DynamicLibrary* dynamic_library_create(const char *name) {
    DynamicLibrary *lib = safe_malloc(sizeof(DynamicLibrary));
    lib->name = string_copy(name);
    lib->handle = NULL;
    lib->loaded = false;
    return lib;
}

void dynamic_library_destroy(DynamicLibrary *lib) {
    if (!lib) return;
    
    if (lib->loaded && lib->handle) {
#ifdef FFI_PLATFORM_WINDOWS
        FreeLibrary((HMODULE)lib->handle);
#elif defined(FFI_PLATFORM_LINUX) || defined(FFI_PLATFORM_MACOS)
        dlclose(lib->handle);
#endif
    }
    
    safe_free(lib->name);
    safe_free(lib);
}

bool dynamic_library_load(DynamicLibrary *lib) {
    if (!lib) return false;
    
#ifdef FFI_PLATFORM_WINDOWS
    lib->handle = LoadLibraryA(lib->name);
#elif defined(FFI_PLATFORM_LINUX) || defined(FFI_PLATFORM_MACOS)
    lib->handle = dlopen(lib->name, RTLD_LAZY);
#endif
    
    lib->loaded = (lib->handle != NULL);
    return lib->loaded;
}

void* dynamic_library_get_symbol(DynamicLibrary *lib, const char *symbol_name) {
    if (!lib || !lib->loaded || !lib->handle) return NULL;
    
    (void)symbol_name;
    
#ifdef FFI_PLATFORM_WINDOWS
    return GetProcAddress((HMODULE)lib->handle, symbol_name);
#elif defined(FFI_PLATFORM_LINUX) || defined(FFI_PLATFORM_MACOS)
    return dlsym(lib->handle, symbol_name);
#else
    // Unsupported platform
    return NULL;
#endif
}
