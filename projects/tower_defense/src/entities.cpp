/*
    File: entities.cpp
    Author: Philip Haynes
*/

#include "entities.hpp"
#include "globals.hpp"

namespace Entities {

const char *circle_tga = "circle.tga";

void Manager::EventAssetInit() {
    globals.assets.filesToLoad.Append(circle_tga);
}

void Manager::EventAssetAcquire() {
    texCircle = globals.assets.FindMapping(circle_tga);
}

void Manager::EventInitialize() {
    entities.Resize(3);
    entities[0].physical.type = SEGMENT;
    entities[0].physical.basis.segment.a = vec2(-64.0, -16.0);
    entities[0].physical.basis.segment.b = vec2(64.0, -14.0);
    // entities[0].physical.type = CIRCLE;
    // entities[0].physical.basis.circle.c = vec2(-64.0, 0.0);
    // entities[0].physical.basis.circle.r = 64.0;
    entities[0].physical.vel = vec2(0.0);
    entities[0].physical.rot = 0.0;
    entities[0].physical.angle = 0.0;

    entities[1].physical.type = CIRCLE;
    entities[1].physical.basis.circle.c = vec2(64.0, 0.0);
    entities[1].physical.basis.circle.r = 64.0;

    entities[2].physical.type = BOX;
    entities[2].physical.basis.box.a = vec2(-64.0, -64.0);
    entities[2].physical.basis.box.b = vec2(128.0, 64.0);

    for (i32 i = 0; i < 3; i++) {
        entities[i].index = i;
        entities[i].physical.pos = vec2(random(0.0, 1280.0, globals.rng), random(0.0, 720.0, globals.rng));
        entities[i].physical.vel = vec2(0.0);// vec2(random(-15.0, 15.0, globals.rng), random(-15.0, 15.0, globals.rng));
        entities[i].physical.rot = 0.0;// random(-0.2, 0.2, globals.rng);
    }
}

void Manager::EventUpdate() {
    if (globals.objects.Pressed(KC_KEY_R)) {
        for (i32 i = 0; i < 3; i++) {
            entities[i].physical.pos = vec2(random(0.0, 1280.0, globals.rng), random(0.0, 720.0, globals.rng));
            // entities[i].physical.vel = vec2(random(-15.0, 15.0, globals.rng), random(-15.0, 15.0, globals.rng));
            // entities[i].physical.rot = random(-0.2, 0.2, globals.rng);
        }
    }
    if (globals.objects.Pressed(KC_KEY_1)) {
        selectedEntity = 0;
    }
    if (globals.objects.Pressed(KC_KEY_2)) {
        selectedEntity = 1;
    }
    if (globals.objects.Pressed(KC_KEY_3)) {
        selectedEntity = 2;
    }
    entities[selectedEntity].physical.pos = vec2(globals.input.cursor.x, globals.input.cursor.y);
    if (globals.objects.Down(KC_KEY_LEFT)) {
        entities[selectedEntity].physical.rot = pi;
    } else if (globals.objects.Down(KC_KEY_RIGHT)) {
        entities[selectedEntity].physical.rot = -pi;
    } else {
        entities[selectedEntity].physical.rot = 0.0;
    }
    for (i32 i = 0; i < entities.size; i++) {
        entities[i].Update(globals.objects.timestep);
    }
}

void Manager::EventDraw(VkCommandBuffer commandBuffer) {
    globals.rendering.BindPipeline2D(commandBuffer);
    for (i32 i = 0; i < entities.size; i++) {
        entities[i].Draw(commandBuffer);
    }
}

bool AABB::Collides(const AABB &other) const {
    return minPos.x <= other.maxPos.x
        && maxPos.x >= other.minPos.x
        && minPos.y <= other.maxPos.y
        && maxPos.y >= other.minPos.y;
}

void AABB::Update(const Physical &physical) {
    switch (physical.type) {
    case SEGMENT: {
        minPos.x = min(physical.actual.segment.a.x, physical.actual.segment.b.x);
        minPos.y = min(physical.actual.segment.a.y, physical.actual.segment.b.y);
        maxPos.x = max(physical.actual.segment.a.x, physical.actual.segment.b.x);
        maxPos.y = max(physical.actual.segment.a.y, physical.actual.segment.b.y);
        break;
    }
    case CIRCLE: {
        minPos.x = physical.actual.circle.c.x - physical.actual.circle.r;
        minPos.y = physical.actual.circle.c.y - physical.actual.circle.r;
        maxPos.x = physical.actual.circle.c.x + physical.actual.circle.r;
        maxPos.y = physical.actual.circle.c.y + physical.actual.circle.r;
        break;
    }
    case BOX: {
        minPos.x = min(physical.actual.box.a.x,
                       min(physical.actual.box.b.x,
                           min(physical.actual.box.c.x,
                               physical.actual.box.d.x)));
        minPos.y = min(physical.actual.box.a.y,
                       min(physical.actual.box.b.y,
                           min(physical.actual.box.c.y,
                               physical.actual.box.d.y)));
        maxPos.x = max(physical.actual.box.a.x,
                       max(physical.actual.box.b.x,
                           max(physical.actual.box.c.x,
                               physical.actual.box.d.x)));
        maxPos.y = max(physical.actual.box.a.y,
                       max(physical.actual.box.b.y,
                           max(physical.actual.box.c.y,
                               physical.actual.box.d.y)));
        break;
    }
    }
}

bool CollisionSegmentSegment(const Physical &a, const Physical &b) {
    const vec2 A = a.actual.segment.a;
    const vec2 B = a.actual.segment.b;
    const vec2 C = b.actual.segment.a;
    const vec2 D = b.actual.segment.b;

    f32 denom, num1, num2;
    denom = ((B.x - A.x) * (D.y - C.y)) - ((B.y - A.y) * (D.x - C.x));
    num1  = ((A.y - C.y) * (D.x - C.x)) - ((A.x - C.x) * (D.y - C.y));
    num2  = ((A.y - C.y) * (B.x - A.x)) - ((A.x - C.x) * (B.y - A.y));

    if (denom == 0) return num1 == 0 && num2 == 0;

    f32 r = num1/denom;
    f32 s = num2/denom;

    return (r >= 0 && r <= 1 && s >= 0 && s <= 1);
}

bool CollisionSegmentCircle(const Physical &a, const Physical &b) {
    const vec2 A = a.actual.segment.a;
    const vec2 B = a.actual.segment.b;
    const vec2 C = b.actual.circle.c;
    vec2 diff = A-B;
    const f32 lengthSquared = absSqr(diff);
    const f32 t = dot(diff, A-C) / lengthSquared;
    vec2 projection;
    if (t < 0.0) {
        projection = A;
    } else if (t > 1.0) {
        projection = B;
    } else {
        projection = A - diff * t;
    }
    f32 dist = absSqr(C - projection);
    return dist <= square(b.actual.circle.r);
}

bool CollisionSegmentBox(const Physical &a, const Physical &b) {
    const mat2 aRotation = mat2::Rotation(-b.angle.value());
    // const vec2 dPos = a.pos - b.pos;
    const vec2 A = (a.actual.segment.a - b.pos) * aRotation;
    if (A.x == median(A.x, b.basis.box.a.x, b.basis.box.b.x) && A.y == median(A.y, b.basis.box.a.y, b.basis.box.b.y)) {
        // Point is inside box
        return true;
    }
    const vec2 B = (a.actual.segment.b - b.pos) * aRotation;
    if (B.x == median(B.x, b.basis.box.a.x, b.basis.box.b.x) && B.y == median(B.y, b.basis.box.a.y, b.basis.box.b.y)) {
        // Point is inside box
        return true;
    }
    f32 t = (b.basis.box.a.y - A.y) / (B.y - A.y);
    if (t == median(t, 0.0, 1.0)) {
        f32 x = A.x + (B.x - A.x) * t;
        if (x == median(x, b.basis.box.a.x, b.basis.box.b.x)) {
            // Segment intersects
            return true;
        }
    }
    t = (b.basis.box.b.y - A.y) / (B.y - A.y);
    if (t == median(t, 0.0, 1.0)) {
        f32 x = A.x + (B.x - A.x) * t;
        if (x == median(x, b.basis.box.a.x, b.basis.box.b.x)) {
            // Segment intersects
            return true;
        }
    }
    t = (b.basis.box.a.x - A.x) / (B.x - A.x);
    if (t == median(t, 0.0, 1.0)) {
        f32 y = A.y + (B.y - A.y) * t;
        if (y == median(y, b.basis.box.a.y, b.basis.box.b.y)) {
            // Segment intersects
            return true;
        }
    }
    t = (b.basis.box.b.x - A.x) / (B.x - A.x);
    if (t == median(t, 0.0, 1.0)) {
        f32 y = A.y + (B.y - A.y) * t;
        if (y == median(y, b.basis.box.a.y, b.basis.box.b.y)) {
            // Segment intersects
            return true;
        }
    }
    // No intersection
    return false;
}

bool CollisionCircleCircle(const Physical &a, const Physical &b) {
    return absSqr(a.actual.circle.c - b.actual.circle.c) <= square(a.actual.circle.r + b.actual.circle.r);
}

bool CollisionCircleBox(const Physical &a, const Physical &b) {
    // TODO: this
    return false;
}

bool CollisionBoxBox(const Physical &a, const Physical &b) {
    // TODO: this
    return false;
}

bool Physical::Collides(const Physical &other) const {
    if (!updated) {
        UpdateActual();
    }
    if (!other.updated) {
        other.UpdateActual();
    }
    // return aabb.Collides(other.aabb);
    if (!aabb.Collides(other.aabb)) {
        return false;
    }
    switch (type) {
    case SEGMENT: {
        switch (other.type) {
        case SEGMENT: {
            return CollisionSegmentSegment(*this, other);
        }
        case CIRCLE: {
            return CollisionSegmentCircle(*this, other);
        }
        case BOX: {
            return CollisionSegmentBox(*this, other);
        }
        }
        break;
    }
    case CIRCLE: {
        switch (other.type) {
        case SEGMENT: {
            return CollisionSegmentCircle(other, *this);
        }
        case CIRCLE: {
            return CollisionCircleCircle(*this, other);
        }
        case BOX: {
            return CollisionCircleBox(*this, other);
        }
        }
        break;
    }
    case BOX: {
        switch (other.type) {
        case SEGMENT: {
            return CollisionSegmentBox(other, *this);
        }
        case CIRCLE: {
            return CollisionCircleBox(other, *this);
        }
        case BOX: {
            return CollisionBoxBox(*this, other);
        }
        }
        break;
    }
    }
    return false;
}

void PhysicalAbsFromBasis(PhysicalAbs &actual, const PhysicalBasis &basis, const CollisionType &type, const vec2 &pos, const Angle32 &angle) {
    mat2 rotation;
    if (angle != 0.0) {
        rotation = mat2::Rotation(angle.value());
    }
    switch (type) {
    case SEGMENT: {
        if (angle != 0.0) {
            actual.segment.a = basis.segment.a * rotation + pos;
            actual.segment.b = basis.segment.b * rotation + pos;
        } else {
            actual.segment.a = basis.segment.a + pos;
            actual.segment.b = basis.segment.b + pos;
        }
        break;
    }
    case CIRCLE: {
        if (angle != 0.0) {
            actual.circle.c = basis.circle.c * rotation + pos;
        } else {
            actual.circle.c = basis.circle.c + pos;
        }
        actual.circle.r = basis.circle.r;
        break;
    }
    case BOX: {
        if (angle != 0.0) {
            actual.box.a = basis.box.a * rotation + pos;
            actual.box.b = basis.box.b * rotation + pos;
            actual.box.c = vec2(basis.box.b.x, basis.box.a.y) * rotation + pos;
            actual.box.d = vec2(basis.box.a.x, basis.box.b.y) * rotation + pos;
        } else {
            actual.box.a = basis.box.a + pos;
            actual.box.b = basis.box.b + pos;
            actual.box.c = vec2(basis.box.b.x, basis.box.a.y) + pos;
            actual.box.d = vec2(basis.box.a.x, basis.box.b.y) + pos;
        }
        break;
    }
    }
}

void Physical::Update(f32 timestep) {
    angle += rot * timestep;
    pos += vel * timestep;
    updated = false;
}

void Physical::UpdateActual() const {
    PhysicalAbsFromBasis(actual, basis, type, pos, angle);
    aabb.Update(*this);
    updated = true;
}

void Entity::Update(f32 timestep) {
    physical.Update(timestep);
    colliding = false;
    for (i32 i = 0; i < globals.entities.entities.size; i++) {
        if (i == index) continue;
        Entity &other = globals.entities.entities[i];
        if (physical.Collides(other.physical)) {
            colliding = true;
        }
    }
}

void Entity::Draw(VkCommandBuffer commandBuffer) {
    vec4 color;
    if (colliding) {
        color = vec4(1.0, 0.0, 0.0, 0.8);
    } else {
        color = vec4(1.0, 1.0, 1.0, 0.8);
    }
    if (physical.type == BOX) {
        const vec2 scale = physical.basis.box.b - physical.basis.box.a;
        globals.rendering.DrawQuad(commandBuffer, Rendering::texBlank, color, physical.pos, scale, vec2(1.0), -physical.basis.box.a / scale, physical.angle);
    } else if (physical.type == SEGMENT) {
        vec2 scale = physical.basis.segment.b - physical.basis.segment.a;
        scale.y = max(scale.y, 2.0);
        globals.rendering.DrawQuad(commandBuffer, Rendering::texBlank, color, physical.pos, scale, vec2(1.0), -physical.basis.segment.a / scale, physical.angle);
    } else {
        const vec2 scale = physical.basis.circle.r * 2.0;
        globals.rendering.DrawQuad(commandBuffer, globals.entities.texCircle, color, physical.pos, scale, vec2(1.0), -physical.basis.circle.c / scale + vec2(0.5), physical.angle);
    }
}

} // namespace Objects
