#define IS_BETA_BUILD

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

static lua_State* L;
static Application* application;
static char keyWeak = ' ';
static const std::string SHAPES[4] = {"c2Circle", "c2AABB", "c2Capsule", "c2Poly"};

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
        switch (t)
        {
        case LUA_TSTRING:
        {
            lua_getglobal(L, "print");
            lua_pushfstring(L, "[S] %d:'%s'", i, lua_tostring(L, i));
            lua_call(L, 1, 0);
        }
            break;
        case LUA_TBOOLEAN:
        {
            lua_getglobal(L, "print");
            lua_pushfstring(L, "[B] %d: %s", i, lua_toboolean(L, i) ? "true" : "false");
            lua_call(L, 1, 0);
        }
            break;
        case LUA_TNUMBER:
        {
            lua_getglobal(L, "print");
            lua_pushfstring(L, "[N] %d: %f", i, lua_tonumber(L, i));
            lua_call(L, 1, 0);
        }
            break;
        default:
        {
            lua_getglobal(L, "print");
            lua_pushfstring(L, "[D] %d: %s", i, lua_typename(L, t));
            lua_call(L, 1, 0);
        }
            break;
        }
        i--;
    }
    lua_getglobal(L, "print");
    lua_pushstring(L, "------------ Stack Dump Finished ------------\n");
    lua_call(L, 1, 0);

    DUMP_INDEX++;
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

inline void setPtr(lua_State* L, void* ptr)
{
    luaL_rawgetptr(L, LUA_REGISTRYINDEX, &keyWeak);
    lua_pushvalue(L, -2);
    luaL_rawsetptr(L, -2, ptr);
    lua_pop(L, 1);
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// ENUMS
///
/////////////////////////////////////////////////////////////////////////////////////////////

void bindEnums(lua_State* L)
{
    lua_getglobal(L, "CuteC2");


    BIND_IENUM(C2_TYPE_NONE, "C2_TYPE_NONE");
    BIND_IENUM(C2_TYPE_CIRCLE, "C2_TYPE_CIRCLE");
    BIND_IENUM(C2_TYPE_AABB, "C2_TYPE_AABB");
    BIND_IENUM(C2_TYPE_CAPSULE, "C2_TYPE_CAPSULE");
    BIND_IENUM(C2_TYPE_POLY, "C2_TYPE_POLY");

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
int createCircle(lua_State* L)
{
    c2Circle* circle = new c2Circle();
    circle->p = c2V(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
    circle->r = luaL_checknumber(L, 3);
    g_pushInstance(L, "c2Circle", circle);

    setPtr(L, circle);

    return 1;
}

int SetCirclePosition(lua_State* L)
{
    c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
    circle->p.x = luaL_checknumber(L, 2);
    circle->p.y = luaL_checknumber(L, 3);
    return 0;
}

int GetCirclePosition(lua_State* L)
{
    c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
    lua_pushnumber(L, circle->p.x );
    lua_pushnumber(L, circle->p.y );
    return 2;
}

int SetCircleRadius(lua_State* L)
{
    c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
    circle->r = luaL_checknumber(L, 2);
    return 0;
}

int GetCircleRadius(lua_State* L)
{
    c2Circle* circle = getPtr<c2Circle>(L, "c2Circle", 1);
    lua_pushnumber(L, circle->r);
    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// AABB
///
/////////////////////////////////////////////////////////////////////////////////////////////

// c2AABB
int createAABB(lua_State* L)
{
    c2AABB* aabb = new c2AABB();
    aabb->min = c2V(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
    aabb->max = c2Add(aabb->min, c2V(luaL_checknumber(L, 3), luaL_checknumber(L, 4)));
    g_pushInstance(L, "c2AABB", aabb);

    setPtr(L, aabb);

    return 1;
}

int SetAABBPosition(lua_State* L)
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

int GetAABBPosition(lua_State* L)
{
    c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
    lua_pushnumber(L, aabb->min.x);
    lua_pushnumber(L, aabb->min.y);
    return 2;
}

int SetAABBSize(lua_State* L)
{
    c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
    float w = luaL_checknumber(L, 2);
    float h = luaL_checknumber(L, 3);
    aabb->max.x = aabb->min.x + w;
    aabb->max.y = aabb->min.y + h;
    return 0;
}

int GetAABBSize(lua_State* L)
{
    c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
    lua_pushnumber(L, aabb->max.x - aabb->min.x);
    lua_pushnumber(L, aabb->max.y - aabb->min.y);
    return 2;
}

int GetAABBRect(lua_State* L)
{
    c2AABB* aabb = getPtr<c2AABB>(L, "c2AABB", 1);
    lua_pushnumber(L, aabb->min.x);
    lua_pushnumber(L, aabb->min.y);
    lua_pushnumber(L, aabb->max.x);
    lua_pushnumber(L, aabb->max.y);
    return 4;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// CAPSULE
///
/////////////////////////////////////////////////////////////////////////////////////////////

// c2Capsule
int createCapsule(lua_State* L)
{
    c2Capsule* capsule = new c2Capsule();
    c2v pos = c2V(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
    float h = luaL_checknumber(L, 3);
    capsule->a = pos;
    capsule->b = c2V(pos.x, pos.y + h);
    capsule->r = luaL_checknumber(L, 4);
    g_pushInstance(L, "c2Capsule", capsule);

    setPtr(L, capsule);

    return 1;
}

int SetCapsulePosition(lua_State* L)
{
    c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    float dx = abs(capsule->a.x - x);
    float dy = abs(capsule->a.y - y);
    capsule->a.x = x;
    capsule->a.y = y;
    capsule->b.x = x + dx;
    capsule->b.y = y + dy;
    return 0;
}

int GetCapsulePosition(lua_State* L)
{
    c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
    lua_pushnumber(L, capsule->a.x);
    lua_pushnumber(L, capsule->a.y);
    return 2;
}

int SetCapsuleHeight(lua_State* L)
{
    c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
    float h = luaL_checknumber(L, 2);
    capsule->b.y = capsule->a.y + h;
    return 0;
}

int GetCapsuleHeight(lua_State* L)
{
    c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
    lua_pushnumber(L, capsule->a.x);
    lua_pushnumber(L, capsule->a.y);
    return 2;
}

int SetCapsuleRadius(lua_State* L)
{
    c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
    capsule->r = luaL_checknumber(L, 2);
    return 0;
}

int GetCapsuleRadius(lua_State* L)
{
    c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
    lua_pushnumber(L, capsule->r);
    return 1;
}

int GetCapsuleData(lua_State* L)
{
    c2Capsule* capsule = getPtr<c2Capsule>(L, "c2Capsule", 1);
    lua_pushnumber(L, capsule->a.x);
    lua_pushnumber(L, capsule->a.y);
    lua_pushnumber(L, capsule->b.x);
    lua_pushnumber(L, capsule->b.y);
    lua_pushnumber(L, capsule->r);
    return 5;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// POLYGON
///
/////////////////////////////////////////////////////////////////////////////////////////////


void UpdatePointsInternal(lua_State* L, int idx, c2Poly* poly)
{
    luaL_checktype(L, idx, LUA_TTABLE);
    int l = lua_objlen(L, idx);
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

// c2Poly
int createPoly(lua_State* L)
{
    c2Poly* poly = new c2Poly();
    UpdatePointsInternal(L, 1, poly);
    g_pushInstance(L, "c2Poly", poly);
    setPtr(L, poly);
    return 1;
}

int UpdatePoints(lua_State* L)
{
    c2Poly* poly = getPtr<c2Poly>(L, "c2Poly", 1);
    UpdatePointsInternal(L, 2, poly);
    return 0;
}

int UpdatePoint(lua_State* L)
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

int GetPoints(lua_State* L)
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

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// RAY
///
/////////////////////////////////////////////////////////////////////////////////////////////

// c2Ray
int createRay(lua_State* L)
{
    c2Ray* ray = new c2Ray();
    ray->d = c2V(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
    ray->p = c2V(luaL_checknumber(L, 3), luaL_checknumber(L, 4));
    ray->t = luaL_optnumber(L, 5, 1.0f);
    g_pushInstance(L, "c2Ray", ray);

    setPtr(L, ray);

    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// TRANSFORM
///
/////////////////////////////////////////////////////////////////////////////////////////////

// c2x
int createTransform(lua_State* L)
{
    c2x* transform = new c2x();
    transform->p = c2V(luaL_optnumber(L, 1, 0.0f), luaL_optnumber(L, 2, 0.0f));
    transform->r = c2Rot(luaL_optnumber(L, 3, 0.0f));
    g_pushInstance(L, "c2x", transform);

    setPtr(L, transform);

    return 1;
}

int SetTransformPosition(lua_State* L)
{
    c2x* transform = getPtr<c2x>(L, "c2x", 1);
    transform->p.x = luaL_checknumber(L, 2);
    transform->p.y = luaL_checknumber(L, 3);
    return 0;
}

int MoveTransformPosition(lua_State* L)
{
    c2x* transform = getPtr<c2x>(L, "c2x", 1);
    transform->p.x += luaL_checknumber(L, 2);
    transform->p.y += luaL_checknumber(L, 3);
    return 0;
}

int SetTransformRotation(lua_State* L)
{
    c2x* transform = getPtr<c2x>(L, "c2x", 1);
    transform->r = c2Rot(luaL_checknumber(L, 2));
    return 0;
}

int RotateTransform(lua_State* L)
{
    c2x* transform = getPtr<c2x>(L, "c2x", 1);
    float r = asin(transform->r.s);
    transform->r = c2Rot(r + luaL_checknumber(L, 2));
    return 0;
}

int GetTransformPosition(lua_State* L)
{
    c2x* transform = getPtr<c2x>(L, "c2x", 1);
    lua_pushnumber(L, transform->p.x);
    lua_pushnumber(L, transform->p.y);
    return 2;
}

int GetTransformRotation(lua_State* L)
{
    c2x* transform = getPtr<c2x>(L, "c2x", 1);
    lua_pushnumber(L, asin(transform->r.s));
    return 1;
}

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

// c2Raycast
int createRaycast(lua_State* L)
{

}

/////////////////////////////////////////////////////////////////////////////////////////////
///
/// HELPERS
///
/////////////////////////////////////////////////////////////////////////////////////////////

struct GShape
{
    void* instance;
    const char* name;
    GShape(void* p_instance, const char* p_name)
    {
        instance = p_instance;
        name = p_name;
    }
};

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

// if shape is unknown, bruteforce :D
GShape getShape(lua_State* L, int idx)
{
    if (lua_isnil(L, idx))
        return GShape(nullptr, "");

    for (int i = 0; i < 4; i++)
    {
        const char* name = SHAPES[i].c_str();
        if (g_isInstanceOf(L, name, idx))
        {
            return GShape(g_getInstance(L, name, idx), name);
        }
    }

    return GShape(nullptr, "");
}

void lua_pushmanifold(lua_State* L, c2Manifold m)
{
    lua_pushinteger(L, m.count);

    lua_pushnumber(L, m.depths[0]);
    lua_pushnumber(L, m.depths[1]);

    lua_pushnumber(L, m.contact_points[0].x);
    lua_pushnumber(L, m.contact_points[0].y);
    lua_pushnumber(L, m.contact_points[1].x);
    lua_pushnumber(L, m.contact_points[1].y);

    lua_pushnumber(L, m.n.x);
    lua_pushnumber(L, m.n.y);
}

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

int c2RaytoCircle_lua(lua_State* L)
{
    c2Ray A = *getPtr<c2Ray>(L, "c2Ray", 1);
    c2Circle B = *getPtr<c2Circle>(L, "c2Circle", 2);
    c2Raycast out;

    int result = c2RaytoCircle(A, B, &out);
    lua_pushboolean(L, result);
    lua_pushnumber(L, out.n.x);
    lua_pushnumber(L, out.n.y);
    lua_pushnumber(L, out.t);
    return 4;
}

int c2RaytoAABB_lua(lua_State* L)
{
    c2Ray A = *getPtr<c2Ray>(L, "c2Ray", 1);
    c2AABB B = *getPtr<c2AABB>(L, "c2AABB", 2);
    c2Raycast out;

    int result = c2RaytoAABB(A, B, &out);
    lua_pushboolean(L, result);
    lua_pushnumber(L, out.n.x);
    lua_pushnumber(L, out.n.y);
    lua_pushnumber(L, out.t);
    return 4;
}

int c2RaytoCapsule_lua(lua_State* L)
{
    c2Ray A = *getPtr<c2Ray>(L, "c2Ray", 1);
    c2Capsule B = *getPtr<c2Capsule>(L, "c2Capsule", 2);
    c2Raycast out;

    int result = c2RaytoCapsule(A, B, &out);
    lua_pushboolean(L, result);
    lua_pushnumber(L, out.n.x);
    lua_pushnumber(L, out.n.y);
    lua_pushnumber(L, out.t);
    return 4;
}

int c2RaytoPoly_lua(lua_State* L)
{
    c2Ray A = *getPtr<c2Ray>(L, "c2Ray", 1);
    c2Poly* B = getPtr<c2Poly>(L, "c2Poly", 2);
    c2x bx_ptr = checkTransform(L, 3);
    c2Raycast out;

    int result = c2RaytoPoly(A, B, &bx_ptr, &out);
    lua_pushboolean(L, result);
    lua_pushnumber(L, out.n.x);
    lua_pushnumber(L, out.n.y);
    lua_pushnumber(L, out.t);
    return 4;
}

int c2CastRay_lua(lua_State* L)
{
    int idx = 1;
    c2Ray A = *getPtr<c2Ray>(L, "c2Ray", 1);
    GShape B = getShape(L, 2);
    c2x bx = checkTransform(L, 3);
    C2_TYPE typeB = (C2_TYPE)luaL_checkinteger(L, 4);
    c2Raycast out;

    int result = c2CastRay(A, B.instance, &bx, typeB, &out);
    lua_pushinteger(L, result);
    lua_pushnumber(L, out.n.x);
    lua_pushnumber(L, out.n.y);
    lua_pushnumber(L, out.t);
    return 4;
}

// TODO?
int c2Hull_lua(lua_State* L)
{
    LUA_THROW_ERROR("Not implemented!");
    //int result = c2Hull(verts, count);
    //lua_pushinteger(L, result);
    return 1;
}

int c2Collide_lua(lua_State* L)
{
    GShape A = getShape(L, 1);
    c2x ax = checkTransform(L, 2);
    C2_TYPE typeA = (C2_TYPE)luaL_checkinteger(L, 3);
    GShape B = getShape(L, 4);
    c2x bx = checkTransform(L, 5);
    C2_TYPE typeB = (C2_TYPE)luaL_checkinteger(L, 6);;
    c2Manifold m;

    c2Collide(A.instance, &ax, typeA, B.instance, &bx, typeB, &m);
    lua_pushmanifold(L, m);
    return 9;
}

int c2Collided_lua(lua_State* L)
{
    GShape A = getShape(L, 1);
    c2x ax = checkTransform(L, 2);
    C2_TYPE typeA = (C2_TYPE)luaL_checkinteger(L, 3);
    GShape B = getShape(L, 4);
    c2x bx = checkTransform(L, 5);
    C2_TYPE typeB = (C2_TYPE)luaL_checkinteger(L, 6);

    int result = c2Collided(A.instance, &ax, typeA, B.instance, &bx, typeB);
    lua_pushboolean(L, result);
    return 1;
}

int c2CircletoCircleManifold_lua(lua_State* L)
{
    c2Circle A = *getPtr<c2Circle>(L, "c2Circle", 1);
    c2Circle B = *getPtr<c2Circle>(L, "c2Circle", 2);
    c2Manifold m;

    c2CircletoCircleManifold(A, B, &m);
    lua_pushmanifold(L, m);
    return 9;
}

int c2CircletoAABBManifold_lua(lua_State* L)
{
    c2Circle A = *getPtr<c2Circle>(L, "c2Circle", 1);
    c2AABB B = *getPtr<c2AABB>(L, "c2AABB", 2);
    c2Manifold m;

    c2CircletoAABBManifold(A, B, &m);
    lua_pushmanifold(L, m);
    return 9;
}

int c2CircletoCapsuleManifold_lua(lua_State* L)
{
    c2Circle A = *getPtr<c2Circle>(L, "c2Circle", 1);
    c2Capsule B = *getPtr<c2Capsule>(L, "c2Capsule", 2);
    c2Manifold m;

    c2CircletoCapsuleManifold(A, B, &m);
    lua_pushmanifold(L, m);
    return 9;
}

int c2AABBtoAABBManifold_lua(lua_State* L)
{
    c2AABB A = *getPtr<c2AABB>(L, "c2AABB", 1);
    c2AABB B = *getPtr<c2AABB>(L, "c2AABB", 2);
    c2Manifold m;

    c2AABBtoAABBManifold(A, B, &m);
    lua_pushmanifold(L, m);
    return 9;
}

int c2AABBtoCapsuleManifold_lua(lua_State* L)
{
    c2AABB A = *getPtr<c2AABB>(L, "c2AABB", 1);
    c2Capsule B = *getPtr<c2Capsule>(L, "c2Capsule", 2);
    c2Manifold m;

    c2AABBtoCapsuleManifold(A, B, &m);
    lua_pushmanifold(L, m);
    return 9;
}

int c2CapsuletoCapsuleManifold_lua(lua_State* L)
{
    c2Capsule A = *getPtr<c2Capsule>(L, "c2Capsule", 1);
    c2Capsule B = *getPtr<c2Capsule>(L, "c2Capsule", 2);
    c2Manifold m;

    c2CapsuletoCapsuleManifold(A, B, &m);
    lua_pushmanifold(L, m);
    return 9;
}

int c2CircletoPolyManifold_lua(lua_State* L)
{
    c2Circle A = *getPtr<c2Circle>(L, "c2Circle", 1);
    c2Poly* B = getPtr<c2Poly>(L, "c2Poly", 2);
    c2x* bx;
    c2Manifold m;

    c2CircletoPolyManifold(A, B, bx, &m);
    lua_pushmanifold(L, m);
    return 9;
}

int c2AABBtoPolyManifold_lua(lua_State* L)
{
    c2AABB A = *getPtr<c2AABB>(L, "c2AABB", 1);
    c2Poly* B = getPtr<c2Poly>(L, "c2Poly", 2);
    c2x bx = checkTransform(L, 3);
    c2Manifold m;

    c2AABBtoPolyManifold(A, B, &bx, &m);
    lua_pushmanifold(L, m);
    return 9;
}

int c2CapsuletoPolyManifold_lua(lua_State* L)
{
    c2Capsule A = *getPtr<c2Capsule>(L, "c2Capsule", 1);
    c2Poly* B = getPtr<c2Poly>(L, "c2Poly", 2);
    c2x bx = checkTransform(L, 3);
    c2Manifold m;

    c2CapsuletoPolyManifold(A, B, &bx, &m);
    lua_pushmanifold(L, m);
    return 9;
}

int c2PolytoPolyManifold_lua(lua_State* L)
{
    c2Poly* A = getPtr<c2Poly>(L, "c2Poly", 1);
    c2x ax = checkTransform(L, 2);
    c2Poly* B = getPtr<c2Poly>(L, "c2Poly", 3);
    c2x bx = checkTransform(L, 4);
    c2Manifold m;

    c2PolytoPolyManifold(A, &ax, B, &bx, &m);
    lua_pushmanifold(L, m);
    return 9;
}

int c2Inflate_lua(lua_State* L)
{
    GShape A = getShape(L, 1);
    C2_TYPE type = (C2_TYPE)luaL_checkinteger(L, 2);
    float skin_factor = luaL_checknumber(L, 3);

    c2Inflate(A.instance, type, skin_factor);
    g_pushInstance(L, A.name, A.instance);
    return 1;
}

int c2Norms_lua(lua_State* L)
{
    LUA_THROW_ERROR("Not implemented!");
    //c2v* verts;
    //c2v* norms;
    //int count;

    //c2Norms(verts, norms, count);
    //
    return 0;
}

int c2MakePoly_lua(lua_State* L)
{
    LUA_THROW_ERROR("Not implemented!");
    //c2Poly* p;

    //c2MakePoly(p);
    //
    return 0;
}

int c2GJK_lua(lua_State* L)
{
    GShape A = getShape(L, 1);
    C2_TYPE typeA = (C2_TYPE)luaL_checkinteger(L, 2);
    c2x ax = checkTransform(L, 3);
    GShape B = getShape(L, 4);
    C2_TYPE typeB = (C2_TYPE)luaL_checkinteger(L, 5);;
    c2x bx = checkTransform(L, 6);
    c2v outA;
    c2v outB;
    int use_radius = lua_toboolean(L, 6);
    int iterations;
    //c2GJKCache* cache;

    float result = c2GJK(
                A.instance, typeA, &ax,
                B.instance, typeB, &bx,
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
    GShape A = getShape(L, 1);
    C2_TYPE typeA = (C2_TYPE)luaL_checkinteger(L, 2);
    c2x ax = checkTransform(L, 3);
    c2v vA = c2V(luaL_checknumber(L, 4), luaL_checknumber(L, 5));
    GShape B = getShape(L, 6);
    C2_TYPE typeB = (C2_TYPE)luaL_checkinteger(L, 7);;
    c2x bx = checkTransform(L, 8);
    c2v vB = c2V(luaL_checknumber(L, 9), luaL_checknumber(L, 10));
    int use_radius = lua_toboolean(L, 11);
    int iterations;

    float result = c2TOI(
                A.instance, typeA, &ax, vA,
                B.instance, typeB, &bx, vB,
                use_radius,
                &iterations);
    lua_pushnumber(L, result);
    lua_pushnumber(L, iterations);
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
        {"setPosition", SetCirclePosition},
        {"getPosition", GetCirclePosition},
        {"setRadius", SetCircleRadius},
        {"getRadius", GetCircleRadius},
        {NULL, NULL}
    };
    g_createClass(L, "c2Circle", NULL, NULL, NULL, circleFunctionsList);

    const luaL_Reg aabbFunctionsList[] = {
        {"setPosition", SetAABBPosition},
        {"getPosition", GetAABBPosition},
        {"setSize", SetAABBSize},
        {"getSize", GetAABBSize},
        {"getRect", GetAABBRect},
        {NULL, NULL}
    };
    g_createClass(L, "c2AABB", NULL, NULL, NULL, aabbFunctionsList);

    const luaL_Reg capsuleFunctionsList[] = {
        {"setPosition", SetCapsulePosition},
        {"getPosition", GetCapsulePosition},
        {"setHeight", SetCapsuleHeight},
        {"getHeight", GetCapsuleHeight},
        {"setRadius", SetCapsuleRadius},
        {"getRadius", GetCapsuleRadius},
        {"getData", GetCapsuleData},
        {NULL, NULL}
    };
    g_createClass(L, "c2Capsule", NULL, NULL, NULL, capsuleFunctionsList);

    const luaL_Reg polyFunctionsList[] = {
        {"updatePoints", UpdatePoints},
        {"updatePoint", UpdatePoint},
        {"getPoints", GetPoints},
        {NULL, NULL}
    };
    g_createClass(L, "c2Poly", NULL, NULL, NULL, polyFunctionsList);

    const luaL_Reg rayFunctionsList[] = {
        {NULL, NULL}
    };
    g_createClass(L, "c2Ray", NULL, NULL, NULL, rayFunctionsList);

    const luaL_Reg c2xFunctionsList[] = {
        {"setPosition", SetTransformPosition},
        {"getPosition", GetTransformPosition},
        {"move", MoveTransformPosition},

        {"setRotation", SetTransformRotation},
        {"getRotation", GetTransformRotation},
        {"rotate", RotateTransform},

        {NULL, NULL}
    };
    g_createClass(L, "c2x", NULL, NULL, NULL, c2xFunctionsList);

    const luaL_Reg cuteC2FunctionsList[] = {
        {"circle", createCircle},
        {"aabb", createAABB},
        {"capsule", createCapsule},
        {"poly", createPoly},
        {"ray", createRay},
        {"transform", createTransform},

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
        {"rayToCircle", c2RaytoCircle_lua},
        {"rayToAABB", c2RaytoAABB_lua},
        {"rayToCapsule", c2RaytoCapsule_lua},
        {"rayToPoly", c2RaytoPoly_lua},
        {"castRay", c2CastRay_lua},
        {"hull", c2Hull_lua},
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

        {"inflate", c2Inflate_lua},
        {"norms", c2Norms_lua},
        {"makePoly", c2MakePoly_lua},
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

    lua_pushcfunction(L, cute_c2_impl::loader);
    lua_setfield(L, -2, PLUGIN_NAME);

    lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State* _UNUSED(L)) { }

#ifdef IS_BETA_BUILD
REGISTER_PLUGIN_NAMED(PLUGIN_NAME, "1.0.0", CuteC2_beta)
#else
REGISTER_PLUGIN_NAMED(PLUGIN_NAME, "1.0.0", CuteC2)
#endif
