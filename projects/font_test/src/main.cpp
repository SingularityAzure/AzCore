#include "AzCore/IO/Log.hpp"
#include "AzCore/memory.hpp"
#include "AzCore/font.hpp"

using namespace AzCore;

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

io::Log cout("main.log", true, true);

i32 main(i32 argumentCount, char **argumentValues) {

	if (argumentCount == 1) {
		cout.PrintLn("In order to use this program, you must pass the name of a font file as an argument.");
		return 0;
	}
	ClockTime start = Clock::now();
	font::Font font;
	font.filename = argumentValues[1];
	if (!font.Load()) {
		cout.PrintLn("Failed to load font: ", font::error);
		return 1;
	}
	WString string = ToWString("ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890[]/=?+_-\"<>',.\\|~!@#$%^&*(){}私ñÑēÈèéîêâô∵…ėȯȧıëäöïü学元気出区電話番号이작품희망");
	// for (u32 i = 0; i < 256; i++) {
	//	 for (char32 c : string) {
	//		 font.PrintGlyph(c);
	//	 }
	// }
	// for (u32 i = 0; i < 64; i++) {
	//	 for (char32 c = 32; c < 128; c++) {
	//		 font.PrintGlyph(c);
	//	 }
	// }
	// font.PrintGlyph((char32)'o');
	font::FontBuilder fontBuilder;
	fontBuilder.font = &font;
	fontBuilder.resolution = font::FontBuilder::HIGH;
	fontBuilder.AddString(string);
	// for (i32 i = 0; i < 1024; i+=64) {
	//	 // cout << "AddRange" << std::endl;
	//	 if (!fontBuilder.AddRange(i, i+63)) {
	//		 cout << "Failed fontBuilder.AddRange: " << font::error << std::endl;
	//		 return 1;
	//	 }
	//	 // cout << "Build" << std::endl;
		if (!fontBuilder.Build()) {
			cout.PrintLn("Failed fontBuilder.Build: ", font::error);
			return 1;
		}
	// }
	cout.PrintLn("Total time: ", FormatTime(Clock::now() - start));
	stbi_write_png("data/atlas.png", fontBuilder.dimensions.x, fontBuilder.dimensions.y, 1, fontBuilder.pixels.data, fontBuilder.dimensions.x);
	return 0;
}
