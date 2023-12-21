#pragma once

namespace Sen::Kernel::Interface {

	/**
	 * Call back method
	*/

	enum CliCallBack
	{
		NULL_OPERATOR,
		MD5_HASH,
		SHA224_HASH,
		SHA256_HASH,
		BASE64_ENCODE,
		BASE64_DECODE,
		ZLIB_COMPRESS,
		ZLIB_UNCOMPRESS,
		GZIP_COMPRESS,
		GZIP_UNCOMPRESS,
		IMAGE_SCALE,
		LZMA_COMPRESS,
		LZMA_UNCOMPRESS,
		BZIP2_COMPRESS,
		BZIP2_UNCOMPRESS,
		RESOURCE_GROUP_SPLIT,
		RESOURCE_GROUP_MERGE,
		RES_INFO_SPLIT,
		RES_INFO_MERGE,
		RESOURCE_GROUP_TO_RES_INFO,
		RES_INFO_TO_RESOURCE_GROUP,
		TEXTURE_ENCODE,
		TEXTURE_DECODE,
		POPCAP_ZLIB_COMPRESS,
		POPCAP_ZLIB_UNCOMPRESS,
		ATLAS_UNPACK,
		ATLAS_PACK,
		RTON_DECODE,
		RTON_ENCODE,
		RTON_DECRYPT,
		RTON_ENCRYPT,
		RTON_DECRYPT_AND_DECODE,
		RTON_ENCODE_AND_ENCRYPT,
		NEWTON_DECODE,
		NEWTON_ENCODE,
		COMPILED_TEXT_DECODE,
		COMPILED_TEXT_ENCODE,
	};

	/**
	 * Color picker
	*/

	enum Color {
		GREEN = 10,
		CYAN = 11,
		RED = 12,
		YELLOW = 14,
	};
	
}