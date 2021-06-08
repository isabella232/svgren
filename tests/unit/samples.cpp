#include <tst/set.hpp>
#include <tst/check.hpp>

#include <regex>

#include <png.h>

#include <utki/span.hpp>
#include <r4/vector.hpp>
#include <papki/fs_file.hpp>
#include <svgdom/dom.hpp>

#include "../../src/svgren/config.hxx"
#include "../../src/svgren/render.hpp"

namespace{
const unsigned tolerance = 10;

const std::string data_dir = "samples_data/";

const std::string render_backend_name =
#if SVGREN_BACKEND == SVGREN_BACKEND_CAIRO
    "cairo"
#elif SVGREN_BACKEND == SVGREN_BACKEND_AGG
    "agg"
#else
#   error "Unknown rendering backend"
#endif
;
}

class Image final{
public:
	/**
	 * @brief Image color depth.
	 */
	enum class ColorDepth_e{
		UNKNOWN = 0,
		GREY  = 1, //1 channel. Only Grey channel
		GREYA = 2, //2 channels. Grey with Alpha channel
		RGB   = 3, //3 channels. Red Green Blue channels
		RGBA  = 4  //4 channels. RGBA format (4 channels)
	};

	/**
	 * @brief Basic image exception.
	 */
	class Exc : public std::runtime_error{
	public:
		Exc(const std::string& msg = std::string()) :
				std::runtime_error(msg.c_str())
		{}
	};

	class IllegalArgumentExc : public Exc{
	public:
		IllegalArgumentExc(const std::string& msg = std::string()) :
				Exc(msg)
		{}
	};

private:
	ColorDepth_e colorDepth_v;
	r4::vector2<unsigned> dim_v = 0;
	std::vector<uint8_t> buf_v; // image pixels data

public:
	/**
	 * @brief Default constructor.
	 * Creates uninitialized Image object.
	 */
	Image() :
			colorDepth_v(ColorDepth_e::UNKNOWN)
	{}

	Image(const Image& im) = default;

	/**
	 * @brief Get image dimensions.
	 * @return Image dimensions.
	 */
	const r4::vector2<unsigned>& dims()const noexcept{
		return this->dim_v;
	}

	/**
	 * @brief Get color depth.
	 * @return Bits per pixel.
	 */
	unsigned bitsPerPixel()const{
		return this->numChannels() * 8;
	}

	/**
	 * @brief Get color depth.
	 * @return Number of color channels.
	 */
	unsigned numChannels()const{
		return unsigned(this->colorDepth_v);
	}

	/**
	 * @brief Get color depth.
	 * @return Color depth type.
	 */
	ColorDepth_e colorDepth()const{
		return this->colorDepth_v;
	}

	/**
	 * @brief Get pixel data.
	 * @return Pixel data of the image.
	 */
	utki::span<uint8_t> buf(){
		return utki::make_span(this->buf_v);
	}

	/**
	 * @brief Get pixel data.
	 * @return Pixel data of the image.
	 */
	utki::span<const uint8_t> buf()const{
		return utki::make_span(this->buf_v);
	}

public:
	/**
	 * @brief Initialize this image object with given parameters.
	 * Pixel data remains uninitialized.
	 * @param dimensions - image dimensions.
	 * @param colorDepth - color depth.
	 */
	void init(r4::vector2<unsigned> dimensions, ColorDepth_e colorDepth){
		this->dim_v = dimensions;
		this->colorDepth_v = colorDepth;
		this->buf_v.resize(this->dims().x() * this->dims().y() * this->numChannels());
	}

	/**
	 * @brief Flip image vertically.
	 */
	void flipVertical();
	
private:
	static void PNG_CustomReadFunction(png_structp pngPtr, png_bytep data, png_size_t length){
		papki::file* fi = reinterpret_cast<papki::file*>(png_get_io_ptr(pngPtr));
		ASSERT_ALWAYS(fi)
	//	TRACE(<< "PNG_CustomReadFunction: fi = " << fi << " pngPtr = " << pngPtr << " data = " << std::hex << data << " length = " << length << std::endl)
		try{
			utki::span<png_byte> bufWrapper(data, size_t(length));
			fi->read(bufWrapper);
	//		TRACE(<< "PNG_CustomReadFunction: fi->Read() finished" << std::endl)
		}catch(...){
			// do not let any exception get out of this function
	//		TRACE(<< "PNG_CustomReadFunction: fi->Read() failed" << std::endl)
		}
	}
public:

	/**
	 * @brief Load image from PNG file.
	 * @param f - PNG file.
	 */
	void loadPNG(const papki::file& fi){
		ASSERT_ALWAYS(!fi.is_open())

		ASSERT_ALWAYS(this->buf_v.size() == 0)

		papki::file::guard file_guard(fi); // this will guarantee that the file will be closed upon exit
//		TRACE(<< "Image::LoadPNG(): file opened" << std::endl)

	#define PNGSIGSIZE 8 // The size of PNG signature (max 8 bytes)
		std::array<png_byte, PNGSIGSIZE> sig;
		memset(&*sig.begin(), 0, sig.size() * sizeof(sig[0]));

		{
			auto ret = // TODO: we should not rely on that it will always read the requested number of bytes (or should we?)

			fi.read(utki::make_span(sig));
			ASSERT_ALWAYS(ret == sig.size() * sizeof(sig[0]))
		}

		if(png_sig_cmp(&*sig.begin(), 0, sig.size() * sizeof(sig[0])) != 0){ // if it is not a PNG-file
			throw Image::Exc("Image::LoadPNG(): not a PNG file");
		}

		// Great!!! We have a PNG-file!
//		TRACE(<< "Image::LoadPNG(): file is a PNG" << std::endl)

		// Create internal PNG-structure to work with PNG file
		// (no warning and error callbacks)
		png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

		png_infop infoPtr = png_create_info_struct(pngPtr);// Create structure with file info

		png_set_sig_bytes(pngPtr, PNGSIGSIZE);// We've already read PNGSIGSIZE bytes

		// Set custom "ReadFromFile" function
		png_set_read_fn(pngPtr, const_cast<papki::file*>(&fi), PNG_CustomReadFunction);

		png_read_info(pngPtr, infoPtr); // Read in all information about file

		// Get information from infoPtr
		png_uint_32 width = 0;
		png_uint_32 height = 0;
		int bitDepth = 0;
		int colorType = 0;
		png_get_IHDR(pngPtr, infoPtr, &width, &height, &bitDepth, &colorType, 0, 0, 0);

		// Strip 16bit png  to 8bit
		if(bitDepth == 16){
			png_set_strip_16(pngPtr);
		}
		// Convert paletted PNG to RGB image
		if(colorType == PNG_COLOR_TYPE_PALETTE){
			png_set_palette_to_rgb(pngPtr);
		}
		// Convert grayscale PNG to 8bit greyscale PNG
		if(colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8){
			png_set_expand_gray_1_2_4_to_8(pngPtr);
		}
		// if(png_get_valid(pngPtr, infoPtr,PNG_INFO_tRNS)) png_set_tRNS_to_alpha(pngPtr);

		// set gamma information
		double gamma = 0.0f;

		// if there's gamma info in the file, set it to 2.2
		if(png_get_gAMA(pngPtr, infoPtr, &gamma)){
			png_set_gamma(pngPtr, 2.2, gamma);
		}else{
			png_set_gamma(pngPtr, 2.2, 0.45455); // set to 0.45455 otherwise (good guess for GIF images on PCs)
		}

		// update info after all transformations
		png_read_update_info(pngPtr, infoPtr);
		// get all dimensions and color info again
		png_get_IHDR(pngPtr, infoPtr, &width, &height, &bitDepth, &colorType, 0, 0, 0);
		ASSERT_ALWAYS(bitDepth == 8)

		// Set image type
		Image::ColorDepth_e imageType;
		switch(colorType){
			case PNG_COLOR_TYPE_GRAY:
				imageType = Image::ColorDepth_e::GREY;
				break;
			case PNG_COLOR_TYPE_GRAY_ALPHA:
				imageType = Image::ColorDepth_e::GREYA;
				break;
			case PNG_COLOR_TYPE_RGB:
				imageType = Image::ColorDepth_e::RGB;
				break;
			case PNG_COLOR_TYPE_RGB_ALPHA:
				imageType = Image::ColorDepth_e::RGBA;
				break;
			default:
				throw Image::Exc("Image::LoadPNG(): unknown colorType");
				break;
		}
		// Great! Number of channels and bits per pixel are initialized now!

		// set image dimensions and set buffer size
		this->init(r4::vector2<unsigned>(width, height), imageType);//Set buf array size (allocate memory)
		// Great! height and width are initialized and buffer memory allocated

//		TRACE(<< "Image::LoadPNG(): memory for image allocated" << std::endl)

		// Read image data
		png_size_t bytesPerRow = png_get_rowbytes(pngPtr, infoPtr);//get bytes per row

		// check that our expectations are correct
		if(bytesPerRow != this->dims().x() * this->numChannels()){
			throw Image::Exc("Image::LoadPNG(): number of bytes per row does not match expected value");
		}

		ASSERT_ALWAYS((bytesPerRow * height) == this->buf_v.size())

//		TRACE(<< "Image::LoadPNG(): going to read in the data" << std::endl)
		{
			ASSERT_ALWAYS(this->dims().y() && this->buf_v.size())
			std::vector<png_bytep> rows(this->dims().y());
			// initialize row pointers
//			TRACE(<< "Image::LoadPNG(): this->buf.Buf() = " << std::hex << this->buf.Buf() << std::endl)
			for(unsigned i = 0; i < this->dims().y(); ++i){
				rows[i] = &*this->buf_v.begin() + i * bytesPerRow;
//				TRACE(<< "Image::LoadPNG(): rows[i] = " << std::hex << rows[i] << std::endl)
			}
//			TRACE(<< "Image::LoadPNG(): row pointers are set" << std::endl)
			// Read in image data!
			png_read_image(pngPtr, &*rows.begin());
//			TRACE(<< "Image::LoadPNG(): image data read" << std::endl)
		}

		png_destroy_read_struct(&pngPtr,0,0); // free libpng memory
	}
};

namespace{
tst::set set("samples", [](tst::suite& suite){
    std::vector<std::string> files;

    {
		const std::regex suffix_regex("^.*\\.svg$");
		auto all_files = papki::fs_file(data_dir).list_dir();

		std::copy_if(
				all_files.begin(),
				all_files.end(),
				std::back_inserter(files),
				[&suffix_regex](auto& f){
					return std::regex_match(f, suffix_regex);
				}
			);
	}

    suite.add<std::string>(
        "sample",
        std::move(files),
        [](const auto& p){
            papki::fs_file in_file(data_dir + p);

            auto dom = svgdom::load(in_file);

            auto res = svgren::render(*dom);
            auto& img = res.pixels;

            papki::fs_file png_file(data_dir + render_backend_name + "/" + papki::not_suffix(in_file.not_dir()) + ".png");

            Image png;
            png.loadPNG(png_file);

            ASSERT_ALWAYS(png.buf().size() != 0)

            tst::check(png.colorDepth() == Image::ColorDepth_e::RGBA, SL) << "Error: PNG color depth is not RGBA: " << unsigned(png.colorDepth());
            
            tst::check(res.dims == png.dims(), SL) << "Error: svg dims " << res.dims << " did not match png dims " << png.dims();

            tst::check(img.size() == png.buf().size() / png.numChannels(), SL) << "Error: svg pixel buffer size (" << img.size() << ") did not match png pixel buffer size(" << png.buf().size() / png.numChannels() << ")";

            for(size_t i = 0; i != img.size(); ++i){
                std::array<uint8_t, 4> rgba;
                rgba[0] = img[i] & 0xff;
                rgba[1] = (img[i] >> 8) & 0xff;
                rgba[2] = (img[i] >> 16) & 0xff;
                rgba[3] = (img[i] >> 24) & 0xff;

                for(unsigned j = 0; j != rgba.size(); ++j){
                    auto c1 = rgba[j];
                    auto c2 = png.buf()[i * png.numChannels() + j];
                    if(c1 > c2){
                        std::swap(c1, c2);
                    }

                    if(unsigned(c2 - c1) > tolerance){
                        uint32_t pixel =
                            uint32_t(png.buf()[i * png.numChannels()]) |
                            (uint32_t(png.buf()[i * png.numChannels() + 1]) << 8) |
                            (uint32_t(png.buf()[i * png.numChannels() + 2]) << 16) |
                            (uint32_t(png.buf()[i * png.numChannels() + 3]) << 24)
                        ;

                        tst::check(false, SL) << "Error: PNG pixel #" << std::dec << i << " [" << (i % res.dims.x()) << ", " << (i / res.dims.y()) << "]" << " (0x" << std::hex << pixel << ") did not match SVG pixel (0x" << img[i] << ")" << ", png_file = " << png_file.path();
                    }
                }
            }
        }
    );
});
}
