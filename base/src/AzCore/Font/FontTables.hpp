/*
	File: FontTables.hpp
	Author: Philip Haynes
	Separating out low-level table definitions so font.hpp
	can have more high-level structures.
*/

#ifndef AZCORE_FONT_TABLES_HPP
#define AZCORE_FONT_TABLES_HPP

#include "../common.hpp"

#include "CFF.hpp"

namespace AzCore {
namespace font {

struct Glyph;
struct GlyphInfo;

// These are the basic types used for TrueType fonts
typedef i16 shortFrac_t;
union Fixed_t {
	struct {
		i16 iPart;
		u16 fPart;
	}; // For the actual fractions
	struct {
		u16 major;
		u16 minor;
	}; // For versions
};
Fixed_t bytesToFixed(char *buffer, const bool swapEndian);
inline f32 ToF32(const Fixed_t &in) {
	f32 out = in.iPart;
	out += (f32)in.fPart / 65535.0f * sign(out);
	return out;
}
typedef i16 FWord_t;
typedef u16 uFWord_t;
typedef i16 F2Dot14_t;
f32 ToF32(const F2Dot14_t& in);
typedef i64 longDateTime_t;

union Tag_t {
	u32 data;
	char name[4];
	Tag_t()=default;
	constexpr Tag_t(const u32 in);
	constexpr Tag_t(const char *in);
};

constexpr Tag_t operator "" _Tag(const char*, const size_t);
Tag_t operator "" _Tag(const u64);

/*  struct: glyfPoint
	Author: Philip Haynes
	An intermediate representation of points parsed from a glyf table glyph.        */
struct glyfPoint {
	vec2 coords;
	bool onCurve;
};

namespace tables {

// All the structs used to represent data extracted directly from the file cannot have implicit alignment
#pragma pack(1)

u32 Checksum(u32 *table, u32 length);
u32 ChecksumV2(u32 *table, u32 length); // Allows lengths to not be a multiple of 4

/*  struct: Record
	Author: Philip Haynes
	Contains information about one table.   */
struct Record {
	Tag_t tableTag;
	u32 checkSum;
	u32 offset; // Offset from beginning of the font file.
	u32 length;

	void Read(std::ifstream &file);
};
static_assert(sizeof(Record) == 16);

/*  struct: Offset
	Author: Philip Haynes
	Contains information about the tables in the font file.     */
struct Offset {
	Tag_t sfntVersion;
		// 0x00010000 for TrueType outlines
		// 0x74727565 "true" for TrueType
		// 0x74797031 "typ1" for old-style PostScript
		// 0x4F54544F "OTTO" for OpenType fonts containing CFF data
	u16 numTables;
	u16 searchRange; // (maximum power of 2 that's <= numTables) * 16
	u16 entrySelector; // log2(maximum power of 2 that's <= numTables)
	u16 rangeShift; // numTables * 16 - searchRange
	Array<Record> tables;
	void Read(std::ifstream &file);
};
static_assert(sizeof(Offset) == 12 + sizeof(Array<Record>));

/*  struct: TTCHeader
	Author: Philip Haynes
	Contains information about the fonts contained in this collection.  */
struct TTCHeader {
	Tag_t ttcTag;
	Fixed_t version;
	u32 numFonts;
	Array<u32> offsetTables; // Offsets to the individial offset tables
	Tag_t dsigTag;
	u32 dsigLength;
	u32 dsigOffset;
	bool Read(std::ifstream &file);
};
static_assert(sizeof(TTCHeader) == 24 + sizeof(Array<u32>));

struct cmap_encoding {
	u16 platformID;
	u16 platformSpecificID;
	u32 offset; // Bytes from beginning of cmap table
	void EndianSwap();
};
static_assert(sizeof(cmap_encoding) == 8);

struct cmap_index {
	u16 version; // Must be set to zero
	u16 numberSubtables; // How many encoding subtables there are
	void EndianSwap();
};
static_assert(sizeof(cmap_index) == 4);

/*
	Here we have the 3 cmap formats supported by Apple's implementations.
	This should be sufficient for reading just about any font file.
	structs:
		cmap_format_any
			- this one will parse the data from any of the following structs, whichever is appropriate
		cmap_format0
			- Windows encoding 0
			- Supports up to 256 glyphs with one 8-bit index per glyph
		cmap_format4
			- Unicode encoding 3
			- Windows encoding 1
			- Support for Unicode Basic Multilingual Plane (UCS-2)
		cmap_format12
			- Unicode encoding 4
			- Microsoft encoding 10
			- Support for full Unicode (UCS-4)
*/
struct cmap_format_any {
	u16 format; // Varies. Used to determine which kind of table we are.
	// Will polymorph if format is supported or return false to indicate it's not supported
	bool EndianSwap();
	u32 GetGlyphIndex(char32 glyph); // Reads a single index
};

struct cmap_format0 {
	u16 format;                     // Should be 0
	u16 length;                     // Length in bytes of the subtable, including header. Should be 262.
	u16 language;                   // Should be 0 for tables that aren't Macintosh platform cmaps.
	u8 glyphIndexArray[256];        // The mapping
	void EndianSwap();
	u32 GetGlyphIndex(char32 glyph); // Reads a single index
};
static_assert(sizeof(cmap_format0) == 262);

struct cmap_format4 {
	u16 format;                     // Should be 4
	u16 length;
	u16 language;
	// Can't say I understand why these are how they are
	u16 segCountX2;                 // 2 * segCount
	u16 searchRange;                // 2 * (2^floor(log2(segCount)))
	u16 entrySelector;              // log2(searchRange/2)
	u16 rangeShift;                 // (2 * segCount) - searchRange
	// The data past this point is variable-sized like so
	// u16 endCode[segCount];       // Ending character code for each segment, last = 0xFFFF
	// u16 reservedPad;             // Should be zero.
	// u16 startCode[segCount];     // Starting character code for each segment.
	// u16 idDelta[segCount];       // Delta for all character codes in segment.
	// u16 idRangeOffset[segCount]; // Offset in bytes to glpyhIndexArray, or 0
	// u16 glpyhIndexArray[...];    // Keep this in big endian
	void EndianSwap();
	// Easy accessors
	inline u16& endCode(const u16 i) {
		return *(&rangeShift + 1 + i);
	}
	inline u16& startCode(const u16 i) {
		return *(&rangeShift + segCountX2/2 + 2 + i);
	}
	inline u16& idDelta(const u16 i) {
		return *(&rangeShift + segCountX2 + 2 + i);
	}
	inline u16& idRangeOffset(const u16 i) {
		return *(&rangeShift + segCountX2*3/2 + 2 + i);
	}
	u32 GetGlyphIndex(char32 glyph); // Reads a single index
};
static_assert(sizeof(cmap_format4) == 14);

struct cmap_format12_group {
	u32 startCharCode;              // First character code in this group
	u32 endCharCode;                // Last character code in this group
	u32 startGlyphCode;             // Glyph index corresponding to the starting character code.
	void EndianSwap();
};
static_assert(sizeof(cmap_format12_group) == 12);

struct cmap_format12 {
	Fixed_t format; // Should be 12.0
	u32 length; // Byte length of this subtable including the header.
	u32 language;
	u32 nGroups; // Number of groupings which follow
	//  The data past this point is variable-sized like so
	// cmap_format12_group groups[nGroups];
	void EndianSwap();
	inline cmap_format12_group& groups(const u32 i) {
		return *((cmap_format12_group*)((char*)this + sizeof(cmap_format12) + sizeof(cmap_format12_group) * i));
	}
	u32 GetGlyphIndex(char32 glyph); // Reads a single index
};
static_assert(sizeof(cmap_format12) == 16);

/*  struct: head
	Author: Philip Haynes
	Contains all the information in a standard 'head' table.    */
struct head {
	Fixed_t version;
	Fixed_t fontRevision;
	u32 checkSumAdjustment;
	u32 magicNumber;
	u16 flags;
		// bit 0  - y value of 0 specifies baseline
		// bit 1  - x position of left most black bit is LSB
		// bit 2  - scaled point size and actual point size will differ
		// bit 3  - use integer scaling instead of fractional
		// bit 4  - (Microsoft - OTF) Instructions may alter advance width
		// bit 5  - Intended to be laid out vertically (i.e. x-coord 0 is the vertical baseline)
		// bit 6  - Must be zero
		// bit 7  - Requires layout for correct linguistic rendering
		// bit 8  - AAT font which has one or more metamorphosis effects by default
		// bit 9  - Contains any strong right-to-left glyphs
		// bit 10 - Contains Indic-style rearrangement effects.
		// bit 11 - (Adobe - OTF) Font data is "lossless". The DSIG table may be invalidated.
		// bit 12 - (Adobe - OTF) Font converted (produce compatible metrics)
		// bit 13 - (Adobe - OTF) Font optimized for ClearType™
		// bit 14 - Glyphs are simply generic symbols for code point ranges
		// bit 15 - Reserved, set to zero.
	u16 unitsPerEm; // range from 64 to 16384
	longDateTime_t created; // international date
	longDateTime_t modified; // international date
	FWord_t xMin; // for all glyph bounding boxes
	FWord_t yMin; // ^^^
	FWord_t xMax; // ^^^
	FWord_t yMax; // ^^^
	u16 macStyle;
		// bit 0 - bold
		// bit 1 - italic
		// bit 2 - underline
		// bit 3 - outline
		// bit 4 - shadow
		// bit 5 - condensed (narrow)
		// bit 6 - extended
	u16 lowestRecPPEM; // Smallest readable size in pixels
	i16 fontDirectionHint;
		// 0  - Mixed directional glyphs
		// 1  - Only strongly left to right
		// 2  - Like 1 but contains neutrals
		// -1 - Only strongly right to left
		// -2 - Like -1 but contains neutrals
	i16 indexToLocFormat; // 0 for short offsets, 1 for long
	i16 glyphDataFormat; // 0 for current format
	void EndianSwap();
};
static_assert(sizeof(head) == 54);

// Same exact thing, only used to indicate that the font doesn't
// have any glyph outlines and only contains embedded bitmaps.
typedef head bhed;

/*  struct: maxp
	Author: Philip Haynes
	Maximum Profile. Necessary for its numGlyphs data.      */
struct maxp {
	Fixed_t version;
	u16 numGlyphs;
	// If version is 1.0 then the following data is valid, otherwise ignore them
	u16 maxPoints;              // Max points in a non-composite glyph
	u16 maxContours;            // Max contours in a non-composite glyph
	u16 maxCompositePoints;     // Max points in a composite glyph
	u16 maxCompositeContours;   // Max contours in a composite glyph
	u16 maxZones;               // 1 if instructions don't use the Twilight Zone (Z0) or 2 if they do
	u16 maxTwilightPoints;      // Maximum points used in Z0
	u16 maxStorage;             // Number of storage area locations
	u16 maxFunctionDefs;        // Number of FDEFs, equal to the highest function number +1
	u16 maxInstructionDefs;     // Number of IDEFs.
	// Maximum stack depth across Font Program ('fpgm' table),
	// CVT Program ('prep' table) and all glyph instructions (in 'glyf' table)
	u16 maxStackElements;
	u16 maxSizeOfInstructions;  // Maximum byte count for glyph instructions
	u16 maxComponentElements;   // Maximum number of components referenced at top level for any composite glyph
	u16 maxComponentDepth;      // Maximum levels of recursion; 1 for simple components
	void EndianSwap();
};
static_assert(sizeof(maxp) == 32);

/*  struct: loca
	Author: Philip Haynes
	Index to Location. Used to carry the offsets into the 'glyf' table of character indices. */
struct loca {
	// All data in this table is variable-sized in the following forms
	// Short version, for when head.indexToLocFormat == 0
	// u16 offsets[maxp.numGlyphs + 1]; // This version is the actual offset / 2
	// Long version, for when head.indexToLocFormat == 1
	// u32 offsets[maxp.numGlyphs + 1];
	void EndianSwap(u16 numGlyphs, bool longOffsets);
	inline u16& offsets16(const u16 i) {
		return *((u16*)this + i);
	}
	inline u32& offsets32(const u16 i) {
		return *((u32*)this + i);
	}
};

/*  struct: glyf_header
	Author: Philip Haynes
	Beginning of every glyph definition.        */
struct glyf_header {
	i16 numberOfContours; // if >= 0, it's a simple glyph. if < 0 then it's a composite glyph.
	i16 xMin;
	i16 yMin;
	i16 xMax;
	i16 yMax;
	void EndianSwap();
};
static_assert(sizeof(glyf_header) == 10);

/*  struct: glyf
	Author: Philip Haynes
	The actual glyph outline data. Cannot be parsed correctly without the loca table.      */
struct glyf {
	/*  All data in this table is of variable size AND offset
		Since the actual offsets of glyphs depend on the loca table,
		I'm just going to describe the anatomy of a single glyph:
	glyf_header header;

		For a simple glyph:
	u16 endPtsOfContours[header.numberOfContours];
	u16 instructionLength;
	u8 instructions[instructionLength];
	u8 flags[...]; // Actual array size varies based on contents according to the following rules:
		Bit 0: Point is on-curve
		Bit 1: xCoord is 1 byte, else xCoord is 2 bytes
		Bit 2: yCoord is 1 byte, else yCoord is 2 bytes
		Bit 3: Repeat this set of flags n times. n is the next u8.
		Bit 4: Dual meaning
			- If bit 1 is set:
				- 1: xCoord is positive
				- 0: xCoord is negative
			- If bit 1 is not set:
				- 1: xCoord is the same as the previous and doesn't have an entry in the xCoords array
				- 0: xCoord in the array is a delta vector from the previous xCoord
		Bit 5: Same as bit 4, but refers to bit 2 and yCoord
	u8 or i16 xCoord[...];
	u8 or i16 yCoord[...];

		For a compound glyph:
	u16 flags;
		Bit 0:  arguments are 2-byte, else arguments are 1 byte
		Bit 1:  arguments are xy values, else arguments are points
		Bit 2:  Round xy values to grid
		Bit 3:  There is a simple scale for the component, else scale is 1.0
		Bit 4:  obsolete
		Bit 5:  At least one additional glyph follows this one
		Bit 6:  x direction has different scale than y direction
		Bit 7:  There is a 2-by-2 transformation matrix that will be used to scale the component
		Bit 8:  Instructions for the component character follow the last component
		Bit 9:  Use metrics from this component for the compound glyph
		Bit 10: The components of this compound glyph overlap
		Bit 11: The composite is designed to have the component offset scaled.
		Bit 12: The composite is designed not to have the component offset scaled.
			- This is the default if neither 11 nor 12 are set. Both should never be set simultaneously.
	u16 glyphIndex; // Glyph index of the component
		The arguments are unsigned if they're points, and signed if they're offsets.
	i16 or u16 or i8 or u8 argument1; // X-offset for component or point number
	i16 or u16 or i8 or u8 argument2; // Y-offset for component or point number
		In the case of Instructions as per flags bit 8, they come after the last component as follows:
	u16 numInstructions;
	u8 instructions[numInstructions];
	*/
	void EndianSwap(loca *loc, u16 numGlyphs, bool longOffsets);
	static void EndianSwapSimple(glyf_header *header);
	static void EndianSwapCompound(glyf_header *header);
};

/*  struct: hhea
	Author: Philip Haynes
	Horizontal header table, used in conjunction with the hmtx table.       */
struct hhea {
	Fixed_t version; // Should be 1.0
	FWord_t ascent; // Distance from baseline of highest ascender
	FWord_t descent; // Distance from baseline of lowest descender
	FWord_t lineGap; // Typographic line gap.
	uFWord_t advanceWidthMax;
	FWord_t minLeftSideBearing;
	FWord_t minRightSideBearing;
	FWord_t xMaxExtent; // max(lsb + (xMax-xMin))
	i16 caretSlopeRise; // Slope of the caret
	i16 caretSlopeRun; // 1/0 for vertical
	FWord_t caretOffset;
	i16 reserved[4];
	i16 metricDataFormat; // 0 for current format
	u16 numOfLongHorMetrics; // Number of advance widths in hmtx. The only thing we actually care about...
	void EndianSwap();
};

/*  struct: longHorMetric
	Author: Philip Haynes
	It's pretty straight forward.       */
struct longHorMetric {
	uFWord_t advanceWidth;
	FWord_t leftSideBearing;
};

/*  struct: hmtx
	Author: Philip Haynes
	Horizontal metrics table. Depends on hhea table.        */
struct hmtx {
	/* All data in this table is of variable size and offset as follows:
	longHorMetric hMetrics[numOfLongHorMetrics]; // From the hhea table
	FWord_t leftSideBearing[...]; // Size is the total number of glyphs minus numOfLongHorMetrics
	*/
	void EndianSwap(u16 numOfLongHorMetrics, u16 numGlyphs);
	longHorMetric Metric(u32 glyphIndex, u16 numOfLongHorMetrics) const;
};

#pragma pack() // We want alignment for parsed data for maximum efficiency

/*  struct: cffParsed
Author: Philip Haynes
Contains pointers to all the INDEX's and any data decompressed for easier use.  */
struct cffParsed {
	bool active = false;    // Whether we're using CFF data or not.
	bool CIDFont = false;   // Whether we're a CID-keyed Font

	head *header = nullptr;
	maxp *maxProfile = nullptr;
	hhea *horHeader = nullptr;
	hmtx *horMetrics = nullptr;

	cff *cffData;
	u8 *nameIndexData;
	u8 *dictIndexData;
	u8 *stringsIndexData;
	u8 *gsubrIndexData;
	u8 *charStringsIndexData;
	u8 *fdArrayData;
	cffs::index *nameIndex;
	cffs::index *dictIndex;
	cffs::index *stringsIndex;
	cffs::index *gsubrIndex;
	cffs::index *charStringsIndex;
	cffs::FDSelect_any *fdSelect;
	cffs::index *fdArray;
	Array<u32> nameIndexOffsets;
	Array<u32> dictIndexOffsets;
	Array<u32> stringsIndexOffsets;
	Array<u32> gsubrIndexOffsets;
	Array<u32> charStringsIndexOffsets;
	Array<u32> fdArrayOffsets;
	mutable Array<cffs::index*> privateIndicesAlreadySwapped; // Because these get read at Glyph retrieval
	cffs::dict dictIndexValues;
	Glyph GetGlyph(u32 glyphIndex) const;
	GlyphInfo GetGlyphInfo(u32 glyphIndex) const;
};

/*  struct: glyfParsed
	Author: Philip Haynes
	Contains all the information needed to use a glyf table.        */
struct glyfParsed {
	bool active = false;
	head *header = nullptr;
	maxp *maxProfile = nullptr;
	loca *indexToLoc = nullptr;
	glyf *glyphData;
	hhea *horHeader = nullptr;
	hmtx *horMetrics = nullptr;
	Array<u32> glyfOffsets;

	Glyph GetGlyph(u32 glyphIndex) const;
	GlyphInfo GetGlyphInfo(u32 glyphIndex) const;
	Glyph ParseSimple(glyf_header *gheader, Array<glyfPoint> *dstArray=nullptr) const;
	Glyph ParseCompound(glyf_header *gheader, Array<glyfPoint> *dstArray=nullptr) const;
};

} // namespace tables
} // namespace font
} // namespace AzCore

#endif // AZCORE_FONT_TABLES_HPP
