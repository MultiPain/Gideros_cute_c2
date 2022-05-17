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
Circle = CuteC2.circle(x, y, r)

-- x (number): x position
-- y (number): y position
-- w (number): width
-- h (number): height
-- anchor point is in top left corner
AABB = CuteC2.aabb(x, y, w, h)

-- x (number): x position
-- y (number): y position
-- h (number): height
-- r (number): radius
Capsule = CuteC2.capsule(x, y, h, r)

-- points (table): table of x/y pairs (example: {0,0, 100,50, 60,30, 40,90})
-- !!!IMPORTANT NOTE!!! maximum amount of points pairs is 8, everything beyond this limit is ignored
Poly = CuteC2.poly(points)

-- x1 (number): x start position
-- y1 (number): y start position
-- x2 (number): x direction position (must be normalized)
-- y2 (number): y direction position (must be normalized)
-- len (number): ray lenght
Ray = CuteC2.ray(x1, y1, x2, y2, len)

-- x (number): x position (default: 0)
-- y (number): y position (default: 0)
-- r (number): rotation (default: 0)
Transform = CuteC2.transform([x, y, r])
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

-- t (number): value from raycast result
x, y = CuteC2.impact(ray, t)
x, y = CuteC2.impact(rayX1, rayY1, rayX2, rayY2, rayLen, t)

-- Mainfold (table):
-- {
--		count = number,
--		depth = {number1, number2},
--		contact_points = {
--			{x = number, y = number}, 
--			{x = number, y = number}
--		}, 
--		normal = {x = number, y = number}
-- }
Mainfold = CuteC2.collide(object1, object2 [, transform1, transform2])
Mainfold = CuteC2.circleToCircleManifold(circle1, circle2)
Mainfold = CuteC2.circleToAABBManifold(circle, aabb)
Mainfold = CuteC2.circleToCapsuleManifold(circle, capsule)
Mainfold = CuteC2.AABBtoAABBManifold(aabb1, aabb2)
Mainfold = CuteC2.AABBtoCapsuleManifold(aabb, capsule)
Mainfold = CuteC2.capsuleToCapsuleManifold(capsule1, capsule2)
Mainfold = CuteC2.circletoPolyManifold(circle, poly)
Mainfold = CuteC2.AABBtoPolyManifold(aabb, poly)
Mainfold = CuteC2.capsuleToPolyManifold(capsule, poly)
Mainfold = CuteC2.polyToPolyManifold(poly1, poly2)
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
hit, toi, nx, ny, px, py, iterations = CuteC2.TOI(object1, v1x, v1y, object2, v2x, v2y [, use_radius, transform1, transform2])
```

## Circle
```lua
Circle:setPosition(x, y)
x, y = Circle:getPosition()
Circle:setRadius(r)
r = Circle:getRadius()
result, normalX, normalY, t = Circle:rayTest(ray [, transform])
result, normalX, normalY, t = Circle:rayTest(rayX1, rayY1, rayX2, rayY2, rayLen [, transform])
Circle:inflate(skin_factor)
bool = Circle:hitTest(x, y)
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
bool = AABB:hitTest(x, y)
minX, minY, maxX, maxY, w, h = AABB:getBoundingBox()
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
x1, y1, x2, y2, r = Capsule:getData()
result, normalX, normalY, t = Capsule:rayTest(ray [, transform])
result, normalX, normalY, t = Capsule:rayTest(rayX1, rayY1, rayX2, rayY2, rayLen [, transform])
Capsule:inflate(skin_factor)
bool = Capsule:hitTest(x, y)
minX, minY, maxX, maxY, w, h = Capsule:getBoundingBox()
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
bool = Poly:hitTest(x, y [, transform])
minX, minY, maxX, maxY, w, h = Poly:getBoundingBox()
```

## Ray
```lua
ray:setStartPosition(x, y)
x, y = Ray:getStartPosition()
ray:setTargetPosition(x, y)
x, y = Ray:getTargetPosition()
Ray:setT(number)
t = Ray:getT()
```

#### Properties
```lua
Ray.x -- get/set
Ray.y -- get/set
Ray.t -- get/set (distance along "direction" from position "x, y" to find endpoint of ray)
Ray.direction -- get/set (angle in radians)
Ray.targetX -- get/set
Ray.targetY -- get/set
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


## Shape types
```lua
CuteC2.TYPE_NONE
CuteC2.TYPE_CIRCLE
CuteC2.TYPE_AABB
CuteC2.TYPE_CAPSULE
CuteC2.TYPE_POLY
```

Example:
```lua
circle = CuteC2.circle(100, 200, 15)
print(circle.__shapeType == CuteC2.TYPE_CIRCLE) -- true
```
