#include "pre_inc.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <SDL2/SDL.h>
#include <string.h>
#include <stdlib.h>

#include "lua_thread.h"
#include "bflib_basics.h"

#include "post_inc.h"

/* ------------------------------------------------------------------ */
/*  Internal data structures                                          */
/* ------------------------------------------------------------------ */

/** A single task submitted to the worker thread. */
typedef struct LuaTask {
    int             task_id;
    char            code[LUA_THREAD_MAX_CODE_LEN];
    struct LuaTask *next;
} LuaTask;

/** A completed result returned by the worker thread. */
typedef struct LuaResult {
    int              task_id;
    TbBool           success;
    char             message[LUA_THREAD_MAX_CODE_LEN];
    struct LuaResult *next;
} LuaResult;

/** Thread-safe queue (singly-linked list with mutex + condition). */
typedef struct TaskQueue {
    LuaTask    *head;
    LuaTask    *tail;
    int         count;
    SDL_mutex  *mutex;
    SDL_cond   *cond;       /**< signalled when a new task is enqueued */
} TaskQueue;

typedef struct ResultQueue {
    LuaResult  *head;
    LuaResult  *tail;
    int         count;
    SDL_mutex  *mutex;
} ResultQueue;

/* ------------------------------------------------------------------ */
/*  Module-level state                                                */
/* ------------------------------------------------------------------ */

static lua_State   *worker_lua   = NULL;
static SDL_Thread  *worker_thread = NULL;
static TaskQueue    task_queue   = {0};
static ResultQueue  result_queue = {0};
static SDL_atomic_t worker_running = {0};

/* ------------------------------------------------------------------ */
/*  Queue helpers                                                     */
/* ------------------------------------------------------------------ */

static void task_queue_init(TaskQueue *q)
{
    q->head  = NULL;
    q->tail  = NULL;
    q->count = 0;
    q->mutex = SDL_CreateMutex();
    q->cond  = SDL_CreateCond();
}

static void task_queue_destroy(TaskQueue *q)
{
    SDL_LockMutex(q->mutex);
    LuaTask *t = q->head;
    while (t) {
        LuaTask *next = t->next;
        free(t);
        t = next;
    }
    q->head = q->tail = NULL;
    q->count = 0;
    SDL_UnlockMutex(q->mutex);
    SDL_DestroyCond(q->cond);
    SDL_DestroyMutex(q->mutex);
    q->mutex = NULL;
    q->cond  = NULL;
}

static TbBool task_queue_push(TaskQueue *q, int task_id, const char *code)
{
    LuaTask *t = (LuaTask *)malloc(sizeof(LuaTask));
    if (!t)
        return false;
    t->task_id = task_id;
    strncpy(t->code, code, LUA_THREAD_MAX_CODE_LEN - 1);
    t->code[LUA_THREAD_MAX_CODE_LEN - 1] = '\0';
    t->next = NULL;

    SDL_LockMutex(q->mutex);
    if (q->tail)
        q->tail->next = t;
    else
        q->head = t;
    q->tail = t;
    q->count++;
    SDL_CondSignal(q->cond);
    SDL_UnlockMutex(q->mutex);
    return true;
}

/** Pop a task; blocks until one is available or the worker is stopped. */
static LuaTask *task_queue_pop(TaskQueue *q)
{
    SDL_LockMutex(q->mutex);
    while (q->head == NULL && SDL_AtomicGet(&worker_running))
        SDL_CondWaitTimeout(q->cond, q->mutex, 100);
    LuaTask *t = q->head;
    if (t) {
        q->head = t->next;
        if (!q->head)
            q->tail = NULL;
        q->count--;
    }
    SDL_UnlockMutex(q->mutex);
    return t;
}

static void result_queue_init(ResultQueue *q)
{
    q->head  = NULL;
    q->tail  = NULL;
    q->count = 0;
    q->mutex = SDL_CreateMutex();
}

static void result_queue_destroy(ResultQueue *q)
{
    SDL_LockMutex(q->mutex);
    LuaResult *r = q->head;
    while (r) {
        LuaResult *next = r->next;
        free(r);
        r = next;
    }
    q->head = q->tail = NULL;
    q->count = 0;
    SDL_UnlockMutex(q->mutex);
    SDL_DestroyMutex(q->mutex);
    q->mutex = NULL;
}

static void result_queue_push(ResultQueue *q, int task_id, TbBool success,
                              const char *msg)
{
    LuaResult *r = (LuaResult *)malloc(sizeof(LuaResult));
    if (!r)
        return;
    r->task_id = task_id;
    r->success = success;
    if (msg) {
        strncpy(r->message, msg, sizeof(r->message) - 1);
        r->message[sizeof(r->message) - 1] = '\0';
    } else {
        r->message[0] = '\0';
    }
    r->next = NULL;

    SDL_LockMutex(q->mutex);
    /* Drop oldest if we are at capacity. */
    while (q->count >= LUA_THREAD_MAX_RESULTS && q->head) {
        LuaResult *old = q->head;
        q->head = old->next;
        if (!q->head)
            q->tail = NULL;
        q->count--;
        free(old);
    }
    if (q->tail)
        q->tail->next = r;
    else
        q->head = r;
    q->tail = r;
    q->count++;
    SDL_UnlockMutex(q->mutex);
}

static LuaResult *result_queue_pop(ResultQueue *q)
{
    SDL_LockMutex(q->mutex);
    LuaResult *r = q->head;
    if (r) {
        q->head = r->next;
        if (!q->head)
            q->tail = NULL;
        q->count--;
    }
    SDL_UnlockMutex(q->mutex);
    return r;
}

/* ------------------------------------------------------------------ */
/*  Worker thread entry point                                         */
/* ------------------------------------------------------------------ */

static int lua_worker_thread_func(void *data)
{
    (void)data;
    SYNCDBG(5, "Lua worker thread started");

    while (SDL_AtomicGet(&worker_running)) {
        LuaTask *task = task_queue_pop(&task_queue);
        if (!task)
            continue;

        /* Execute the code in the worker's private Lua state. */
        int res = luaL_dostring(worker_lua, task->code);
        const char *msg = NULL;
        TbBool ok = true;
        if (res != LUA_OK) {
            ok = false;
            msg = lua_tostring(worker_lua, -1);
            ERRORLOG("Lua worker error (task %d): %s", task->task_id,
                     msg ? msg : "Unknown error");
            lua_pop(worker_lua, 1);
        } else {
            /* If the code pushed a return value, capture it. */
            if (lua_gettop(worker_lua) > 0) {
                msg = lua_tostring(worker_lua, -1); /* may be NULL */
                lua_settop(worker_lua, 0);
            }
        }

        result_queue_push(&result_queue, task->task_id, ok, msg);
        free(task);
    }

    SYNCDBG(5, "Lua worker thread exiting");
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Public API                                                        */
/* ------------------------------------------------------------------ */

TbBool lua_thread_init(void)
{
    if (worker_thread) {
        WARNLOG("Lua worker thread already initialised");
        return true;
    }

    /* Create a private Lua state for the worker. */
    worker_lua = luaL_newstate();
    if (!worker_lua) {
        ERRORLOG("Failed to create worker Lua state");
        return false;
    }
    luaL_openlibs(worker_lua);

    task_queue_init(&task_queue);
    result_queue_init(&result_queue);

    SDL_AtomicSet(&worker_running, 1);
    worker_thread = SDL_CreateThread(lua_worker_thread_func,
                                     "lua_worker", NULL);
    if (!worker_thread) {
        ERRORLOG("Failed to create Lua worker thread: %s", SDL_GetError());
        lua_close(worker_lua);
        worker_lua = NULL;
        task_queue_destroy(&task_queue);
        result_queue_destroy(&result_queue);
        return false;
    }

    SYNCDBG(3, "Lua worker thread initialised");
    return true;
}

void lua_thread_shutdown(void)
{
    if (!worker_thread)
        return;

    /* Signal the worker to stop and wake it. */
    SDL_AtomicSet(&worker_running, 0);
    SDL_LockMutex(task_queue.mutex);
    SDL_CondSignal(task_queue.cond);
    SDL_UnlockMutex(task_queue.mutex);

    SDL_WaitThread(worker_thread, NULL);
    worker_thread = NULL;

    if (worker_lua) {
        lua_close(worker_lua);
        worker_lua = NULL;
    }

    task_queue_destroy(&task_queue);
    result_queue_destroy(&result_queue);

    SYNCDBG(3, "Lua worker thread shut down");
}

TbBool lua_thread_submit(int task_id, const char *code)
{
    if (!worker_thread || !SDL_AtomicGet(&worker_running)) {
        ERRORLOG("Lua worker thread is not running");
        return false;
    }
    if (!code || code[0] == '\0') {
        ERRORLOG("Empty Lua code submitted to worker");
        return false;
    }
    return task_queue_push(&task_queue, task_id, code);
}

/* Static buffer used by lua_thread_poll to hand the result string back. */
static char poll_result_buf[LUA_THREAD_MAX_CODE_LEN];

TbBool lua_thread_poll(int *task_id, TbBool *success, const char **result)
{
    LuaResult *r = result_queue_pop(&result_queue);
    if (!r)
        return false;
    if (task_id)
        *task_id = r->task_id;
    if (success)
        *success = r->success;
    strncpy(poll_result_buf, r->message, sizeof(poll_result_buf) - 1);
    poll_result_buf[sizeof(poll_result_buf) - 1] = '\0';
    if (result)
        *result = poll_result_buf;
    free(r);
    return true;
}

TbBool lua_thread_is_idle(void)
{
    if (!worker_thread)
        return true;
    SDL_LockMutex(task_queue.mutex);
    int pending = task_queue.count;
    SDL_UnlockMutex(task_queue.mutex);
    return pending == 0;
}

/* ------------------------------------------------------------------ */
/*  Lua-facing helper functions                                       */
/* ------------------------------------------------------------------ */

/**
 * RunAsync(task_id, code_string)
 * Submit a piece of Lua code for execution on the worker thread.
 * The worker has its own Lua state with only the standard libraries;
 * it cannot access game objects directly.  Use it for pure computation.
 *
 * @param task_id  integer identifier for matching results later
 * @param code     Lua source code string
 * @return boolean  true if enqueued successfully
 */
static int l_RunAsync(lua_State *L)
{
    int task_id = (int)luaL_checkinteger(L, 1);
    const char *code = luaL_checkstring(L, 2);
    TbBool ok = lua_thread_submit(task_id, code);
    lua_pushboolean(L, ok);
    return 1;
}

/**
 * PollAsyncResult()
 * Check whether a result from the worker thread is available.
 *
 * @return task_id, success, result_string   or  nil if nothing ready
 */
static int l_PollAsyncResult(lua_State *L)
{
    int task_id;
    TbBool success;
    const char *result;
    if (lua_thread_poll(&task_id, &success, &result)) {
        lua_pushinteger(L, task_id);
        lua_pushboolean(L, success);
        lua_pushstring(L, result);
        return 3;
    }
    lua_pushnil(L);
    return 1;
}

/**
 * IsAsyncIdle()
 * @return boolean  true if the worker has no pending tasks
 */
static int l_IsAsyncIdle(lua_State *L)
{
    lua_pushboolean(L, lua_thread_is_idle());
    return 1;
}

void lua_thread_register_functions(lua_State *L)
{
    lua_pushcfunction(L, l_RunAsync);
    lua_setglobal(L, "RunAsync");

    lua_pushcfunction(L, l_PollAsyncResult);
    lua_setglobal(L, "PollAsyncResult");

    lua_pushcfunction(L, l_IsAsyncIdle);
    lua_setglobal(L, "IsAsyncIdle");
}
