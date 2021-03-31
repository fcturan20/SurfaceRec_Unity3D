#include "Texture_Resource.h"
#include <flatbuffers/flatbuffers.h>

namespace GFX_API {
	Texture_Properties::Texture_Properties() {}
	Texture_Properties::Texture_Properties(TEXTURE_DIMENSIONs dimension, TEXTURE_MIPMAPFILTER mipmap_filtering, TEXTURE_WRAPPING wrapping, TEXTURE_CHANNELs channel_type)
		: DIMENSION(dimension), MIPMAP_FILTERING(mipmap_filtering), WRAPPING(wrapping), CHANNEL_TYPE(channel_type){}

	Texture_Resource::Texture_Resource() {

	}
	Texture_Resource::~Texture_Resource() {
		delete DATA;
	}


}