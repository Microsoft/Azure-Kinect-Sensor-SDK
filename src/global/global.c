// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

// This library
#include <k4ainternal/global.h>

// System dependencies

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include <assert.h>
#ifdef _WIN32

C_ASSERT(sizeof(k4a_init_once_t) == sizeof(INIT_ONCE));

static BOOL CALLBACK InitGlobalFunction(PINIT_ONCE InitOnce, PVOID Parameter, PVOID *lpContext)
{
    UNREFERENCED_PARAMETER(InitOnce);
    UNREFERENCED_PARAMETER(lpContext);

    k4a_init_once_function_t *callback = (k4a_init_once_function_t *)Parameter;

    callback();

    return TRUE;
}
#endif

void global_init_once(k4a_init_once_t *init_once, k4a_init_once_function_t *init_function)
{

#ifdef _WIN32
    if (InitOnceExecuteOnce((INIT_ONCE *)init_once, InitGlobalFunction, (void *)init_function, NULL))
    {
        return;
    }
#else
    if (0 == pthread_once((pthread_once_t *)init_once, init_function))
    {
        return;
    }
#endif

    assert(0);
    return;
}
