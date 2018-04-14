#include "texture.h"
namespace PO::Graphic
{
	bool texture::load(texture_agent& ta, TextureStyle style, std::experimental::filesystem::path name, size_t mipmap)
	{
		scription = {};
		Implement::texture_scription final_result;
		if (ta.direct_load(style, std::move(name), mipmap, final_result))
		{
			scription = std::move(final_result);
			return true;
		}
		else {
			return false;
		}
	}


}