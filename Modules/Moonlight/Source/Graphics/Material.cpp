#include "Material.h"
#include "optick.h"
#include "Texture.h"

namespace Moonlight
{
	Material::Material()
		: Textures(TextureType::Count, nullptr)
	{
	}

	Material::~Material()
	{
	}

	void Material::SetTexture(const TextureType& textureType, std::shared_ptr<Moonlight::Texture> loadedTexture)
	{
		Textures[textureType] = loadedTexture;
	}

	const Moonlight::Texture* Material::GetTexture(const TextureType& type) const
	{
		return Textures[type].get();
	}

	std::vector<std::shared_ptr<Moonlight::Texture>>& Material::GetTextures()
	{
		return Textures;
	}
}