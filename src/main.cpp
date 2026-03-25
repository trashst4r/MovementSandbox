#include "raylib.h"
#include "raymath.h"
#include <algorithm>
#include <cmath>
#include <vector>

static constexpr float SURFACE_EPSILON = 0.01f;

struct Player {
    Vector3 pos{};
    Vector3 vel{};
    float yaw = 0.0f;
    float pitch = 0.0f;
    bool grounded = false;
    bool prevGrounded = false;
};

struct Block {
    Vector3 center;
    Vector3 size;
    Color color;
};

struct MovementConfig {
    float gravity = 20.0f;
    float jumpVelocity = 8.0f;
    float groundAccel = 50.0f;
    float airAccel = 12.0f;
    float friction = 8.0f;
    float maxSpeed = 7.0f;
    float playerHalfWidth = 0.3f;
    float playerHeight = 1.8f;
    float eyeHeight = 1.6f;
};

struct AccelResult {
    float projectedSpeed = 0.0f;
    float addSpeed = 0.0f;
    float accelSpeed = 0.0f;
};

struct DebugFrameData {
    Vector3 wishDir{0.0f, 0.0f, 0.0f};
    float wishSpeed = 0.0f;
    float horizontalSpeed = 0.0f;
    float verticalSpeed = 0.0f;
    AccelResult accel{};
    bool grounded = false;
    bool prevGrounded = false;
    bool jumpIntent = false;
    bool jumpTriggered = false;
};

struct JumpInputState {
    bool holdSpace = false;
    bool pressedSpace = false;
    bool scrollDown = false;
    bool triggered = false;
};

static float Dot2D(const Vector3& a, const Vector3& b) {
    return a.x * b.x + a.z * b.z;
}

static float Length2D(const Vector3& v) {
    return std::sqrtf(v.x * v.x + v.z * v.z);
}

static Vector3 Normalize2D(const Vector3& v) {
    float len = Length2D(v);
    if (len <= 0.0001f) return {0.0f, 0.0f, 0.0f};
    return {v.x / len, 0.0f, v.z / len};
}

static Vector3 GetForward(float yaw) {
    return Vector3Normalize({std::sinf(yaw), 0.0f, std::cosf(yaw)});
}

static Vector3 GetRight(float yaw) {
    Vector3 f = GetForward(yaw);
    return {-f.z, 0.0f, f.x};
}

static Vector3 ComputeWishDir(float yaw) {
    Vector3 wish{};
    Vector3 forward = GetForward(yaw);
    Vector3 right = GetRight(yaw);

    if (IsKeyDown(KEY_W)) wish = Vector3Add(wish, forward);
    if (IsKeyDown(KEY_S)) wish = Vector3Subtract(wish, forward);
    if (IsKeyDown(KEY_D)) wish = Vector3Add(wish, right);
    if (IsKeyDown(KEY_A)) wish = Vector3Subtract(wish, right);

    return Normalize2D(wish);
}

static AccelResult Accelerate(Vector3& vel, const Vector3& wishDir, float wishSpeed, float accel, float dt) {
    AccelResult result{};
    float projected = Dot2D(vel, wishDir);
    float addSpeed = wishSpeed - projected;
    if (addSpeed <= 0.0f || wishDir.x == 0.0f && wishDir.z == 0.0f) {
        result.projectedSpeed = projected;
        return result;
    }

    float accelSpeed = accel * dt * wishSpeed;
    if (accelSpeed > addSpeed) accelSpeed = addSpeed;

    vel.x += wishDir.x * accelSpeed;
    vel.z += wishDir.z * accelSpeed;

    result.projectedSpeed = projected;
    result.addSpeed = addSpeed;
    result.accelSpeed = accelSpeed;
    return result;
}

static void ApplyFriction(Vector3& vel, float dt, float friction) {
    float speed = Length2D(vel);
    if (speed < 0.0001f) return;

    float drop = speed * friction * dt;
    float newSpeed = std::max(0.0f, speed - drop);
    float scale = newSpeed / speed;
    vel.x *= scale;
    vel.z *= scale;
}

static BoundingBox MakeBlockBox(const Block& b) {
    Vector3 half = Vector3Scale(b.size, 0.5f);
    return {{b.center.x - half.x, b.center.y - half.y, b.center.z - half.z},
            {b.center.x + half.x, b.center.y + half.y, b.center.z + half.z}};
}

static BoundingBox MakePlayerBox(const Player& p, float halfWidth, float height) {
    return {{p.pos.x - halfWidth, p.pos.y, p.pos.z - halfWidth},
            {p.pos.x + halfWidth, p.pos.y + height, p.pos.z + halfWidth}};
}

static JumpInputState ReadJumpInput() {
    JumpInputState jump{};
    jump.holdSpace = IsKeyDown(KEY_SPACE);
    jump.pressedSpace = IsKeyPressed(KEY_SPACE);
    jump.scrollDown = GetMouseWheelMove() < 0.0f;
    return jump;
}

enum class Axis { X, Y, Z };

static void MoveAxis(Player& player, Axis axis, float dt, const std::vector<Block>& blocks, float halfWidth, float height) {
    float* pos = nullptr;
    float* vel = nullptr;

    switch (axis) {
        case Axis::X: pos = &player.pos.x; vel = &player.vel.x; break;
        case Axis::Y: pos = &player.pos.y; vel = &player.vel.y; break;
        case Axis::Z: pos = &player.pos.z; vel = &player.vel.z; break;
    }

    *pos += *vel * dt;

    if (axis == Axis::Y && player.pos.y < 0.0f) {
        player.pos.y = 0.0f;
        if (*vel < 0.0f) {
            *vel = 0.0f;
            player.grounded = true;
        }
    }

    BoundingBox playerBox = MakePlayerBox(player, halfWidth, height);
    for (const Block& block : blocks) {
        BoundingBox blockBox = MakeBlockBox(block);
        if (!CheckCollisionBoxes(playerBox, blockBox)) continue;

        float overlapY = std::min(playerBox.max.y, blockBox.max.y) - std::max(playerBox.min.y, blockBox.min.y);
        if (axis != Axis::Y && overlapY <= SURFACE_EPSILON) {
            continue;
        }

        if (axis == Axis::X) {
            if (*vel > 0.0f) player.pos.x = blockBox.min.x - halfWidth;
            else if (*vel < 0.0f) player.pos.x = blockBox.max.x + halfWidth;
            *vel = 0.0f;
        } else if (axis == Axis::Z) {
            if (*vel > 0.0f) player.pos.z = blockBox.min.z - halfWidth;
            else if (*vel < 0.0f) player.pos.z = blockBox.max.z + halfWidth;
            *vel = 0.0f;
        } else {
            if (*vel > 0.0f) {
                player.pos.y = blockBox.min.y - height;
            } else if (*vel < 0.0f) {
                player.pos.y = blockBox.max.y;
                player.grounded = true;
            }
            *vel = 0.0f;
        }

        playerBox = MakePlayerBox(player, halfWidth, height);
    }
}

static void UpdateCameraAngles(Player& player) {
    Vector2 mouse = GetMouseDelta();
    player.yaw -= mouse.x * 0.0025f;
    player.pitch -= mouse.y * 0.0025f;
    player.pitch = std::clamp(player.pitch, -1.5f, 1.5f);
}

static Camera3D BuildCamera(const Player& player, float eyeHeight) {
    Vector3 forward = {
        std::sinf(player.yaw) * std::cosf(player.pitch),
        std::sinf(player.pitch),
        std::cosf(player.yaw) * std::cosf(player.pitch)};

    Camera3D cam{};
    cam.position = {player.pos.x, player.pos.y + eyeHeight, player.pos.z};
    cam.target = Vector3Add(cam.position, forward);
    cam.up = {0.0f, 1.0f, 0.0f};
    cam.fovy = 90.0f;
    cam.projection = CAMERA_PERSPECTIVE;
    return cam;
}

static void DrawDebugOverlay(const Player& player, const DebugFrameData& debug) {
    int x = 20;
    int y = 20;
    const int line = 18;

    DrawText("Movement Sandbox 3D", x, y, 24, RAYWHITE);
    y += 30;

    DrawText(TextFormat("Pos: (%.2f, %.2f, %.2f)", player.pos.x, player.pos.y, player.pos.z), x, y, 18, LIGHTGRAY); y += line;
    DrawText(TextFormat("Vel: (%.2f, %.2f, %.2f)", player.vel.x, player.vel.y, player.vel.z), x, y, 18, LIGHTGRAY); y += line;
    DrawText(TextFormat("Horizontal Speed: %.2f", debug.horizontalSpeed), x, y, 18, GREEN); y += line;
    DrawText(TextFormat("Vertical Speed: %.2f", debug.verticalSpeed), x, y, 18, GREEN); y += line;
    DrawText(TextFormat("Grounded: %s", debug.grounded ? "true" : "false"), x, y, 18, YELLOW); y += line;
    DrawText(TextFormat("Prev Grounded: %s", debug.prevGrounded ? "true" : "false"), x, y, 18, YELLOW); y += line;
    DrawText(TextFormat("Wish Dir: (%.2f, %.2f)", debug.wishDir.x, debug.wishDir.z), x, y, 18, SKYBLUE); y += line;
    DrawText(TextFormat("Wish Speed: %.2f", debug.wishSpeed), x, y, 18, SKYBLUE); y += line;
    DrawText(TextFormat("Projected Speed: %.2f", debug.accel.projectedSpeed), x, y, 18, ORANGE); y += line;
    DrawText(TextFormat("Add Speed: %.2f", debug.accel.addSpeed), x, y, 18, ORANGE); y += line;
    DrawText(TextFormat("Accel Speed: %.2f", debug.accel.accelSpeed), x, y, 18, ORANGE); y += line;
    DrawText(TextFormat("Jump Intent: %s", debug.jumpIntent ? "true" : "false"), x, y, 18, PINK); y += line;
    DrawText(TextFormat("Jump Triggered: %s", debug.jumpTriggered ? "true" : "false"), x, y, 18, PINK); y += line;
}

int main() {
    InitWindow(1280, 720, "Movement Sandbox 3D");
    DisableCursor();
    SetTargetFPS(144);

    Player player{};
    MovementConfig config{};

    std::vector<Block> blocks = {
        {{3.0f, 1.0f, 3.0f}, {2.0f, 2.0f, 2.0f}, GRAY},
        {{-4.0f, 0.5f, -3.0f}, {2.0f, 1.0f, 2.0f}, GRAY},
        {{0.0f, 0.5f, 6.0f}, {4.0f, 1.0f, 1.0f}, GRAY},
        {{6.0f, 1.5f, -2.0f}, {2.0f, 3.0f, 2.0f}, GRAY}
    };

    DebugFrameData debug{};

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        player.prevGrounded = player.grounded;
        UpdateCameraAngles(player);
        JumpInputState jumpInput = ReadJumpInput();

        Vector3 wishDir = ComputeWishDir(player.yaw);
        debug.wishDir = wishDir;
        debug.wishSpeed = config.maxSpeed;

        if (player.grounded) {
            ApplyFriction(player.vel, dt, config.friction);
            debug.accel = Accelerate(player.vel, wishDir, config.maxSpeed, config.groundAccel, dt);

            bool wantsJump = jumpInput.scrollDown || jumpInput.pressedSpace || jumpInput.holdSpace;
            if (wantsJump) {
                player.vel.y = config.jumpVelocity;
                player.grounded = false;
                jumpInput.triggered = true;
            }
        } else {
            debug.accel = Accelerate(player.vel, wishDir, config.maxSpeed, config.airAccel, dt);
            player.vel.y -= config.gravity * dt;
        }

        player.grounded = false;

        MoveAxis(player, Axis::Y, dt, blocks, config.playerHalfWidth, config.playerHeight);
        MoveAxis(player, Axis::X, dt, blocks, config.playerHalfWidth, config.playerHeight);
        MoveAxis(player, Axis::Z, dt, blocks, config.playerHalfWidth, config.playerHeight);

        debug.horizontalSpeed = Length2D(player.vel);
        debug.verticalSpeed = player.vel.y;
        debug.grounded = player.grounded;
        debug.prevGrounded = player.prevGrounded;
        debug.jumpIntent = jumpInput.holdSpace || jumpInput.pressedSpace || jumpInput.scrollDown;
        debug.jumpTriggered = jumpInput.triggered;

        Camera3D cam = BuildCamera(player, config.eyeHeight);

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(cam);
        DrawPlane({0.0f, 0.0f, 0.0f}, {100.0f, 100.0f}, DARKGREEN);
        for (const Block& block : blocks) {
            DrawCube(block.center, block.size.x, block.size.y, block.size.z, block.color);
            DrawCubeWires(block.center, block.size.x, block.size.y, block.size.z, DARKGRAY);
        }
        EndMode3D();

        DrawDebugOverlay(player, debug);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
