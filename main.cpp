//#define IS_BETA_BUILD
#define IMPLEMENT_METAMETHODS

#define _UNUSED(n)
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"

#include "lua.hpp"
#include "luautil.h"

#include "gplugin.h"
#include "gfile.h"
#include "gstdio.h"
#include "ginput.h"
#include "application.h"
#include "luaapplication.h"

#define CUTE_C2_IMPLEMENTATION
#include "cute_c2.h"

#ifdef IS_BETA_BUILD
#define PLUGIN_NAME "CuteC2_beta"
#else
#define PLUGIN_NAME "CuteC2"
#endif

#define LUA_ASSERT(EXP, MSG) if (!(EXP)) { lua_pushstring(L, MSG); lua_error(L); }
#define LUA_ASSERTF(EXP, FMT, ...) if (!(EXP)) { lua_pushfstring(L, FMT, __VA_ARGS__); lua_error(L); }
#define LUA_THROW_ERROR(MSG) lua_pushstring(L, MSG); lua_error(L);
#define LUA_THROW_ERRORF(FMT, ...) lua_pushfstring(L, FMT, __VA_ARGS__); lua_error(L);
#define LUA_PRINTF(FMT, ...) lua_getglobal(L, "print"); lua_pushfstring(L, FMT, __VA_ARGS__); lua_call(L, 1, 0);
#define LUA_PRINT(MSG) lua_getglobal(L, "print"); lua_pushstring(L, MSG); lua_call(L, 1, 0);
#define BIND_IENUM(value, name) lua_pushinteger(L, value); lua_setfield(L, -2, name);
#define BIND_FENUM(value, name) lua_pushnumber(L, value); lua_setfield(L, -2, name);

#ifdef IMPLEMENT_METAMETHODS
#define ADD_METAMETHODS(name, index, newindex) \
    luaL_getmetatable(L, name); \
    lua_pushcfunction(L, index, #index); \
    lua_setfield(L, -2, "__index"); \
    lua_pushcfunction(L, newindex, #newindex); \
    lua_setfield(L, -2, "__newindex"); \
    lua_setmetatable(L, -2);
#else
#define ADD_METAMETHODS(L, index, newindex) (((void)(L))
#endif

static lua_State* L;
static Application* application;
static char keyWeak = ' ';
//static const std::string SHAPES[4] = {"c2Circle", "c2AABB", "c2Capsule", "c2Poly"};

namespace cute_c2_impl
{
////////////////////////////////////////////////////////////////////////////////
///
/// DEBUG TOOL
///
////////////////////////////////////////////////////////////////////////////////

static int DUMP_INDEX = 0;

static void stackDump(lua_State* L, const char* prefix = "")
{
    int i = lua_gettop(L);
    lua_getglobal(L, "print");
    lua_pushfstring(L, "----------------      %d      ----------------\n>%s\n----------------  Stack Dump ----------------", DUMP_INDEX, prefix);
    lua_call(L, 1, 0);
    while (i)
    {
        int t = lua_type(L, i);
        lua_getglobal(L, "print");
        switch (t)
        {
            case LUA_TSTRING: lua_pushfstring(L, "[S] %d:'%s'", i, lua_tostring(L, i));
            case LUA_TBOOLEAN: lua_pushfstring(L, "[B] %d: %s", i, lua_toboolean(L, i) ? "true" : "false");
            case LUA_TNUMBER: lua_pushfstring(L, "[N] %d: %f", i, lua_tonumber(L, i));
            case LUA_TVECTOR:
            {
                const float* vec = lua_tovector(L, i);
                lua_pushfstring(L, "[V] %d: {%f, %f, %f, %f}", i, vec[0], vec[1], vec[2], vec[3]);
            }
            break;
            default: lua_pushfstring(L, "[D] %d: %s", i, lua_typename(L, t));
        }
        i--;
        lua_call(L, 1, 0);
    }
    lua_getglobal(L, "print");
    lua_pushstring(L, "------------ Stack Dump Finished ------------\n");
    lua_call(L, 1, 0);

    DUMP_INDEX++;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// HELPERS
///
/////////////////////////////////////////////////////////////////////////////////////////////

void* getRawPtr(lua_State* L, int idx)
{
    lua_getfield(L, idx, "__userdata"); // get adress
    if (lua_isnil(L, -1) != 0)
    {
        lua_pop(L, 1);
        luaL_error(L, "index '__userdata' cannot be found");
    }
    void* ptr = *(void**)lua_touserdata(L, -1);
    lua_pop(L, 1);
    
    return ptr;
}

C2_TYPE getShapeType(lua_State* L, int idx)
{
    lua_rawgetfield(L, idx, "__shapeType");
    C2_TYPE shapeType = (C2_TYPE)lua_tointeger(L, -1);
    lua_pop(L, 1);
    return shapeType;
}

int lua_pushmanifold(lua_State* L, c2Manifold m)
{
// OLD
//    lua_pushinteger(L, m.count);

//    lua_pushnumber(L, m.depths[0]);
//    lua_pushnumber(L, m.depths[1]);

//    lua_pushnumber(L, m.contact_points[0].x);
//    lua_pushnumber(L, m.contact_points[0].y);
//    lua_pushnumber(L, m.contact_points[1].x);
//    lua_pushnumber(L, m.contact_points[1].y);

//    lua_pushnumber(L, m.n.x);
//    lua_pushnumber(L, m.n.y);
    
//    return 9;
// NEW
//  {count = value, depth = {value1, value2}, contact_points = {{x = value, y = value}, {x = value, y = value}}, normal = {x = value, y = value}}
    lua_createtable(L, 0, 4);                   // result_t
    lua_pushnumber(L, m.count);                 // result_t, number
    lua_setfield(L, -2, "count");               // result_t
    
    lua_createtable(L, 2, 0);                   // result_t, depths_t
    lua_pushnumber(L, m.depths[0]);             // result_t, depths_t, number
    lua_rawseti(L, -2, 1);                      // result_t, depths_t
    lua_pushnumber(L, m.depths[1]);             // result_t, depths_t, number
    lua_rawseti(L, -2, 2);                      // result_t, depths_t
    lua_setfield(L, -2, "depths");              // result_t
    
    lua_createtable(L, 2, 0);                   // result_t, cp_t
    
    lua_createtable(L, 0, 2);                   // result_t, cp_t, p1_t
    lua_pushnumber(L, m.contact_points[0].x);   // result_t, cp_t, p1_t, number
    lua_setfield(L, -2, "x");                   // result_t, cp_t, p1_t
    lua_pushnumber(L, m.contact_points[0].y);   // result_t, cp_t, p1_t, number
    lua_setfield(L, -2, "y");                   // result_t, cp_t, p1_t
    lua_rawseti(L, -2, 1);                      // result_t, cp_t
    
    lua_createtable(L, 0, 2);                   // result_t, cp_t, p2_t
    lua_pushnumber(L, m.contact_points[1].x);   // result_t, cp_t, p2_t, number
    lua_setfield(L, -2, "x");                   // result_t, cp_t, p2_t
    lua_pushnumber(L, m.contact_points[1].y);   // result_t, cp_t, p2_t, number
    lua_setfield(L, -2, "y");                   // result_t, cp_t, p2_t
    lua_rawseti(L, -2, 2);                      // result_t, cp_t
    lua_setfield(L, -2, "contact_points");      // result_t
    
    lua_createtable(L, 0, 2);                   // result_t, norm_t
    lua_pushnumber(L, m.n.x);                   // result_t, norm_t, number
    lua_setfield(L, -2, "x");                   // result_t, norm_t
    lua_pushnumber(L, m.n.y);                   // result_t, norm_t, number
    lua_setfield(L, -2, "y");                   // result_t, norm_t
    lua_setfield(L, -2, "normal");              // result_t
    
    return 1;
}

c2x checkTransform(lua_State* L, int idx);

void addMetaMethods(lua_State* L, const char* classname, const char* metaname, lua_CFunction index, lua_CFunction newindex)
{
    // https://stackoverflow.com/questions/20332518/lua-c-index-is-always-invoked-even-the-tables-field-is-known
    // Do NOT invoke "__index" and "__newindex" methods on KNOWN fields
    
    // Stack:
    // -1: ojbect
    // -2: argument_1
    // ...
    
    luaL_getmetatable(L, classname);
    // -1: mt
    // -2: ojbect
    
    lua_pushvalue(L, -1);
    // -1: mt
    // -2: mt
    // -3: ojbect
    
    lua_setfield(L, -2, "__newindex");
    // -1: mt
    // -2: ojbect
    
    // Small hack to be able to get object instance inside
    // "__index" & "__newindex" metamethods
    lua_getfield(L, -2, "__userdata");
    // -1: userdata
    // -2: mt
    // -3: ojbect
    
    lua_setfield(L, -2, "__userdata"); // hack
    // -1: mt
    // -2: ojbect
    
    luaL_newmetatable(L, metaname);
    // -1: new_mt
    // -2: mt
    // -3: object
    
    lua_pushcfunction(L, index, "debug_index");
    // -1: func
    // -2: new_mt
    // -3: mt
    // -4: object
    
    lua_setfield(L, -2, "__index");
    // -1: new_mt
    // -2: mt
    // -3: object
    
    lua_pushcfunction(L, newindex, "debug_newindex");
    // -1: func
    // -2: new_mt
    // -3: mt
    // -4: object
    
    lua_setfield(L, -2, "__newindex");
    // -1: new_mt
    // -2: mt
    // -3: object
    
    lua_setmetatable(L, -2);
    // -1: mt
    // -2: object
    
    lua_remove(L, -1);
    // -1: object
}

////////////////////////////////////////////////////////////////////////////////
///
/// TEMPLATES
///
////////////////////////////////////////////////////////////////////////////////

template<class T>
inline T* getPtr(lua_State* L, const char* name, int idx = 1)
{
    return static_cast<T*>(g_getInstance(L, name, idx));
}

template<class T>
inline T* getPtrNoCheck(lua_State* L, const char* name, int idx = 1)
{
    g_disableTypeChecking();
    T* obj = static_cast<T*>(g_getInstance(L, name, idx));
    g_enableTypeChecking();        
    return obj;
}

inline void setPtr(lua_State* L, void* ptr)
{
    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
    lua_pushvalue(L, -2);
    luaL_rawsetptr(L, -2, ptr);
    lua_pop(L, 1);
}

template<class T>
int objRayTest(lua_State* L, const char* classname, C2_TYPE obj_type)
{
    T* obj = getPtr<T>(L, classname, 1);
    c2Raycast out;
    
    if (lua_gettop(L) > 3)
    {
        c2Ray ray = c2Ray();
        ray.p = c2V(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
        ray.d = c2V(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
        ray.t = luaL_checknumber(L, 6);
        c2x bx = checkTransform(L, 7);
        
        int result = c2CastRay(ray, obj, &bx, obj_type, &out);
        
        lua_pushinteger(L, result);
        lua_pushnumber(L, out.n.x);
        lua_pushnumber(L, out.n.y);
        lua_pushnumber(L, out.t);
        return 4;
        
    }
    c2Ray A = *getPtr<c2Ray>(L, "c2Ray", 2);
    c2x bx = checkTransform(L, 3);
    
    int result = c2CastRay(A, obj, &bx, obj_type, &out);
    
    lua_pushinteger(L, result);
    lua_pushnumber(L, out.n.x);
    lua_pushnumber(L, out.n.y);
    lua_pushnumber(L, out.t);
    return 4;
}

template<class T>
void objInflate(lua_State* L, const char* classname, C2_TYPE obj_type)
{
    T* circle = getPtr<T>(L, classname, 1);
    float skin_factor = luaL_checknumber(L, 2);
    c2Inflate(circle, obj_type, skin_factor);
}

/////

c2x checkTransform(lua_State* L, int idx)
{
    if (lua_isnil(L, idx))
        return c2xIdentity();
    if (lua_gettop(L) > idx - 1)
    {
        return *getPtr<c2x>(L, "c2x", idx);
    }
    return c2xIdentity();
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// ENUMS
///
/////////////////////////////////////////////////////////////////////////////////////////////

void bindEnums(lua_State* L)
{
    lua_getglobal(L, "CuteC2");

    BIND_IENUM(C2_TYPE_NONE, "TYPE_NONE");
    BIND_IENUM(C2_TYPE_CIRCLE, "TYPE_CIRCLE");
    BIND_IENUM(C2_TYPE_AABB, "TYPE_AABB");
    BIND_IENUM(C2_TYPE_CAPSULE, "TYPE_CAPSULE");
    BIND_IENUM(C2_TYPE_POLY, "TYPE_POLY");

    lua_pop(L, 1);
}


/////////////////////////////////////////////////////////////////////////////////////////////
///
/// Shapes
///
/////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// CIRCLE
///
/////////////////////////////////////////////////////////////////////////////////////////////

// c2Circle

static int cirlceIndex(lua_State* L) // __index
{
    const char* key = lua_tostring(L, 2);
    
    if (strcmp(key, "radius") == 0)
    {
        c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
        lua_pushnumber(L, circle->r);
        return 1;
    }
    else if (strcmp(key, "x") == 0)
    {
        c2Circle* circle = getPtrNoCheck<c2Circle>(L, "c2Circle", 1);
        lua_pushnumber(L, circle->p.x);
        return 1;
    }
    else if (strcmp(key, "y") == 0)
    {
        c2Circle* circle = getPtrNoCheck<c2Circle>(L, "c2Circle", 1);
        lua_pushnumber(L, circle->p.y);
        return 1;
    }
    
    lua_getmetatable(L, -2);
    lua_rawgetfield(L, -1, key);
    return 1;
}

static int cirlceNewIndex(lua_State* L) // __newindex
{
    const char* key = lua_tostring(L, 2);
    float value = lua_tonumber(L, 3);
    
    if (strcmp(key, "radius") == 0)
    {
        c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
        circle->r = value;
        return 0;
    }
    else if (strcmp(key, "x") == 0)
    {
        c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
        circle->p.x = value;
        return 0;
    }
    else if (strcmp(key, "y") == 0)
    {
        c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
        circle->p.y = value;
        return 0;
    }
    
    lua_rawset(L, 1);
    return 0;
}

int createCircle(lua_State* L)
{
    c2Circle* circle = new c2Circle();
    circle->p = c2V(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
    circle->r = luaL_checknumber(L, 3);
    g_pushInstance(L, "c2Circle", circle);
    
    // Store shape directly in table to be able to
    // use global raycast & inflate functions (e.g. CuteC2.raycast(myShape, myRay))
    lua_pushinteger(L, C2_TYPE_CIRCLE);
    lua_setfield(L, -2, "__shapeType");
    
    ADD_METAMETHODS("c2Circle", cirlceIndex, cirlceNewIndex);
    setPtr(L, circle);
    
    return 1;
}

int setCirclePosition(lua_State* L)
{
    c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
    circle->p.x = luaL_checknumber(L, 2);
    circle->p.y = luaL_checknumber(L, 3);
    return 0;
}

int getCirclePosition(lua_State* L)
{
    c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
    lua_pushnumber(L, circle->p.x );
    lua_pushnumber(L, circle->p.y );
    return 2;
}

int setCircleRadius(lua_State* L)
{
    c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
    circle->r = luaL_checknumber(L, 2);
    return 0;
}

int getCircleRadius(lua_State* L)
{
    c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
    lua_pushnumber(L, circle->r);
    return 1;
}

int cirlceGetData(lua_State* L)
{
    c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
    lua_pushnumber(L, circle->p.x);
    lua_pushnumber(L, circle->p.y);
    lua_pushnumber(L, circle->r);
    return 3;
}

int getCirlceBoundingBox(lua_State* L)
{
    c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
    float size = circle->r * 2.0f;
    
    lua_pushnumber(L, circle->p.x - circle->r);
    lua_pushnumber(L, circle->p.y - circle->r);
    lua_pushnumber(L, circle->p.x + circle->r);
    lua_pushnumber(L, circle->p.x + circle->r);
    lua_pushnumber(L, size);
    lua_pushnumber(L, size);
    return 6;
}

int cirlceRayTest(lua_State* L)
{
    return objRayTest<c2Circle>(L, "c2Circle", C2_TYPE_CIRCLE);
}

int circleInflate(lua_State* L)
{
    objInflate<c2Circle>(L, "c2Circle", C2_TYPE_CIRCLE);
    return 0;
}

int circleHitTest(lua_State* L)
{
    c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
    c2v point = c2V(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    int result = c2CircleToPoint(*circle, point);
    lua_pushboolean(L, result);
    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// AABB
///
/////////////////////////////////////////////////////////////////////////////////////////////

// c2AABB

static int AABBIndex(lua_State* L) // __index
{
    const char* key = lua_tostring(L, 2);
    
    if (strcmp(key, "width") == 0)
    {
        c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
        float w = aabb->max.x - aabb->min.x;
        lua_pushnumber(L, w);
        return 1;
    }
    else if (strcmp(key, "height") == 0)
    {
        c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
        float h = aabb->max.y - aabb->min.y;
        lua_pushnumber(L, h);
        return 1;
    }
    else if (strcmp(key, "x") == 0)
    {
        c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
        lua_pushnumber(L, aabb->min.x);
        return 1;
    }
    else if (strcmp(key, "y") == 0)
    {
        c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
        lua_pushnumber(L, aabb->min.y);
        return 1;
    }
    
    lua_getmetatable(L, -2);
    lua_rawgetfield(L, -1, key);
    return 1;
}

static int AABBNewIndex(lua_State* L) // __newindex
{
    const char* key = lua_tostring(L, 2);
    float value = lua_tonumber(L, 3);
    
    if (strcmp(key, "width") == 0)
    {
        c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
        aabb->max.x = aabb->min.x + value;
        return 0;
    }
    else if (strcmp(key, "height") == 0)
    {
        c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
        aabb->max.y = aabb->min.y + value;
        return 0;
    }
    else if (strcmp(key, "x") == 0)
    {
        c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
        float w = aabb->max.x - aabb->min.x;
        aabb->min.x = value;
        aabb->max.x = value + w;
        return 0;
    }
    else if (strcmp(key, "y") == 0)
    {
        c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
        float h = aabb->max.y - aabb->min.y;
        aabb->min.y = value;
        aabb->max.y = value + h;
        return 0;
    }
    
    lua_rawset(L, 1);
    return 0;
}

int createAABB(lua_State* L)
{
    c2AABB* aabb = new c2AABB();
    aabb->min = c2V(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
    aabb->max = c2Add(aabb->min, c2V(luaL_checknumber(L, 3), luaL_checknumber(L, 4)));
    g_pushInstance(L, "c2AABB", aabb);

    lua_pushinteger(L, C2_TYPE_AABB);
    lua_setfield(L, -2, "__shapeType");
    
    ADD_METAMETHODS("c2AABB", AABBIndex, AABBNewIndex);
    setPtr(L, aabb);

    return 1;
}

int setAABBPosition(lua_State* L)
{
    c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
    float w = aabb->max.x - aabb->min.x;
    float h = aabb->max.y - aabb->min.y;
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    aabb->min.x = x;
    aabb->min.y = y;
    aabb->max.x = x + w;
    aabb->max.y = y + h;
    return 0;
}

int getAABBPosition(lua_State* L)
{
    c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
    lua_pushnumber(L, aabb->min.x);
    lua_pushnumber(L, aabb->min.y);
    return 2;
}

int setAABBSize(lua_State* L)
{
    c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
    float w = luaL_checknumber(L, 2);
    float h = luaL_checknumber(L, 3);
    aabb->max.x = aabb->min.x + w;
    aabb->max.y = aabb->min.y + h;
    return 0;
}

int getAABBSize(lua_State* L)
{
    c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
    lua_pushnumber(L, aabb->max.x - aabb->min.x);
    lua_pushnumber(L, aabb->max.y - aabb->min.y);
    return 2;
}

int getAABBData(lua_State* L)
{
    c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
    lua_pushnumber(L, aabb->min.x);
    lua_pushnumber(L, aabb->min.y);
    lua_pushnumber(L, aabb->max.x);
    lua_pushnumber(L, aabb->max.y);
    return 4;
}

int getAABBboundingBox(lua_State* L)
{
    c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);    
    lua_pushnumber(L, aabb->min.x);
    lua_pushnumber(L, aabb->min.y);
    lua_pushnumber(L, aabb->max.x);
    lua_pushnumber(L, aabb->max.y);
    lua_pushnumber(L, aabb->max.x - aabb->min.x);
    lua_pushnumber(L, aabb->max.y - aabb->min.y);
    return 6;
}

int AABBRayTest(lua_State* L)
{
    return objRayTest<c2AABB>(L, "c2AABB", C2_TYPE_AABB);
}

int AABBInflate(lua_State* L)
{
    objInflate<c2AABB>(L, "c2AABB", C2_TYPE_AABB);
    return 0;
}

int AABBHitTest(lua_State* L)
{
    c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
    c2v point = c2V(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    int result = c2AABBtoPoint(*aabb, point);
    lua_pushboolean(L, result);
    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// CAPSULE
///
/////////////////////////////////////////////////////////////////////////////////////////////

// c2Capsule

static int capsuleIndex(lua_State* L) // __index
{
    const char* key = lua_tostring(L, 2);    
    
    if (strcmp(key, "radius") == 0)
    {
        c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
        lua_pushnumber(L, capsule->r);
        return 1;
    }
    else if (strcmp(key, "height") == 0)
    {
        c2Capsule* capsule = getPtrNoCheck<c2Capsule>(L, "c2Capsule", 1);
        float h = capsule->b.y - capsule->a.y;
        lua_pushnumber(L, h);
        return 1;
    }
    else if (strcmp(key, "x") == 0)
    {
        c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
        lua_pushnumber(L, capsule->a.x);
        return 1;
    }
    else if (strcmp(key, "y") == 0)
    {
        c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
        lua_pushnumber(L, capsule->a.y);
        return 1;
    }
    
    lua_getmetatable(L, -2);
    lua_rawgetfield(L, -1, key);
    return 1;
}

static int capsuleNewIndex(lua_State* L) // __newindex
{
    const char* key = lua_tostring(L, 2);
    float value = lua_tonumber(L, 3);
    
    if (strcmp(key, "radius") == 0)
    {
        c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
        capsule->r = value;
        return 0;
    }
    else if (strcmp(key, "height") == 0)
    {
        c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
        capsule->b.y = capsule->a.y + value;
        return 0;
    }
    else if (strcmp(key, "x") == 0)
    {
        c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
        capsule->a.x = value;
        capsule->b.x = value;
        return 0;
    }
    else if (strcmp(key, "y") == 0)
    {
        c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
        float h = capsule->b.y - capsule->a.y;
        capsule->a.y = value;
        capsule->b.y = value + h;
        return 0;
    }
    
    lua_rawset(L, 1);
    return 0;
}

int createCapsule(lua_State* L)
{
    c2Capsule* capsule = new c2Capsule();
    c2v pos = c2V(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
    float h = luaL_checknumber(L, 3);
    capsule->a = pos;
    capsule->b = c2V(pos.x, pos.y + h);
    capsule->r = luaL_checknumber(L, 4);
    g_pushInstance(L, "c2Capsule", capsule);

    lua_pushinteger(L, C2_TYPE_CAPSULE);
    lua_setfield(L, -2, "__shapeType");
    
    ADD_METAMETHODS("c2Capsule", capsuleIndex, capsuleNewIndex);
    setPtr(L, capsule);

    return 1;
}

int setCapsulePosition(lua_State* L)
{
    c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    float h = capsule->b.y - capsule->a.y;
    capsule->a.x = x;
    capsule->a.y = y;
    capsule->b.x = x;
    capsule->b.y = y + h;
    return 0;
}

int getCapsulePosition(lua_State* L)
{
    c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
    lua_pushnumber(L, capsule->a.x);
    lua_pushnumber(L, capsule->a.y);
    return 2;
}

int setCapsuleHeight(lua_State* L)
{
    c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
    float h = luaL_checknumber(L, 2);
    capsule->b.y = capsule->a.y + h;
    return 0;
}

int getCapsuleHeight(lua_State* L)
{
    c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
    lua_pushnumber(L, capsule->b.y - capsule->a.y);
    return 1;
}

int setCapsuleRadius(lua_State* L)
{
    c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
    capsule->r = luaL_checknumber(L, 2);
    return 0;
}

int getCapsuleRadius(lua_State* L)
{
    c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
    lua_pushnumber(L, capsule->r);
    return 1;
}

int getCapsuleBoundingBox(lua_State* L)
{
    c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
    float size = capsule->r * 2.0f;
    
    lua_pushnumber(L, capsule->a.x - capsule->r);
    lua_pushnumber(L, capsule->a.y - capsule->r);
    lua_pushnumber(L, capsule->b.x + capsule->r);
    lua_pushnumber(L, capsule->b.y + capsule->r);
    lua_pushnumber(L, size);
    lua_pushnumber(L, capsule->b.y - capsule->a.y + size);
    return 6;
}

int getCapsuleData(lua_State* L)
{
    c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
    lua_pushnumber(L, capsule->a.x);
    lua_pushnumber(L, capsule->a.y);
    lua_pushnumber(L, capsule->b.x);
    lua_pushnumber(L, capsule->b.y);
    lua_pushnumber(L, capsule->r);
    return 5;
}

int capsuleRayTest(lua_State* L)
{
    return objRayTest<c2Capsule>(L, "c2Capsule", C2_TYPE_CAPSULE);
}

int capsuleInflate(lua_State* L)
{
    objInflate<c2Capsule>(L, "c2Capsule", C2_TYPE_CAPSULE);
    return 0;
}

int capsuleHitTest(lua_State* L)
{
    c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
    c2v point = c2V(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    c2AABB aabb = c2AABB();
    aabb.min = c2V(capsule->a.x - capsule->r, capsule->a.y);
    aabb.max = c2V(capsule->a.x + capsule->r, capsule->b.y);
    
    if (c2AABBtoPoint(aabb, point))
    {
        lua_pushboolean(L, true);
        return 1;
    }
    
    c2Circle circleA = c2Circle();
    circleA.p = capsule->a;
    circleA.r = capsule->r;
    
    if (c2CircleToPoint(circleA, point))
    {
        lua_pushboolean(L, true);
        return 1;
    }
    
    c2Circle circleB = c2Circle();
    circleB.p = capsule->b;
    circleB.r = capsule->r;
    
    if (c2CircleToPoint(circleB, point))
    {
        lua_pushboolean(L, true);
        return 1;
    }
    
    lua_pushboolean(L, false);
    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// POLYGON
///
/////////////////////////////////////////////////////////////////////////////////////////////


void updatePointsInternal(lua_State* L, int idx, c2Poly* poly)
{
    luaL_checktype(L, idx, LUA_TTABLE);
    int l = lua_objlen(L, idx);
    LUA_ASSERT(l % 2 == 0, "Incorrect points table size");
    l = c2Min(l, C2_MAX_POLYGON_VERTS * 2);
    poly->count = l / 2;

    for (int i = 0, j = 0; i < l; i += 2, ++j)
    {
        lua_rawgeti(L, idx, i + 1);
        float x = luaL_checknumber(L, -1);
        lua_pop(L, 1);
        lua_rawgeti(L, idx, i + 2);
        float y = luaL_checknumber(L, -1);
        lua_pop(L, 1);
        poly->verts[j] = c2V(x, y);
    }
    c2MakePoly(poly);
}

void getBoundingBoxInternal(c2Poly* poly, c2x& transform, c2v& min, c2v& max)
{
    for (int i = 0; i < poly->count; i++)
    {
        c2v pt = c2Add(transform.p, poly->verts[i]);
        min.x = c2Min(min.x, pt.x);
        min.y = c2Min(min.y, pt.y);
        max.x = c2Max(max.x, pt.x);
        max.y = c2Max(max.y, pt.y);
    }
}

// c2Poly
int createPoly(lua_State* L)
{
    c2Poly* poly = new c2Poly();
    updatePointsInternal(L, 1, poly);
    g_pushInstance(L, "c2Poly", poly);
    
    lua_pushinteger(L, C2_TYPE_POLY);
    lua_setfield(L, -2, "__shapeType");
    
    setPtr(L, poly);
    return 1;
}

int updatePoints(lua_State* L)
{
    c2Poly* poly = getPtr<c2Poly>(L, "c2Poly", 1);
    updatePointsInternal(L, 2, poly);
    return 0;
}

int updatePoint(lua_State* L)
{
    c2Poly* poly = getPtr<c2Poly>(L, "c2Poly", 1);
    int index = luaL_checkinteger(L, 2) - 1;
    index = c2Min(index, C2_MAX_POLYGON_VERTS);
    // Update position
    poly->verts[index].x = luaL_checknumber(L, 3);
    poly->verts[index].y = luaL_checknumber(L, 4);

    // Update normal
    int next = index + 1 < poly->count ? index + 1 : 0;
    c2v e = c2Sub(poly->verts[next], poly->verts[index]);
    poly->norms[index] = c2Norm(c2CCW90(e));
    return 0;
}

int getPoints(lua_State* L)
{
    c2Poly* poly = getPtr<c2Poly>(L, "c2Poly", 1);
    lua_createtable(L, poly->count * 2, 0);
    for (int i = 0; i < poly->count; i++)
    {
        lua_pushnumber(L, poly->verts[i].x);
        lua_rawseti(L, 2, i * 2 + 1);

        lua_pushnumber(L, poly->verts[i].y);
        lua_rawseti(L, 2, i * 2 + 2);
    }
    return 1;
}

int getPolyBoundingBox(lua_State* L)
{
    c2Poly* poly = getPtr<c2Poly>(L, "c2Poly", 1);
    c2x transform = checkTransform(L, 2);
    
    c2v min = c2V(FLT_MAX, FLT_MAX);
    c2v max = c2V(0.0f, 0.0f);
    
    getBoundingBoxInternal(poly, transform, min, max);
    
    lua_pushnumber(L, min.x);
    lua_pushnumber(L, min.y);
    lua_pushnumber(L, max.x);
    lua_pushnumber(L, max.y);
    lua_pushnumber(L, max.x - min.x);
    lua_pushnumber(L, max.y - min.y);
    
    return 6;
}

int polyGetData(lua_State* L)
{
    LUA_THROW_ERROR("NOT IMPLEMENTED YET");
    // TODO
    return 0;
}

int polyRayTest(lua_State* L)
{
    return objRayTest<c2Poly>(L, "c2Poly", C2_TYPE_POLY);
}

int polyInflate(lua_State* L)
{
    objInflate<c2Poly>(L, "c2Poly", C2_TYPE_POLY);
    return 0;
}

int polyHitTest(lua_State* L)
{
    //LUA_THROW_ERROR("NOT IMPLEMENTED YET");
    // TODO
    c2Poly* poly = getPtr<c2Poly>(L, "c2Poly", 1);
    c2v point = c2V(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    c2x transform = checkTransform(L, 4);
     
    c2AABB aabb = c2AABB();
    aabb.min = c2V(FLT_MAX, FLT_MAX);
    
    getBoundingBoxInternal(poly, transform, aabb.min, aabb.max);
    
    lua_pushboolean(L, c2AABBtoPoint(aabb, point));
    
    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// RAY
///
/////////////////////////////////////////////////////////////////////////////////////////////

// c2Ray

static int rayIndex(lua_State* L) // __index
{
    const char* key = lua_tostring(L, 2);    
    
    if (strcmp(key, "x") == 0)
    {
        c2Ray* ray = getPtrNoCheck<c2Ray>(L, "c2Ray", 1);
        lua_pushnumber(L, ray->p.x);
        return 1;
    }
    else if (strcmp(key, "y") == 0)
    {
        c2Ray* ray = getPtrNoCheck<c2Ray>(L, "c2Ray", 1);
        lua_pushnumber(L, ray->p.y);
        return 1;
    }
    else if (strcmp(key, "t") == 0)
    {
        c2Ray* ray = getPtrNoCheck<c2Ray>(L, "c2Ray", 1);
        lua_pushnumber(L, ray->t);
        return 1;
    }
    else if (strcmp(key, "direction") == 0)
    {
        c2Ray* ray = getPtrNoCheck<c2Ray>(L, "c2Ray", 1);
        lua_pushnumber(L, atan2(ray->p.y, ray->p.x));
        return 1;
    }
    else if (strcmp(key, "targetX") == 0)
    {
        c2Ray* ray = getPtrNoCheck<c2Ray>(L, "c2Ray", 1);
        lua_pushnumber(L, ray->d.x);
        return 1;
    }
    else if (strcmp(key, "targetY") == 0)
    {
        c2Ray* ray = getPtrNoCheck<c2Ray>(L, "c2Ray", 1);
        lua_pushnumber(L, ray->d.y);
        return 1;
    }
    
    lua_getmetatable(L, -2);
    lua_rawgetfield(L, -1, key);
    return 1;
}

static int rayNewIndex(lua_State* L) // __newindex
{
    const char* key = lua_tostring(L, 2);
    float value = lua_tonumber(L, 3);
    
    if (strcmp(key, "x") == 0)
    {
        c2Ray* ray = getPtrNoCheck<c2Ray>(L, "c2Ray", 1);
        ray->p.x = value;
        return 0;
    }
    else if (strcmp(key, "y") == 0)
    {
        c2Ray* ray = getPtrNoCheck<c2Ray>(L, "c2Ray", 1);
        ray->p.y = value;
        return 0;
    }
    else if (strcmp(key, "t") == 0)
    {
        c2Ray* ray = getPtrNoCheck<c2Ray>(L, "c2Ray", 1);
        ray->t = value;
        return 0;
    }
    else if (strcmp(key, "direction") == 0)
    {
        c2Ray* ray = getPtrNoCheck<c2Ray>(L, "c2Ray", 1);
        ray->p.x = cosf(value);
        ray->p.y = sinf(value);
        return 0;
    }
    else if (strcmp(key, "targetX") == 0)
    {
        c2Ray* ray = getPtrNoCheck<c2Ray>(L, "c2Ray", 1);
        ray->d.x = value;
        return 0;
    }
    else if (strcmp(key, "targetY") == 0)
    {
        c2Ray* ray = getPtrNoCheck<c2Ray>(L, "c2Ray", 1);
        ray->d.y = value;
        return 0;
    }
    
    lua_rawset(L, 1);
    return 0;
}

int createRay(lua_State* L)
{
    c2Ray* ray = new c2Ray();
    ray->p = c2V(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
    ray->d = c2V(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ray->t = luaL_optnumber(L, 5, 1.0f);
    g_pushInstance(L, "c2Ray", ray);
    
    ADD_METAMETHODS("c2Ray", rayIndex, rayNewIndex);
    setPtr(L, ray);

    return 1;
}

int rayGetStartPosition(lua_State* L)
{
    c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
    lua_pushnumber(L, ray->p.x);
    lua_pushnumber(L, ray->p.y);
    return 2;
}

int raySetStartPosition(lua_State* L)
{
    c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
    ray->p.x = luaL_checknumber(L, 2);
    ray->p.y = luaL_checknumber(L, 3);
    return 0;
}

int rayGetTargetPosition(lua_State* L)
{
    c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
    lua_pushnumber(L, ray->d.x);
    lua_pushnumber(L, ray->d.y);
    return 2;
}

int raySetTargetPosition(lua_State* L)
{
    c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
    ray->d.x = luaL_checknumber(L, 2);
    ray->d.y = luaL_checknumber(L, 3);
    return 0;
}

int rayGetDistance(lua_State* L)
{
    c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
    lua_pushnumber(L, ray->t);
    return 1;
}

int raySetDistance(lua_State* L)
{
    c2Ray* ray = getPtr<c2Ray>(L, "c2Ray", 1);
    ray->t = luaL_checknumber(L, 2);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// TRANSFORM
///
/////////////////////////////////////////////////////////////////////////////////////////////

// c2x

static int transformIndex(lua_State* L) // __index
{
    const char* key = lua_tostring(L, 2);    
    
    if (strcmp(key, "x") == 0)
    {
        c2x* transform = getPtr<c2x>(L, "c2x", 1);
        lua_pushnumber(L, transform->p.x);
        return 1;
    }
    else if (strcmp(key, "y") == 0)
    {
        c2x* transform = getPtr<c2x>(L, "c2x", 1);
        lua_pushnumber(L, transform->p.y);
        return 1;
    }
    else if (strcmp(key, "rotation") == 0)
    {
        c2x* transform = getPtr<c2x>(L, "c2x", 1);
        lua_pushnumber(L, asin(transform->r.s));
        return 1;
    }
    
    lua_getmetatable(L, -2);
    lua_rawgetfield(L, -1, key);
    return 1;
}

static int transformNewIndex(lua_State* L) // __newindex
{
    const char* key = lua_tostring(L, 2);
    float value = lua_tonumber(L, 3);
    
    if (strcmp(key, "x") == 0)
    {
        c2x* transform = getPtr<c2x>(L, "c2x", 1);
        transform->p.x = value;
        return 0;
    }
    else if (strcmp(key, "y") == 0)
    {
        c2x* transform = getPtr<c2x>(L, "c2x", 1);
        transform->p.y = value;
        return 0;
    }
    else if (strcmp(key, "y") == 0)
    {
        c2x* transform = getPtr<c2x>(L, "c2x", 1);
        transform->r = c2Rot(value);
        return 0;
    }
    
    lua_rawset(L, 1);
    return 0;
}

int createTransform(lua_State* L)
{
    c2x* transform = new c2x();
    transform->p = c2V(luaL_optnumber(L, 1, 0.0f), luaL_optnumber(L, 2, 0.0f));
    transform->r = c2Rot(luaL_optnumber(L, 3, 0.0f));
    g_pushInstance(L, "c2x", transform);
    
    ADD_METAMETHODS("c2x", transformIndex, transformNewIndex);
    setPtr(L, transform);

    return 1;
}

int setTransformPosition(lua_State* L)
{
    c2x* transform = getPtr<c2x>(L, "c2x", 1);
    transform->p.x = luaL_checknumber(L, 2);
    transform->p.y = luaL_checknumber(L, 3);
    return 0;
}

int moveTransformPosition(lua_State* L)
{
    c2x* transform = getPtr<c2x>(L, "c2x", 1);
    transform->p.x += luaL_checknumber(L, 2);
    transform->p.y += luaL_checknumber(L, 3);
    return 0;
}

int setTransformRotation(lua_State* L)
{
    c2x* transform = getPtr<c2x>(L, "c2x", 1);
    transform->r = c2Rot(luaL_checknumber(L, 2));
    return 0;
}

int rotateTransform(lua_State* L)
{
    c2x* transform = getPtr<c2x>(L, "c2x", 1);
    float r = asin(transform->r.s);
    transform->r = c2Rot(r + luaL_checknumber(L, 2));
    return 0;
}

int getTransformPosition(lua_State* L)
{
    c2x* transform = getPtr<c2x>(L, "c2x", 1);
    lua_pushnumber(L, transform->p.x);
    lua_pushnumber(L, transform->p.y);
    return 2;
}

int getTransformRotation(lua_State* L)
{
    c2x* transform = getPtr<c2x>(L, "c2x", 1);
    lua_pushnumber(L, asin(transform->r.s));
    return 1;
}

/*
// c2v array
int cteatePointsArray(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    int l = lua_objlen(L, 1);
    c2v* arr = new c2v[l / 2];
    for (int i = 0; i < l; i += 2)
    {
        lua_rawgeti(L, 1, i + 1);
        float x = luaL_checknumber(L, -1);
        lua_pop(L, 1);
        lua_rawgeti(L, 1, i + 2);
        float y = luaL_checknumber(L, -1);
        lua_pop(L, 1);
        arr[i] = c2V(x, y);
    }

    g_pushInstance(L, "c2Points", arr);

    setPtr(L, arr);

    return 1;
}
*/

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// CUTE_C2_API
///
/////////////////////////////////////////////////////////////////////////////////////////////

int c2CircletoCircle_lua(lua_State* L)
{
    c2Circle A = *getPtr<c2Circle>(L, "c2Circle", 1);
    c2Circle B = *getPtr<c2Circle>(L, "c2Circle", 2);

    int result = c2CircletoCircle(A, B);
    lua_pushboolean(L, result);
    return 1;
}

int c2CircletoAABB_lua(lua_State* L)
{
    c2Circle A = *getPtr<c2Circle>(L, "c2Circle", 1);
    c2AABB B = *getPtr<c2AABB>(L, "c2AABB", 2);

    int result = c2CircletoAABB(A, B);
    lua_pushboolean(L, result);
    return 1;
}

int c2CircletoCapsule_lua(lua_State* L)
{
    c2Circle A = *getPtr<c2Circle>(L, "c2Circle", 1);
    c2Capsule B = *getPtr<c2Capsule>(L, "c2Capsule", 2);

    int result = c2CircletoCapsule(A, B);
    lua_pushboolean(L, result);
    return 1;
}

int c2AABBtoAABB_lua(lua_State* L)
{
    c2AABB A = *getPtr<c2AABB>(L, "c2AABB", 1);
    c2AABB B = *getPtr<c2AABB>(L, "c2AABB", 2);

    int result = c2AABBtoAABB(A, B);
    lua_pushboolean(L, result);
    return 1;
}

int c2AABBtoCapsule_lua(lua_State* L)
{
    c2AABB A = *getPtr<c2AABB>(L, "c2AABB", 1);
    c2Capsule B = *getPtr<c2Capsule>(L, "c2Capsule", 2);

    int result = c2AABBtoCapsule(A, B);
    lua_pushboolean(L, result);
    return 1;
}

int c2CapsuletoCapsule_lua(lua_State* L)
{
    c2Capsule A = *getPtr<c2Capsule>(L, "c2Capsule", 1);
    c2Capsule B = *getPtr<c2Capsule>(L, "c2Capsule", 2);

    int result = c2CapsuletoCapsule(A, B);
    lua_pushboolean(L, result);
    return 1;
}

int c2CircletoPoly_lua(lua_State* L)
{
    c2Circle A = *getPtr<c2Circle>(L, "c2Circle", 1);
    c2Poly* B = getPtr<c2Poly>(L, "c2Poly", 2);
    c2x transofrm = checkTransform(L, 3);

    int result = c2CircletoPoly(A, B, &transofrm);
    lua_pushboolean(L, result);
    return 1;
}

int c2AABBtoPoly_lua(lua_State* L)
{
    c2AABB A = *getPtr<c2AABB>(L, "c2AABB", 1);
    c2Poly* B = getPtr<c2Poly>(L, "c2Poly", 2);
    c2x transofrm = checkTransform(L, 3);

    int result = c2AABBtoPoly(A, B, &transofrm);
    lua_pushboolean(L, result);
    return 1;
}

int c2CapsuletoPoly_lua(lua_State* L)
{
    c2Capsule A = *getPtr<c2Capsule>(L, "c2Capsule", 1);
    c2Poly* B = getPtr<c2Poly>(L, "c2Poly", 1);
    c2x transofrm = checkTransform(L, 3);

    int result = c2CapsuletoPoly(A, B, &transofrm);
    lua_pushboolean(L, result);
    return 1;
}

int c2PolytoPoly_lua(lua_State* L)
{
    c2Poly* A = getPtr<c2Poly>(L, "c2Poly", 1);
    c2x transofrmA = checkTransform(L, 2);
    c2Poly* B = getPtr<c2Poly>(L, "c2Poly", 3);
    c2x transofrmB = checkTransform(L, 4);

    int result = c2PolytoPoly(A, &transofrmA, B, &transofrmB);
    lua_pushboolean(L, result);
    return 1;
}

int c2CastRay_lua(lua_State* L)
{
    c2Raycast out;
    if (lua_gettop(L) > 3)
    {
        c2Ray ray = c2Ray();
        ray.p = c2V(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
        ray.d = c2V(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
        ray.t = luaL_checknumber(L, 5);
        
        C2_TYPE shapeType = getShapeType(L, 6);
        
        void* obj = getRawPtr(L, 6);
        
        c2x bx = checkTransform(L, 7);
        
        int result = c2CastRay(ray, obj, &bx, shapeType, &out);
        lua_pushinteger(L, result);
        lua_pushnumber(L, out.n.x);
        lua_pushnumber(L, out.n.y);
        lua_pushnumber(L, out.t);
        return 4;
    }
    
    c2Ray ray = *getPtr<c2Ray>(L, "c2Ray", 1);        
    void* obj = getRawPtr(L, 2);
    c2x bx = checkTransform(L, 3);
    
    C2_TYPE shapeType = getShapeType(L, 2);

    int result = c2CastRay(ray, obj, &bx, shapeType, &out);
    lua_pushinteger(L, result);
    lua_pushnumber(L, out.n.x);
    lua_pushnumber(L, out.n.y);
    lua_pushnumber(L, out.t);
    return 4;
}

int c2Collide_lua(lua_State* L)
{
    void* A = getRawPtr(L, 1);
    C2_TYPE shapeTypeA = getShapeType(L, 1);
    
    void* B = getRawPtr(L, 2);
    C2_TYPE shapeTypeB = getShapeType(L, 2);
    
    c2x transformA = checkTransform(L, 3);
    c2x transformB = checkTransform(L, 4);
    
    c2Manifold m;

    c2Collide(A, &transformA, shapeTypeA, B, &transformB, shapeTypeB, &m);
    
    return lua_pushmanifold(L, m);
}

int c2Collided_lua(lua_State* L)
{
    void* A = getRawPtr(L, 1);
    C2_TYPE shapeTypeA = getShapeType(L, 1);
    
    void* B = getRawPtr(L, 2);
    C2_TYPE shapeTypeB = getShapeType(L, 2);
    
    c2x transformA = checkTransform(L, 3);
    c2x transformB = checkTransform(L, 4);

    int result = c2Collided(A, &transformA, shapeTypeA, B, &transformB, shapeTypeB);
    lua_pushboolean(L, result);
    return 1;
}

int c2CircletoCircleManifold_lua(lua_State* L)
{
    c2Circle A = *getPtr<c2Circle>(L, "c2Circle", 1);
    c2Circle B = *getPtr<c2Circle>(L, "c2Circle", 2);
    c2Manifold m;

    c2CircletoCircleManifold(A, B, &m);
    
    return lua_pushmanifold(L, m);
}

int c2CircletoAABBManifold_lua(lua_State* L)
{
    c2Circle A = *getPtr<c2Circle>(L, "c2Circle", 1);
    c2AABB B = *getPtr<c2AABB>(L, "c2AABB", 2);
    c2Manifold m;

    c2CircletoAABBManifold(A, B, &m);
    
    return lua_pushmanifold(L, m);
}

int c2CircletoCapsuleManifold_lua(lua_State* L)
{
    c2Circle A = *getPtr<c2Circle>(L, "c2Circle", 1);
    c2Capsule B = *getPtr<c2Capsule>(L, "c2Capsule", 2);
    c2Manifold m;

    c2CircletoCapsuleManifold(A, B, &m);
    
    return lua_pushmanifold(L, m);
}

int c2AABBtoAABBManifold_lua(lua_State* L)
{
    c2AABB A = *getPtr<c2AABB>(L, "c2AABB", 1);
    c2AABB B = *getPtr<c2AABB>(L, "c2AABB", 2);
    c2Manifold m;

    c2AABBtoAABBManifold(A, B, &m);
    
    return lua_pushmanifold(L, m);
}

int c2AABBtoCapsuleManifold_lua(lua_State* L)
{
    c2AABB A = *getPtr<c2AABB>(L, "c2AABB", 1);
    c2Capsule B = *getPtr<c2Capsule>(L, "c2Capsule", 2);
    c2Manifold m;

    c2AABBtoCapsuleManifold(A, B, &m);
    
    return lua_pushmanifold(L, m);
}

int c2CapsuletoCapsuleManifold_lua(lua_State* L)
{
    c2Capsule A = *getPtr<c2Capsule>(L, "c2Capsule", 1);
    c2Capsule B = *getPtr<c2Capsule>(L, "c2Capsule", 2);
    c2Manifold m;

    c2CapsuletoCapsuleManifold(A, B, &m);
    
    return lua_pushmanifold(L, m);
}

int c2CircletoPolyManifold_lua(lua_State* L)
{
    c2Circle A = *getPtr<c2Circle>(L, "c2Circle", 1);
    c2Poly* B = getPtr<c2Poly>(L, "c2Poly", 2);
    c2x* bx;
    c2Manifold m;

    c2CircletoPolyManifold(A, B, bx, &m);
    
    return lua_pushmanifold(L, m);
}

int c2AABBtoPolyManifold_lua(lua_State* L)
{
    c2AABB A = *getPtr<c2AABB>(L, "c2AABB", 1);
    c2Poly* B = getPtr<c2Poly>(L, "c2Poly", 2);
    c2x bx = checkTransform(L, 3);
    c2Manifold m;

    c2AABBtoPolyManifold(A, B, &bx, &m);
    
    return lua_pushmanifold(L, m);
}

int c2CapsuletoPolyManifold_lua(lua_State* L)
{
    c2Capsule A = *getPtr<c2Capsule>(L, "c2Capsule", 1);
    c2Poly* B = getPtr<c2Poly>(L, "c2Poly", 2);
    c2x bx = checkTransform(L, 3);
    c2Manifold m;

    c2CapsuletoPolyManifold(A, B, &bx, &m);
    
    return lua_pushmanifold(L, m);
}

int c2PolytoPolyManifold_lua(lua_State* L)
{
    c2Poly* A = getPtr<c2Poly>(L, "c2Poly", 1);
    c2x ax = checkTransform(L, 2);
    c2Poly* B = getPtr<c2Poly>(L, "c2Poly", 3);
    c2x bx = checkTransform(L, 4);
    c2Manifold m;

    c2PolytoPolyManifold(A, &ax, B, &bx, &m);
    
    return lua_pushmanifold(L, m);
}

int c2GJK_lua(lua_State* L)
{
    void* A = getRawPtr(L, 1);
    C2_TYPE shapeTypeA = getShapeType(L, 1);
    
    void* B = getRawPtr(L, 2);
    C2_TYPE shapeTypeB = getShapeType(L, 2);
    
    c2x transformA = checkTransform(L, 3);
    c2x transformB = checkTransform(L, 4);
    
    c2v outA;
    c2v outB;
    int use_radius = lua_toboolean(L, 6);
    int iterations;

    float result = c2GJK(
                A, shapeTypeA, &transformA,
                B, shapeTypeB, &transformB,
                &outA, &outB,
                use_radius,
                &iterations,
                0);
    lua_pushnumber(L, result);
    lua_pushnumber(L, outA.x);
    lua_pushnumber(L, outA.y);
    lua_pushnumber(L, outB.x);
    lua_pushnumber(L, outB.y);
    lua_pushnumber(L, iterations);
    return 6;
}

int c2TOI_lua(lua_State* L)
{
    void* A = getRawPtr(L, 1);
    C2_TYPE shapeTypeA = getShapeType(L, 1);
    c2v vA = c2V(luaL_checknumber(L, 2), luaL_checknumber(L, 3));
    
    void* B = getRawPtr(L, 4);
    C2_TYPE shapeTypeB = getShapeType(L, 4);
    c2v vB = c2V(luaL_checknumber(L, 5), luaL_checknumber(L, 6));
    
    int use_radius = lua_toboolean(L, 7);
    
    c2x transformA = checkTransform(L, 8);
    c2x transformB = checkTransform(L, 9);
    
    c2TOIResult result = c2TOI(
                A, shapeTypeA, &transformA, vA,
                B, shapeTypeB, &transformB, vB,
                use_radius);
    
    lua_pushinteger(L, result.hit);
    lua_pushnumber(L, result.toi);
    lua_pushnumber(L, result.n.x);
    lua_pushnumber(L, result.n.y);
    lua_pushnumber(L, result.p.x);
    lua_pushnumber(L, result.p.y);
    lua_pushnumber(L, result.iterations);
    return 7;
}

int c2Impact_lua(lua_State* L)
{
    if (lua_gettop(L) > 2)
    {
        c2Ray ray = c2Ray();
        ray.d = c2V(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
        ray.p = c2V(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
        ray.t = luaL_checknumber(L, 5);
        float t = luaL_checknumber(L, 6);
        c2v result = c2Impact(ray, t);
        lua_pushnumber(L, result.x);
        lua_pushnumber(L, result.y);
        return 2;
    }
    c2Ray ray = *getPtr<c2Ray>(L, "c2Ray", 1);
    float t = luaL_checknumber(L, 2);
    c2v result = c2Impact(ray, t);
    lua_pushnumber(L, result.x);
    lua_pushnumber(L, result.y);
    return 2;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// LOADER
///
/////////////////////////////////////////////////////////////////////////////////////////////

int loader(lua_State* L)
{
    const luaL_Reg circleFunctionsList[] = {
        {"setPosition", setCirclePosition},
        {"getPosition", getCirclePosition},
        {"setRadius", setCircleRadius},
        {"getRadius", getCircleRadius},
        {"getData", cirlceGetData},
        {"getBoundingBox", getCirlceBoundingBox},
        {"rayTest", cirlceRayTest},
        {"inflate", circleInflate},
        {"hitTest", circleHitTest},
        {NULL, NULL}
    };
    g_createClass(L, "c2Circle", NULL, NULL, NULL, circleFunctionsList);

    const luaL_Reg aabbFunctionsList[] = {
        {"setPosition", setAABBPosition},
        {"getPosition", getAABBPosition},
        {"setSize", setAABBSize},
        {"getSize", getAABBSize},
        {"getData", getAABBData},
        {"getBoundingBox", getAABBboundingBox},
        {"rayTest", AABBRayTest},
        {"inflate", AABBInflate},
        {"hitTest", AABBHitTest},
        {NULL, NULL}
    };
    g_createClass(L, "c2AABB", NULL, NULL, NULL, aabbFunctionsList);

    const luaL_Reg capsuleFunctionsList[] = {
        {"setPosition", setCapsulePosition},
        {"getPosition", getCapsulePosition},
        {"setHeight", setCapsuleHeight},
        {"getHeight", getCapsuleHeight},
        {"setRadius", setCapsuleRadius},
        {"getRadius", getCapsuleRadius},
        {"getBoundingBox", getCapsuleBoundingBox},
        {"getData", getCapsuleData},
        {"rayTest", capsuleRayTest},
        {"inflate", capsuleInflate},
        {"hitTest", capsuleHitTest},
        {NULL, NULL}
    };
    g_createClass(L, "c2Capsule", NULL, NULL, NULL, capsuleFunctionsList);

    const luaL_Reg polyFunctionsList[] = {
        {"updatePoints", updatePoints},
        {"updatePoint", updatePoint},
        {"getPoints", getPoints},
        {"getBoundingBox", getPolyBoundingBox},
        {"getData", polyGetData},
        {"rayTest", polyRayTest},
        {"inflate", polyInflate},
        {"hitTest", polyHitTest},
        {NULL, NULL}
    };
    g_createClass(L, "c2Poly", NULL, NULL, NULL, polyFunctionsList);

    const luaL_Reg rayFunctionsList[] = {
        {"getStartPosition", rayGetStartPosition},
        {"setStartPosition", raySetStartPosition},
        {"getTargetPosition", rayGetTargetPosition},
        {"setTargetPosition", raySetTargetPosition},
        {"getT", rayGetDistance},
        {"setT", raySetDistance},
        {NULL, NULL}
    };
    g_createClass(L, "c2Ray", NULL, NULL, NULL, rayFunctionsList);

    const luaL_Reg c2xFunctionsList[] = {
        {"setPosition", setTransformPosition},
        {"getPosition", getTransformPosition},
        {"move", moveTransformPosition},

        {"setRotation", setTransformRotation},
        {"getRotation", getTransformRotation},
        {"rotate", rotateTransform},

        {NULL, NULL}
    };
    g_createClass(L, "c2x", NULL, NULL, NULL, c2xFunctionsList);

    const luaL_Reg cuteC2FunctionsList[] = {
        {"circle", createCircle},
        {"aabb", createAABB},
        {"capsule", createCapsule},
        {"poly", createPoly},
        {"ray", createRay},
        {"transform", createTransform}, //c2x

        {"circleToCircle", c2CircletoCircle_lua},
        {"circleToAABB", c2CircletoAABB_lua},
        {"circleToCapsule", c2CircletoCapsule_lua},
        {"AABBtoAABB", c2AABBtoAABB_lua},
        {"AABBtoCapsule", c2AABBtoCapsule_lua},
        {"capsuleToCapsule", c2CapsuletoCapsule_lua},
        {"circleToPoly", c2CircletoPoly_lua},
        {"AABBtoPoly", c2AABBtoPoly_lua},
        {"capsuleToPoly", c2CapsuletoPoly_lua},
        {"polyToPoly", c2PolytoPoly_lua},
        
        {"castRay", c2CastRay_lua},
        {"collide", c2Collide_lua},
        {"collided", c2Collided_lua},

        {"circleToCircleManifold", c2CircletoCircleManifold_lua},
        {"circleToAABBManifold", c2CircletoAABBManifold_lua},
        {"circleToCapsuleManifold", c2CircletoCapsuleManifold_lua},
        {"AABBtoAABBManifold", c2AABBtoAABBManifold_lua},
        {"AABBtoCapsuleManifold", c2AABBtoCapsuleManifold_lua},
        {"capsuleToCapsuleManifold", c2CapsuletoCapsuleManifold_lua},
        {"circletoPolyManifold", c2CircletoPolyManifold_lua},
        {"AABBtoPolyManifold", c2AABBtoPolyManifold_lua},
        {"capsuleToPolyManifold", c2CapsuletoPolyManifold_lua},
        {"polyToPolyManifold", c2PolytoPolyManifold_lua},
        
        {"impact", c2Impact_lua},
        
        {"GJK", c2GJK_lua},
        {"TOI", c2TOI_lua},
        {NULL, NULL}
    };

    g_createClass(L, "CuteC2", NULL, NULL, NULL, cuteC2FunctionsList);

    luaL_newweaktable(L);
    luaL_rawsetptr(L, LUA_REGISTRYINDEX, &keyWeak);

    bindEnums(L);

    return 1;
}

}

static void g_initializePlugin(lua_State* L)
{
    ::L = L;

    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");

    lua_pushcfunction(L, cute_c2_impl::loader, "loader");
    lua_setfield(L, -2, PLUGIN_NAME);

    lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State* _UNUSED(L)) { }

#ifdef IS_BETA_BUILD
REGISTER_PLUGIN_NAMED(PLUGIN_NAME, "1.0.0", CuteC2_beta)
#else
REGISTER_PLUGIN_NAMED(PLUGIN_NAME, "1.0.0", CuteC2)
#endif
