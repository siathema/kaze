#pragma once

#include <queue>

#include "array.hpp"
#include "camera.hpp"
#include "game.hpp"
#include "mesh.hpp"
#include "sMath.hpp"

namespace SMOBA {
struct Texture2D {
    u32 TextureID;
    i32 Width, Height;

    Texture2D(u8 *image, i32 width, i32 height);
    Texture2D(i32 width, i32 height);
    ~Texture2D();

    void Bind();

    i32 GetWidth() { return Width; }
    i32 GetHeight() { return Height; }
};

enum ShaderType {
    SIMPLEBLIT,
    DEBUGLINES,
    FILLRECTANGLE,
    SIMPLEMESH,
    FINAL,
    SIMPLE3DCOLOR,
    NONE,
    SHADERCOUNT
};

enum RenderType {
    TEXTURERENDER,
    RECTANGLERENDER,
    STRINGRENDER,
    TEXTRECTRENDER,
    MESHRENDER,
    SIMPLE3DDEBUGLINES,
    RENDERCOUNT
};

struct RenderCommand {
    RenderType RenderType;
    ShaderType ShaderType;
    ID Mesh;
    ID Texture;
    iRect TextureRect;
    vec3 Point1;
    vec3 Point2;
    vec4 Color;
    vec3 Pos;
    vec3 Scale;
    quat Quat;
    r32 Rot;
    const char *String;
};

struct Renderer {

    u32 ShaderIDs[7];
    u32 QuadVAO;
    u32 OverlayVAO;
    u32 VBOs[3];
    u32 FrameBuffer, RBO, FrameTexture;
    r32 TexCoords[12];
    Texture2D *FrameBufferTexture;
    ViewportInfo *Viewport;

    Renderer(ViewportInfo *viewportInfo);

    void Render(std::queue<RenderCommand> *renderQueue, Camera *camera);
    void Draw_Text_Rect(const char *str, Camera &camera, r32 x, r32 y,
                        r32 width, r32 height, vec4 color);
    void Render_String(const char *str, Camera &camera, vec2 pos, vec2 scale,
                       r32 rot, vec4 color);
    void Draw_Overlay();
    void Draw_Texture(Texture2D *texture, Camera &camera, iRect &textureRect,
                      vec2 position, vec2 size, r32 rotate, r32 depth,
                      vec4 color, ShaderType shader = SIMPLEBLIT);
    void Draw_Mesh(const Mesh &mesh, Texture2D *texture, Camera &camera,
                   vec3 &pos, vec3 &scale, quat &rot, vec4 &color,
                   ShaderType shader = SIMPLEMESH);
    void Draw_3d_Debug_Line(vec3 point1, vec3 point2, vec4 color,
                            Camera &camera);
    void Init_Render_Data();
    void Load_Shaders();
};
} // namespace SMOBA
