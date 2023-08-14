#pragma once

#include "game.hpp"
#include "mesh.hpp"
#include "meshes.hpp"
#include "renderer.hpp"
#include "textures.hpp"
#include "voxel.hpp"

namespace SMOBA {
namespace ASSETS {
void Load_Assets(ViewportInfo &viewport);
void Unload_Assets();
Texture2D *Get_Texture(ID assetID);
Mesh *Get_Mesh(ID assetID);
extern Array<Texture2D> Textures;
extern Array<Mesh> Meshes;
} // namespace ASSETS
} // namespace SMOBA
