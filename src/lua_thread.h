/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lua_thread.h
 *     Header file for lua_thread.c.
 * @par Purpose:
 *     Provides a worker thread with its own Lua state for offloading
 *     heavy Lua computations from the main game thread.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef LUATHREAD_H
#define LUATHREAD_H

#include "globals.h"
#include "bflib_basics.h"
#include <lua.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Maximum number of completed results that can be queued before the oldest
 * are discarded. Keeps memory bounded if results are not polled.
 */
#define LUA_THREAD_MAX_RESULTS 64

/**
 * Maximum length of a Lua code string that can be submitted as a task.
 */
#define LUA_THREAD_MAX_CODE_LEN 4096

/**
 * Initialise the Lua worker thread and its private Lua state.
 * Call once after the main Lua state (Lvl_script) has been opened.
 * @return true on success, false on failure.
 */
TbBool lua_thread_init(void);

/**
 * Shut down the worker thread and free all resources.
 * Waits for the worker to finish its current task before returning.
 * Call before closing the main Lua state.
 */
void lua_thread_shutdown(void);

/**
 * Submit a Lua code string for execution on the worker thread.
 * The code is copied internally; the caller may free it after this call.
 *
 * @param task_id  Caller-chosen identifier echoed back in the result.
 * @param code     Lua source code to execute on the worker thread.
 * @return true if the task was enqueued, false if the queue is full or
 *         the worker is not running.
 */
TbBool lua_thread_submit(int task_id, const char *code);

/**
 * Poll for a completed result from the worker thread.
 * This is meant to be called from the main thread (e.g. once per game tick).
 *
 * @param[out] task_id   Set to the task_id that was submitted.
 * @param[out] success   Set to true if the Lua code ran without error.
 * @param[out] result    Points to a static buffer with the result string
 *                       (the return value of the Lua code converted to string,
 *                        or the error message on failure).  Valid until the
 *                        next call to lua_thread_poll.
 * @return true if a result was available, false if no result is ready.
 */
TbBool lua_thread_poll(int *task_id, TbBool *success, const char **result);

/**
 * Return whether the worker thread is currently idle (no pending tasks).
 */
TbBool lua_thread_is_idle(void);

/**
 * Register the Lua-side helper functions (RunAsync, PollAsyncResult, etc.)
 * into the given Lua state.  Called from reg_host_functions().
 */
void lua_thread_register_functions(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif /* LUATHREAD_H */
