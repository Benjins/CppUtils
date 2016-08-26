#include "unicode.h"
#include "strings.h"
#include "macros.h"

U32String DecodeUTF8(const char* _start, int byteCount){
	// You know, you'd think static_cast would work.  But apparently not.
	const unsigned char* start = reinterpret_cast<const unsigned char*>(_start);
	U32String decoded = {};
	decoded.length = 0;
	decoded.start = (unsigned int*)malloc(byteCount*sizeof(unsigned int));

	unsigned char highBits[] = {0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE};

	for (int i = 0; i < byteCount; ){
		if ((start[i] & 0x80) == 0){
			// High bit not set, just ascii
			decoded.start[decoded.length] = start[i];
			decoded.length++;
			i++;
		}
		else{
			bool foundBit = false;
			for (int j = 0; j < BNS_ARRAY_COUNT(highBits) - 1; j++){
				if ((start[i] & highBits[j + 1]) == highBits[j]){
					int codepoint = (start[i] & (~highBits[j + 1]));
					for (int k = 0; k < j + 1; k++){
						if ((start[i + 1 + k] & 0xC0) != 0x80){
							ASSERT_WARN("Subsequent byte in UTF-8 string with improper high two bits: 0x%X", start[i + 1 + k]);
						}
						codepoint = (codepoint << 6) | (start[i + 1 + k] & 0x3F);
					}

					decoded.start[decoded.length] = codepoint;
					decoded.length++;
					i += (j + 2);
					foundBit = true;
					break;
				}
			}

			ASSERT_MSG(foundBit, "Improper formatting on byte: 0x%X", start[i]);
		}
	}

	return decoded;
}

int GetUTF8Size(const U32String string){
	unsigned int upperLimits[] = {0x80, 0x800, 0x10000, 0x200000, 0x4000000, 0x80000000};

	int byteCount = 0;
	for (int i = 0; i < string.length; i++) {
		bool foundLimit = false;
		for (int j = 0; j < BNS_ARRAY_COUNT(upperLimits); j++) {
			if (string.start[i] < upperLimits[j]) {
				byteCount += (j + 1);
				foundLimit = true;
				break;
			}
		}

		ASSERT_MSG(foundLimit, "Unicode codepoint %d out of range, occur at position %d.", string.start[i], i);
	}

	return byteCount;
}

int EncodeUTF8(const U32String inString, unsigned char* outString){
	unsigned int upperLimits[] = { 0x800, 0x10000, 0x200000, 0x4000000, 0x80000000 };
	unsigned char highBits[] = { 0xC0, 0xE0, 0xF0, 0xF8};

	int bytesWritten = 0;
	for (int i = 0; i < inString.length; i++) {
		if (inString.start[i] < 0x80) {
			outString[bytesWritten] = inString.start[i];
			bytesWritten++;
		}
		else {
			bool foundLimit = false;
			unsigned int codepoint = inString.start[i];
			for (int j = 0; j < BNS_ARRAY_COUNT(upperLimits); j++) {
				if (codepoint < upperLimits[j]) {
					for (int k = j + 1; k > 0; k--) {
						outString[bytesWritten + k] = (codepoint & 0x3F) | 0x80;
						codepoint >>= 6;
					}

					outString[bytesWritten] = codepoint | highBits[j];

					bytesWritten += (j + 2);
					foundLimit = true;
					break;
				}
			}

			ASSERT_MSG(foundLimit, "Unicode codepoint %d out of range, occur at position %d.", inString.start[i], i);
		}
	}

	return bytesWritten;
}

int U32FindChar(const U32String string, unsigned int codepoint){
	for (int i = 0; i < string.length; i++) {
		if (string.start[i] == codepoint) {
			return i;
		}
	}

	return -1;
}

int U32StrFind(const U32String haystack, const U32String needle){
	int hIndex = 0, nIndex = 0;
	while (hIndex < haystack.length && nIndex < needle.length) {
		if (haystack.start[hIndex] == needle.start[nIndex]) {
			nIndex++;
		}
		else {
			hIndex -= nIndex;
			nIndex = 0;
		}

		hIndex++;
	}

	if (nIndex == needle.length) {
		return hIndex - nIndex;
	}
	else {
		return -1;
	}
}

U32String U32StrDup(const U32String string){
	U32String newStr;
	newStr.length = string.length;
	newStr.start = (unsigned int*)malloc((newStr.length + 1) * sizeof(unsigned int));
	
	MemCpy(newStr.start, string.start, newStr.length * sizeof(unsigned int));
	newStr.start[newStr.length] = 0;
	
	return newStr;
}

// Strips out any characters outside of 0-127 ascii range
void ConvertToAscii(const U32String string, char* outAscii){
	
}

// Search is inclusive for both low and high (i.e. ascii search would be 0, 127)
int CountCharactersInRange(const U32String string, unsigned int codepointLow, unsigned int codepointHigh){
	return -1;
}

void DeallocateU32String(U32String string){
	free(string.start);
}

UnicodeBlockInfo _unicodeBlocks[] = {
	{ 0x0000, 0x007F, UBT_BasicLatin },
	{ 0x0080, 0x00FF, UBT_Latin_1Supplement },
	{ 0x0100, 0x017F, UBT_LatinExtended_A },
	{ 0x0180, 0x024F, UBT_LatinExtended_B },
	{ 0x0250, 0x02AF, UBT_IPAExtensions },
	{ 0x02B0, 0x02FF, UBT_SpacingModifierLetters },
	{ 0x0300, 0x036F, UBT_CombiningDiacriticalMarks },
	{ 0x0370, 0x03FF, UBT_GreekandCoptic },
	{ 0x0400, 0x04FF, UBT_Cyrillic },
	{ 0x0500, 0x052F, UBT_CyrillicSupplement },
	{ 0x0530, 0x058F, UBT_Armenian },
	{ 0x0590, 0x05FF, UBT_Hebrew },
	{ 0x0600, 0x06FF, UBT_Arabic },
	{ 0x0700, 0x074F, UBT_Syriac },
	{ 0x0750, 0x077F, UBT_ArabicSupplement },
	{ 0x0780, 0x07BF, UBT_Thaana },
	{ 0x07C0, 0x07FF, UBT_NKo },
	{ 0x0800, 0x083F, UBT_Samaritan },
	{ 0x0840, 0x085F, UBT_Mandaic },
	{ 0x08A0, 0x08FF, UBT_ArabicExtended_A },
	{ 0x0900, 0x097F, UBT_Devanagari },
	{ 0x0980, 0x09FF, UBT_Bengali },
	{ 0x0A00, 0x0A7F, UBT_Gurmukhi },
	{ 0x0A80, 0x0AFF, UBT_Gujarati },
	{ 0x0B00, 0x0B7F, UBT_Oriya },
	{ 0x0B80, 0x0BFF, UBT_Tamil },
	{ 0x0C00, 0x0C7F, UBT_Telugu },
	{ 0x0C80, 0x0CFF, UBT_Kannada },
	{ 0x0D00, 0x0D7F, UBT_Malayalam },
	{ 0x0D80, 0x0DFF, UBT_Sinhala },
	{ 0x0E00, 0x0E7F, UBT_Thai },
	{ 0x0E80, 0x0EFF, UBT_Lao },
	{ 0x0F00, 0x0FFF, UBT_Tibetan },
	{ 0x1000, 0x109F, UBT_Myanmar },
	{ 0x10A0, 0x10FF, UBT_Georgian },
	{ 0x1100, 0x11FF, UBT_HangulJamo },
	{ 0x1200, 0x137F, UBT_Ethiopic },
	{ 0x1380, 0x139F, UBT_EthiopicSupplement },
	{ 0x13A0, 0x13FF, UBT_Cherokee },
	{ 0x1400, 0x167F, UBT_UnifiedCanadianAboriginalSyllabics },
	{ 0x1680, 0x169F, UBT_Ogham },
	{ 0x16A0, 0x16FF, UBT_Runic },
	{ 0x1700, 0x171F, UBT_Tagalog },
	{ 0x1720, 0x173F, UBT_Hanunoo },
	{ 0x1740, 0x175F, UBT_Buhid },
	{ 0x1760, 0x177F, UBT_Tagbanwa },
	{ 0x1780, 0x17FF, UBT_Khmer },
	{ 0x1800, 0x18AF, UBT_Mongolian },
	{ 0x18B0, 0x18FF, UBT_UnifiedCanadianAboriginalSyllabicsExtended },
	{ 0x1900, 0x194F, UBT_Limbu },
	{ 0x1950, 0x197F, UBT_TaiLe },
	{ 0x1980, 0x19DF, UBT_NewTaiLue },
	{ 0x19E0, 0x19FF, UBT_KhmerSymbols },
	{ 0x1A00, 0x1A1F, UBT_Buginese },
	{ 0x1A20, 0x1AAF, UBT_TaiTham },
	{ 0x1AB0, 0x1AFF, UBT_CombiningDiacriticalMarksExtended },
	{ 0x1B00, 0x1B7F, UBT_Balinese },
	{ 0x1B80, 0x1BBF, UBT_Sundanese },
	{ 0x1BC0, 0x1BFF, UBT_Batak },
	{ 0x1C00, 0x1C4F, UBT_Lepcha },
	{ 0x1C50, 0x1C7F, UBT_OlChiki },
	{ 0x1C80, 0x1C8F, UBT_CyrillicExtended_C },
	{ 0x1CC0, 0x1CCF, UBT_SundaneseSupplement },
	{ 0x1CD0, 0x1CFF, UBT_VedicExtensions },
	{ 0x1D00, 0x1D7F, UBT_PhoneticExtensions },
	{ 0x1D80, 0x1DBF, UBT_PhoneticExtensionsSupplement },
	{ 0x1DC0, 0x1DFF, UBT_CombiningDiacriticalMarksSupplement },
	{ 0x1E00, 0x1EFF, UBT_LatinExtendedAdditional },
	{ 0x1F00, 0x1FFF, UBT_GreekExtended },
	{ 0x2000, 0x206F, UBT_GeneralPunctuation },
	{ 0x2070, 0x209F, UBT_SuperscriptsandSubscripts },
	{ 0x20A0, 0x20CF, UBT_CurrencySymbols },
	{ 0x20D0, 0x20FF, UBT_CombiningDiacriticalMarksforSymbols },
	{ 0x2100, 0x214F, UBT_LetterlikeSymbols },
	{ 0x2150, 0x218F, UBT_NumberForms },
	{ 0x2190, 0x21FF, UBT_Arrows },
	{ 0x2200, 0x22FF, UBT_MathematicalOperators },
	{ 0x2300, 0x23FF, UBT_MiscellaneousTechnical },
	{ 0x2400, 0x243F, UBT_ControlPictures },
	{ 0x2440, 0x245F, UBT_OpticalCharacterRecognition },
	{ 0x2460, 0x24FF, UBT_EnclosedAlphanumerics },
	{ 0x2500, 0x257F, UBT_BoxDrawing },
	{ 0x2580, 0x259F, UBT_BlockElements },
	{ 0x25A0, 0x25FF, UBT_GeometricShapes },
	{ 0x2600, 0x26FF, UBT_MiscellaneousSymbols },
	{ 0x2700, 0x27BF, UBT_Dingbats },
	{ 0x27C0, 0x27EF, UBT_MiscellaneousMathematicalSymbols_A },
	{ 0x27F0, 0x27FF, UBT_SupplementalArrows_A },
	{ 0x2800, 0x28FF, UBT_BraillePatterns },
	{ 0x2900, 0x297F, UBT_SupplementalArrows_B },
	{ 0x2980, 0x29FF, UBT_MiscellaneousMathematicalSymbols_B },
	{ 0x2A00, 0x2AFF, UBT_SupplementalMathematicalOperators },
	{ 0x2B00, 0x2BFF, UBT_MiscellaneousSymbolsandArrows },
	{ 0x2C00, 0x2C5F, UBT_Glagolitic },
	{ 0x2C60, 0x2C7F, UBT_LatinExtended_C },
	{ 0x2C80, 0x2CFF, UBT_Coptic },
	{ 0x2D00, 0x2D2F, UBT_GeorgianSupplement },
	{ 0x2D30, 0x2D7F, UBT_Tifinagh },
	{ 0x2D80, 0x2DDF, UBT_EthiopicExtended },
	{ 0x2DE0, 0x2DFF, UBT_CyrillicExtended_A },
	{ 0x2E00, 0x2E7F, UBT_SupplementalPunctuation },
	{ 0x2E80, 0x2EFF, UBT_CJKRadicalsSupplement },
	{ 0x2F00, 0x2FDF, UBT_KangxiRadicals },
	{ 0x2FF0, 0x2FFF, UBT_IdeographicDescriptionCharacters },
	{ 0x3000, 0x303F, UBT_CJKSymbolsandPunctuation },
	{ 0x3040, 0x309F, UBT_Hiragana },
	{ 0x30A0, 0x30FF, UBT_Katakana },
	{ 0x3100, 0x312F, UBT_Bopomofo },
	{ 0x3130, 0x318F, UBT_HangulCompatibilityJamo },
	{ 0x3190, 0x319F, UBT_Kanbun },
	{ 0x31A0, 0x31BF, UBT_BopomofoExtended },
	{ 0x31C0, 0x31EF, UBT_CJKStrokes },
	{ 0x31F0, 0x31FF, UBT_KatakanaPhoneticExtensions },
	{ 0x3200, 0x32FF, UBT_EnclosedCJKLettersandMonths },
	{ 0x3300, 0x33FF, UBT_CJKCompatibility },
	{ 0x3400, 0x4DBF, UBT_CJKUnifiedIdeographsExtensionA },
	{ 0x4DC0, 0x4DFF, UBT_YijingHexagramSymbols },
	{ 0x4E00, 0x9FFF, UBT_CJKUnifiedIdeographs },
	{ 0xA000, 0xA48F, UBT_YiSyllables },
	{ 0xA490, 0xA4CF, UBT_YiRadicals },
	{ 0xA4D0, 0xA4FF, UBT_Lisu },
	{ 0xA500, 0xA63F, UBT_Vai },
	{ 0xA640, 0xA69F, UBT_CyrillicExtended_B },
	{ 0xA6A0, 0xA6FF, UBT_Bamum },
	{ 0xA700, 0xA71F, UBT_ModifierToneLetters },
	{ 0xA720, 0xA7FF, UBT_LatinExtended_D },
	{ 0xA800, 0xA82F, UBT_SylotiNagri },
	{ 0xA830, 0xA83F, UBT_CommonIndicNumberForms },
	{ 0xA840, 0xA87F, UBT_Phags_pa },
	{ 0xA880, 0xA8DF, UBT_Saurashtra },
	{ 0xA8E0, 0xA8FF, UBT_DevanagariExtended },
	{ 0xA900, 0xA92F, UBT_KayahLi },
	{ 0xA930, 0xA95F, UBT_Rejang },
	{ 0xA960, 0xA97F, UBT_HangulJamoExtended_A },
	{ 0xA980, 0xA9DF, UBT_Javanese },
	{ 0xA9E0, 0xA9FF, UBT_MyanmarExtended_B },
	{ 0xAA00, 0xAA5F, UBT_Cham },
	{ 0xAA60, 0xAA7F, UBT_MyanmarExtended_A },
	{ 0xAA80, 0xAADF, UBT_TaiViet },
	{ 0xAAE0, 0xAAFF, UBT_MeeteiMayekExtensions },
	{ 0xAB00, 0xAB2F, UBT_EthiopicExtended_A },
	{ 0xAB30, 0xAB6F, UBT_LatinExtended_E },
	{ 0xAB70, 0xABBF, UBT_CherokeeSupplement },
	{ 0xABC0, 0xABFF, UBT_MeeteiMayek },
	{ 0xAC00, 0xD7AF, UBT_HangulSyllables },
	{ 0xD7B0, 0xD7FF, UBT_HangulJamoExtended_B },
	{ 0xD800, 0xDB7F, UBT_HighSurrogates },
	{ 0xDB80, 0xDBFF, UBT_HighPrivateUseSurrogates },
	{ 0xDC00, 0xDFFF, UBT_LowSurrogates },
	{ 0xE000, 0xF8FF, UBT_PrivateUseArea },
	{ 0xF900, 0xFAFF, UBT_CJKCompatibilityIdeographs },
	{ 0xFB00, 0xFB4F, UBT_AlphabeticPresentationForms },
	{ 0xFB50, 0xFDFF, UBT_ArabicPresentationForms_A },
	{ 0xFE00, 0xFE0F, UBT_VariationSelectors },
	{ 0xFE10, 0xFE1F, UBT_VerticalForms },
	{ 0xFE20, 0xFE2F, UBT_CombiningHalfMarks },
	{ 0xFE30, 0xFE4F, UBT_CJKCompatibilityForms },
	{ 0xFE50, 0xFE6F, UBT_SmallFormVariants },
	{ 0xFE70, 0xFEFF, UBT_ArabicPresentationForms_B },
	{ 0xFF00, 0xFFEF, UBT_HalfwidthandFullwidthForms },
	{ 0xFFF0, 0xFFFF, UBT_Specials },
	{ 0x10000, 0x1007F, UBT_LinearBSyllabary },
	{ 0x10080, 0x100FF, UBT_LinearBIdeograms },
	{ 0x10100, 0x1013F, UBT_AegeanNumbers },
	{ 0x10140, 0x1018F, UBT_AncientGreekNumbers },
	{ 0x10190, 0x101CF, UBT_AncientSymbols },
	{ 0x101D0, 0x101FF, UBT_PhaistosDisc },
	{ 0x10280, 0x1029F, UBT_Lycian },
	{ 0x102A0, 0x102DF, UBT_Carian },
	{ 0x102E0, 0x102FF, UBT_CopticEpactNumbers },
	{ 0x10300, 0x1032F, UBT_OldItalic },
	{ 0x10330, 0x1034F, UBT_Gothic },
	{ 0x10350, 0x1037F, UBT_OldPermic },
	{ 0x10380, 0x1039F, UBT_Ugaritic },
	{ 0x103A0, 0x103DF, UBT_OldPersian },
	{ 0x10400, 0x1044F, UBT_Deseret },
	{ 0x10450, 0x1047F, UBT_Shavian },
	{ 0x10480, 0x104AF, UBT_Osmanya },
	{ 0x104B0, 0x104FF, UBT_Osage },
	{ 0x10500, 0x1052F, UBT_Elbasan },
	{ 0x10530, 0x1056F, UBT_CaucasianAlbanian },
	{ 0x10600, 0x1077F, UBT_LinearA },
	{ 0x10800, 0x1083F, UBT_CypriotSyllabary },
	{ 0x10840, 0x1085F, UBT_ImperialAramaic },
	{ 0x10860, 0x1087F, UBT_Palmyrene },
	{ 0x10880, 0x108AF, UBT_Nabataean },
	{ 0x108E0, 0x108FF, UBT_Hatran },
	{ 0x10900, 0x1091F, UBT_Phoenician },
	{ 0x10920, 0x1093F, UBT_Lydian },
	{ 0x10980, 0x1099F, UBT_MeroiticHieroglyphs },
	{ 0x109A0, 0x109FF, UBT_MeroiticCursive },
	{ 0x10A00, 0x10A5F, UBT_Kharoshthi },
	{ 0x10A60, 0x10A7F, UBT_OldSouthArabian },
	{ 0x10A80, 0x10A9F, UBT_OldNorthArabian },
	{ 0x10AC0, 0x10AFF, UBT_Manichaean },
	{ 0x10B00, 0x10B3F, UBT_Avestan },
	{ 0x10B40, 0x10B5F, UBT_InscriptionalParthian },
	{ 0x10B60, 0x10B7F, UBT_InscriptionalPahlavi },
	{ 0x10B80, 0x10BAF, UBT_PsalterPahlavi },
	{ 0x10C00, 0x10C4F, UBT_OldTurkic },
	{ 0x10C80, 0x10CFF, UBT_OldHungarian },
	{ 0x10E60, 0x10E7F, UBT_RumiNumeralSymbols },
	{ 0x11000, 0x1107F, UBT_Brahmi },
	{ 0x11080, 0x110CF, UBT_Kaithi },
	{ 0x110D0, 0x110FF, UBT_SoraSompeng },
	{ 0x11100, 0x1114F, UBT_Chakma },
	{ 0x11150, 0x1117F, UBT_Mahajani },
	{ 0x11180, 0x111DF, UBT_Sharada },
	{ 0x111E0, 0x111FF, UBT_SinhalaArchaicNumbers },
	{ 0x11200, 0x1124F, UBT_Khojki },
	{ 0x11280, 0x112AF, UBT_Multani },
	{ 0x112B0, 0x112FF, UBT_Khudawadi },
	{ 0x11300, 0x1137F, UBT_Grantha },
	{ 0x11400, 0x1147F, UBT_Newa },
	{ 0x11480, 0x114DF, UBT_Tirhuta },
	{ 0x11580, 0x115FF, UBT_Siddham },
	{ 0x11600, 0x1165F, UBT_Modi },
	{ 0x11660, 0x1167F, UBT_MongolianSupplement },
	{ 0x11680, 0x116CF, UBT_Takri },
	{ 0x11700, 0x1173F, UBT_Ahom },
	{ 0x118A0, 0x118FF, UBT_WarangCiti },
	{ 0x11AC0, 0x11AFF, UBT_PauCinHau },
	{ 0x11C00, 0x11C6F, UBT_Bhaiksuki },
	{ 0x11C70, 0x11CBF, UBT_Marchen },
	{ 0x12000, 0x123FF, UBT_Cuneiform },
	{ 0x12400, 0x1247F, UBT_CuneiformNumbersandPunctuation },
	{ 0x12480, 0x1254F, UBT_EarlyDynasticCuneiform },
	{ 0x13000, 0x1342F, UBT_EgyptianHieroglyphs },
	{ 0x14400, 0x1467F, UBT_AnatolianHieroglyphs },
	{ 0x16800, 0x16A3F, UBT_BamumSupplement },
	{ 0x16A40, 0x16A6F, UBT_Mro },
	{ 0x16AD0, 0x16AFF, UBT_BassaVah },
	{ 0x16B00, 0x16B8F, UBT_PahawhHmong },
	{ 0x16F00, 0x16F9F, UBT_Miao },
	{ 0x16FE0, 0x16FFF, UBT_IdeographicSymbolsandPunctuation },
	{ 0x17000, 0x187FF, UBT_Tangut },
	{ 0x18800, 0x18AFF, UBT_TangutComponents },
	{ 0x1B000, 0x1B0FF, UBT_KanaSupplement },
	{ 0x1BC00, 0x1BC9F, UBT_Duployan },
	{ 0x1BCA0, 0x1BCAF, UBT_ShorthandFormatControls },
	{ 0x1D000, 0x1D0FF, UBT_ByzantineMusicalSymbols },
	{ 0x1D100, 0x1D1FF, UBT_MusicalSymbols },
	{ 0x1D200, 0x1D24F, UBT_AncientGreekMusicalNotation },
	{ 0x1D300, 0x1D35F, UBT_TaiXuanJingSymbols },
	{ 0x1D360, 0x1D37F, UBT_CountingRodNumerals },
	{ 0x1D400, 0x1D7FF, UBT_MathematicalAlphanumericSymbols },
	{ 0x1D800, 0x1DAAF, UBT_SuttonSignWriting },
	{ 0x1E000, 0x1E02F, UBT_GlagoliticSupplement },
	{ 0x1E800, 0x1E8DF, UBT_MendeKikakui },
	{ 0x1E900, 0x1E95F, UBT_Adlam },
	{ 0x1EE00, 0x1EEFF, UBT_ArabicMathematicalAlphabeticSymbols },
	{ 0x1F000, 0x1F02F, UBT_MahjongTiles },
	{ 0x1F030, 0x1F09F, UBT_DominoTiles },
	{ 0x1F0A0, 0x1F0FF, UBT_PlayingCards },
	{ 0x1F100, 0x1F1FF, UBT_EnclosedAlphanumericSupplement },
	{ 0x1F200, 0x1F2FF, UBT_EnclosedIdeographicSupplement },
	{ 0x1F300, 0x1F5FF, UBT_MiscellaneousSymbolsandPictographs },
	{ 0x1F600, 0x1F64F, UBT_Emoticons },
	{ 0x1F650, 0x1F67F, UBT_OrnamentalDingbats },
	{ 0x1F680, 0x1F6FF, UBT_TransportandMapSymbols },
	{ 0x1F700, 0x1F77F, UBT_AlchemicalSymbols },
	{ 0x1F780, 0x1F7FF, UBT_GeometricShapesExtended },
	{ 0x1F800, 0x1F8FF, UBT_SupplementalArrows_C },
	{ 0x1F900, 0x1F9FF, UBT_SupplementalSymbolsandPictographs },
	{ 0x20000, 0x2A6DF, UBT_CJKUnifiedIdeographsExtensionB },
	{ 0x2A700, 0x2B73F, UBT_CJKUnifiedIdeographsExtensionC },
	{ 0x2B740, 0x2B81F, UBT_CJKUnifiedIdeographsExtensionD },
	{ 0x2B820, 0x2CEAF, UBT_CJKUnifiedIdeographsExtensionE },
	{ 0x2F800, 0x2FA1F, UBT_CJKCompatibilityIdeographsSupplement },
	{ 0xE0000, 0xE007F, UBT_Tags },
	{ 0xE0100, 0xE01EF, UBT_VariationSelectorsSupplement },
	{ 0xF0000, 0xFFFFF, UBT_SupplementaryPrivateUseArea_A },
	{ 0x100000, 0x10FFFF, UBT_SupplementaryPrivateUseArea_B }
};

UnicodeBlockType GetBlockTypeOfCodePoint(unsigned int c) {
	UnicodeBlockInfo* blockTable = GetUnicodeBlocks();
	for (int i = 0; i < UBT_Count; i++) {
		if (blockTable[i].low < c && c < blockTable[i].high) {
			return blockTable[i].blockType;
		}
	}

	return UBT_Count;
}

UnicodeBlockInfo* GetUnicodeBlocks() {
	return _unicodeBlocks;
}

#if defined(UNICODE_TEST_MAIN)

#include "filesys.h"

int main(int argc, char** argv){
	
	
	int fileLength = 0;
	unsigned char* fileBytes = ReadBinaryFile("unicode_test.txt", &fileLength);
	
	U32String utf32 = DecodeUTF8((char*)fileBytes, fileLength);

	int reEncodeSize = GetUTF8Size(utf32);

	unsigned char* reEncoded = (unsigned char*)malloc(reEncodeSize);
	int bytesWritten = EncodeUTF8(utf32, reEncoded);

	ASSERT(reEncodeSize == fileLength);

	int diff = memcmp(fileBytes, reEncoded, reEncodeSize);
	ASSERT(diff == 0);
	
	U32String utf32dup = U32StrDup(utf32);
	ASSERT(utf32dup.length == utf32.length);
	int dupDiff = memcmp(utf32dup.start, utf32.start, utf32.length);
	ASSERT(dupDiff == 0);

	// 0x6211 is the codepoint for the chinese character 'wo3', meaning me/I
	int locOfWo = U32FindChar(utf32, 0x6211);
	ASSERT(locOfWo == 3);

	FILE* unicodeOut = fopen("unicode__out.txt", "wb");
	fwrite(reEncoded, 1, bytesWritten, unicodeOut);
	fclose(unicodeOut);

	DeallocateU32String(utf32dup);
	DeallocateU32String(utf32);
	
	free(fileBytes);
	free(reEncoded);
	
	return 0;
}


#endif
