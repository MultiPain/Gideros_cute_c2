# cute_c2 LUA binding for [Gideros mobile](http://giderosmobile.com/)
[cute_c2](https://github.com/RandyGaul/cute_headers)

# API
```lua
require "CuteC2"
```

## Global functions

### Objects
```lua
-- x (number): x position
-- y (number): y position
-- r (number): radius
table = CuteC2.circle(x, y, r)

-- x (number): x position
-- y (number): y position
-- w (number): width
-- h (number): height
-- anchor point is in top left corner
table = CuteC2.aabb(x, y, w, h)

-- x (number): x position
-- y (number): y position
-- h (number): height
-- r (number): radius
table = CuteC2.capsule(x, y, h, r)

-- points (table): table of x/y pairs (example: {0,0, 100,50, 60,30, 40,90})
-- !!!IMPORTANT NOTE!!! maximum amount of points pairs is 8, everything beyond this limit is ignored
table = CuteC2.poly(points)

-- x1 (number): x start position
-- y1 (number): y start position
-- x2 (number): x direction position (must be normalized)
-- y2 (number): y direction position (must be normalized)
-- len (number): ray lenght
table = CuteC2.ray(x1, y1, x2, y2, len)

-- x (number): x position (default: 0)
-- y (number): y position (default: 0)
-- r (number): rotation (default: 0)
table = CuteC2.transform([x, y, r])
```

### Functions
```lua
bool = CuteC2.circleToCircle(circle1, circle2)
bool = CuteC2.circleToAABB(circle, aabb)
bool = CuteC2.circleToCapsule(circle, capsule)
bool = CuteC2.AABBtoAABB(aabb1, aabb2)
bool = CuteC2.AABBtoCapsule(aabb, capsule)
bool = CuteC2.capsuleToCapsule(capsule1, capsule2)
bool = CuteC2.circleToPoly(circle, poly)
bool = CuteC2.AABBtoPoly(aabb, poly)
bool = CuteC2.capsuleToPoly(capsule, poly)
bool = CuteC2.polyToPoly(poly1, poly2)
bbol = CuteC2.collided(object1, object2 [, transform1, transform2])

result, normalX, normalY, t = CuteC2.castRay(ray, object [, transform])
result, normalX, normalY, t = CuteC2.castRay(rayX1, rayY1, rayX2, rayY2, rayLen, object [, transform])

-- 'MAINFOLD' is 9 values: count, depths1, depths2, contact_point1x, contact_point1y, contact_point2x, contact_point2y, normalX, normalY
MAINFOLD = CuteC2.collide(object1, object2 [, transform1, transform2])
MAINFOLD = CuteC2.circleToCircleManifold(circle1, circle2)
MAINFOLD = CuteC2.circleToAABBManifold(circle, aabb)
MAINFOLD = CuteC2.circleToCapsuleManifold(circle, capsule)
MAINFOLD = CuteC2.AABBtoAABBManifold(aabb1, aabb2)
MAINFOLD = CuteC2.AABBtoCapsuleManifold(aabb, capsule)
MAINFOLD = CuteC2.capsuleToCapsuleManifold(capsule1, capsule2)
MAINFOLD = CuteC2.circletoPolyManifold(circle, poly)
MAINFOLD = CuteC2.AABBtoPolyManifold(aabb, poly)
MAINFOLD = CuteC2.capsuleToPolyManifold(capsule, poly)
MAINFOLD = CuteC2.polyToPolyManifold(poly1, poly2)
```
### Advanced functions
Check the [source code](https://github.com/RandyGaul/cute_headers/blob/df3b63e072afa275a72ce8aa7fce0428a5966e0c/cute_c2.h#L342) for more information
```lua
distance, aX, aY, bX, bY, iterations = CuteC2.GJK(object1, object2 [, transform1, transform2])
-- hit (number): 1 if shapes were touching at the TOI, 0 if they never hit.
-- toi (number): The time of impact between two shapes.
-- nx, ny (numbers): Surface normal from shape A to B at the time of impact.
-- px, py (numbers): Point of contact between shapes A and B at time of impact.
-- iterations (number): Number of iterations the solver underwent.
hit, toi, nx, ny, px, py, iterations = CuteC2.TOI(object1, v1x, v1y, object2, v2x, v2y [, use_radians, transform1, transform2])
```

## Circle
```lua
Circle:setPosition(x, y)
x, y = Circle:getPosition()
Circle:setRadius(r)
r = Circle:getRadius()
Circle:rayTest()
result, normalX, normalY, t = Circle:rayTest(ray [, transform])
result, normalX, normalY, t = Circle:rayTest(rayX1, rayY1, rayX2, rayY2, rayLen [, transform])
Circle:inflate(skin_factor)
```

#### Properties
```lua
Circle.x -- get/set
Circle.y -- get/set
Circle.radius -- get/set
```

## AABB
```lua
AABB:setPosition(x, y)
x, y = AABB:getPosition()
AABB:setSize(w, h)
w, h = AABB:getSize()
minX, minY, maxX, maxY = AABB:getRect()
result, normalX, normalY, t = AABB:rayTest(ray [, transform])
result, normalX, normalY, t = AABB:rayTest(rayX1, rayY1, rayX2, rayY2, rayLen [, transform])
AABB:inflate(skin_factor)
```

#### Properties
```lua
AABB.x -- get/set
AABB.y -- get/set
AABB.width -- get/set
AABB.height -- get/set
```

## Capsule
```lua
Capsule:setPosition(x, y)
x, y = Capsule:getPosition()
Capsule:setHeight(h)
h = Capsule:getHeight()
Capsule:setRadius(r)
r = Capsule:getRadius()
x, y, w, h, r = Capsule:getData()
result, normalX, normalY, t = Capsule:rayTest(ray [, transform])
result, normalX, normalY, t = Capsule:rayTest(rayX1, rayY1, rayX2, rayY2, rayLen [, transform])
Capsule:inflate(skin_factor)
```

#### Properties
```lua
Capsule.x -- get/set
Capsule.y -- get/set
Capsule.radius -- get/set
Capsule.height -- get/set
```

## Poly
```lua
-- replaces existing points
Poly:updatePoints(points)
Poly:updatePoint(index, x, y)
table = Poly:getPoints()
result, normalX, normalY, t = Poly:rayTest(ray [, transform])
result, normalX, normalY, t = Poly:rayTest(rayX1, rayY1, rayX2, rayY2, rayLen [, transform])
Poly:inflate(skin_factor)
```

## Transform
```lua
Transform:setPosition(x, y)
x, y = Transform:getPosition()
Transform:move(deltaX, deltaY)
Transform:setRotation(radians)
radians = Transform:getRotation()
Transform:rotate(amount_in_radians)
```

#### Properties
```lua
Transform.x -- get/set
Transform.y -- get/set
Transform.rotation -- get/set
```