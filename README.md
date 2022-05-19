# cute_c2 LUA binding for [Gideros mobile](http://giderosmobile.com/)
[cute_c2](https://github.com/RandyGaul/cute_headers)

# API
```lua
require "CuteC2"
```

## Global functions

### Objects
```lua
-- x, y (numbers): position 
-- r (number): radius
Circle = CuteC2.circle(x, y, r)

-- minX, minY (numbers): top left corner position 
-- maxX, maxY (numbers): bottom right corner position
-- anchor point is in top left corner
AABB = CuteC2.aabb(minX, minY, maxX, maxY)

-- x, y (numbers): position 
-- h (number): height
-- r (number): radius
Capsule = CuteC2.capsule(x, y, h, r)

-- points (table): table of x/y pairs (example: {0,0, 100,50, 60,30, 40,90})
-- !!!IMPORTANT NOTE!!! maximum amount of points pairs is 8, everything beyond this limit is ignored
Poly = CuteC2.poly(points)

-- x1, y1 (numbers): start position
-- x2, y2 (numbers): destenation position (gets normalized)
-- len (number): ray lenght (default: distance between (x1, y1) and (x2, y2))
Ray = CuteC2.ray(x1, y1, x2, y2 [, len])

-- x, y (numbers): position (default: (0, 0))
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
-- hit (bool): true if shapes were touching at the TOI, false if they never hit.
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
Circle:setX(x)
x = Circle:getX()
Circle:setY(y)
y = Circle:getY()
Circle:setRadius(radius)
radius = Circle:getRadius()
minX, minY, maxX, maxY = Circle:getBoundingBox()
x, y, radius = Circle:getData()
Circle:move(dx, dy)
Circle:inflate(skin_factor)
bool = Circle:hitTest(x, y)
result, normalX, normalY, t = Circle:rayTest(ray [, transform])
result, normalX, normalY, t = Circle:rayTest(rayX1, rayY1, rayX2, rayY2, rayLen [, transform])
```

## AABB
```lua
-- x, y (number): top left corner position
AABB:setMinPosition(x, y)
x, y = AABB:getMinPosition()
-- x, y (number): bottom right corner position
AABB:setMaxPosition(x, y)
x, y = AABB:getMaxPosition()
-- x, y (number): center position
AABB:setPosition(x, y)
x, y = AABB:getPosition()
AABB:setMinX(x)
x = AABB:getMinX()
AABB:setMinY(y)
y = AABB:getMinY()
AABB:setMaxX(x)
x = AABB:getMaxX()
AABB:setMaxY(y)
y = AABB:getMaxY()
AABB:setX(x)
x = AABB:getX()
AABB:setY(y)
y = AABB:getY()
-- w, h (number): full size (relative to shape center)
AABB:setSize(w, h)
w, h = AABB:getSize()
-- w, h (number): half size (relative to shape center)
AABB:setHalfSize(w, h)
w, h = AABB:getHalfSize()
-- w (number): full width (relative to shape center)
AABB:setWidth(w)
w = AABB:getWidth()
-- h (number): full height (relative to shape center)
AABB:setHeight(h)
h = AABB:getHeight()
-- w (number): half width (relative to shape center)
AABB:setHalfWidth(w)
w = AABB:getHalfWidth()
-- h (number): half height (relative to shape center)
AABB:setHalfHeight(h)
h = AABB:getHalfHeight()

minX, minY, maxX, maxY = AABB:getBoundingBox()
minX, minY, maxX, maxY, fullW, fullH = AABB:getData()
AABB:move(dx, dy)
AABB:inflate(skin_factor)
bool = AABB:hitTest(x, y)
result, normalX, normalY, t = AABB:rayTest(ray [, transform])
result, normalX, normalY, t = AABB:rayTest(rayX1, rayY1, rayX2, rayY2, rayLen [, transform])
```

## Capsule
```lua
         , - ~ ~ ~ - ,-------------.-
     , '               ' ,         |
   ,                       ,       |  Radius
  ,                         ,      |
 ,- - - - - - -X-<Tip - - - -,-----|-
 ,                           ,     |
 ,                           ,     |
 ,                           ,     |
 ,             X < Center    ,     | Height
 ,                           ,     |
 ,                           ,     |
 ,             |             ,     |
 ,- - - - - - -X-<Base- - - -,-----|-
  ,                         ,      |
   ,                       ,       | Radius
     ,                  , '        |
       ' - , _ _ _ ,  '_ _ _ _ _ _ |_
```

```lua
-- x, y (number): center position
Capsule:setPosition(x, y)
x, y = Capsule:getPosition()
-- x, y (number): position of the top point
Capsule:setTipPosition(x, y)
x, y = Capsule:getTipPosition()
-- x, y (number): position of the bottom point
Capsule:setBasePosition(x, y)
x, y = Capsule:getBasePosition()
Capsule:setX(x)
x = Capsule:getX()
Capsule:setY(y)
y = Capsule:getY()
Capsule:setTipX(x)
x = Capsule:getTipX()
Capsule:setTipY(y)
y = Capsule:getTipY()
Capsule:setBaseX(x)
x = Capsule:getBaseX()
Capsule:setBaseY(y)
y = Capsule:getBaseY()
-- h (number): height
Capsule:setHeight(h)
h = Capsule:getHeight()
-- r (number): radius
Capsule:setRadius(r)
r = Capsule:getRadius()
Capsule:setSize(radius, height)
radius, height = Capsule:getSize()
minX, minY, maxX, maxY = Capsule:getBoundingBox()
tipX, tipY, baseX, baseY, radius = Capsule:getData()
Capsule:move(dx, dy)
Capsule:inflate(skin_factor)
bool = Capsule:hitTest(x, y)
result, normalX, normalY, t = Capsule:rayTest(ray [, transform])
result, normalX, normalY, t = Capsule:rayTest(rayX1, rayY1, rayX2, rayY2, rayLen [, transform])
```

## Poly
```lua
-- points (table): table of x/y pairs (example: {0,0, 100,50, 60,30, 40,90})
Poly:setPoints(points)
Poly:updatePoint(index, x, y)
table = Poly:getPoints()
table = Poly:getNormals()
Poly:inflate(skin_factor)
bool = Poly:hitTest(x, y [, transform])
minX, minY, maxX, maxY, w, h = Poly:getBoundingBox()
-- return a table with all points & normals: {points = {...}, normals = {...}}
table = Poly:getData()
Poly:inflate(skin_factor)
result, normalX, normalY, t = Poly:rayTest(ray [, transform])
result, normalX, normalY, t = Poly:rayTest(rayX1, rayY1, rayX2, rayY2, rayLen [, transform])
```

## Ray
```lua
ray:setStartPosition(x, y)
x, y = Ray:getStartPosition()
-- x, y (number): destenation point (gets normalized, and changed to a direction vector)
ray:setTargetPosition(x, y)
x, y = Ray:getTargetPosition()
Ray:setStartX(x)
x = Ray:getStartX()
Ray:setStartY(y)
y = Ray:getStartY()
x = Ray:getTargetX()
y = Ray:getTargetY()
Ray:setLength(len)
len = Ray:getLength()
Ray:setDirection(radians)
radians = Ray:getDirection()
Ray:move(dx, dy)
startX, startY, directionX, directionY, len = Poly:getData()
```

## Transform
```lua
Transform:setPosition(x, y)
x, y = Transform:getPosition()
Transform:setX(x)
x = Transform:getX()
Transform:setY(y)
y = Transform:getY()
Transform:move(deltaX, deltaY)
Transform:setRotation(radians)
radians = Transform:getRotation()
Transform:rotate(amount_in_radians)
Transform:move(dx, dy)
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
