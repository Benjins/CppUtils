#ifndef UNICODE_H
#define UNICODE_H

#pragma once

struct U32String{
	unsigned int* start;
	int length;
};

U32String DecodeUTF8(const char* start, int byteCount);
int GetUTF8Size(const U32String string);
int EncodeUTF8(const U32String inString, char* outString);

int U32FindChar(const U32String string, unsigned int codepoint);
int U32StrFind(const U32String haystack, const U32String needle);
U32String U32StrDup(const U32String string);

// Strips out any characters outside of 0-127 ascii range
void ConvertToAscii(const U32String string, char* outAscii);

// Search is inclusive for both low and high (i.e. ascii search would be 0, 127)
int CountCharactersInRange(const U32String string, unsigned int codepointLow, unsigned int codepointHigh);

void DeallocateU32String(U32String string);

//--------------------------------
// Unicode block information:
// From http://ftp.unicode.org/Public/9.0.0/ucd/Blocks.txt

enum UnicodeBlockType {
	UBT_BasicLatin,
	UBT_Latin_1Supplement,
	UBT_LatinExtended_A,
	UBT_LatinExtended_B,
	UBT_IPAExtensions,
	UBT_SpacingModifierLetters,
	UBT_CombiningDiacriticalMarks,
	UBT_GreekandCoptic,
	UBT_Cyrillic,
	UBT_CyrillicSupplement,
	UBT_Armenian,
	UBT_Hebrew,
	UBT_Arabic,
	UBT_Syriac,
	UBT_ArabicSupplement,
	UBT_Thaana,
	UBT_NKo,
	UBT_Samaritan,
	UBT_Mandaic,
	UBT_ArabicExtended_A,
	UBT_Devanagari,
	UBT_Bengali,
	UBT_Gurmukhi,
	UBT_Gujarati,
	UBT_Oriya,
	UBT_Tamil,
	UBT_Telugu,
	UBT_Kannada,
	UBT_Malayalam,
	UBT_Sinhala,
	UBT_Thai,
	UBT_Lao,
	UBT_Tibetan,
	UBT_Myanmar,
	UBT_Georgian,
	UBT_HangulJamo,
	UBT_Ethiopic,
	UBT_EthiopicSupplement,
	UBT_Cherokee,
	UBT_UnifiedCanadianAboriginalSyllabics,
	UBT_Ogham,
	UBT_Runic,
	UBT_Tagalog,
	UBT_Hanunoo,
	UBT_Buhid,
	UBT_Tagbanwa,
	UBT_Khmer,
	UBT_Mongolian,
	UBT_UnifiedCanadianAboriginalSyllabicsExtended,
	UBT_Limbu,
	UBT_TaiLe,
	UBT_NewTaiLue,
	UBT_KhmerSymbols,
	UBT_Buginese,
	UBT_TaiTham,
	UBT_CombiningDiacriticalMarksExtended,
	UBT_Balinese,
	UBT_Sundanese,
	UBT_Batak,
	UBT_Lepcha,
	UBT_OlChiki,
	UBT_CyrillicExtended_C,
	UBT_SundaneseSupplement,
	UBT_VedicExtensions,
	UBT_PhoneticExtensions,
	UBT_PhoneticExtensionsSupplement,
	UBT_CombiningDiacriticalMarksSupplement,
	UBT_LatinExtendedAdditional,
	UBT_GreekExtended,
	UBT_GeneralPunctuation,
	UBT_SuperscriptsandSubscripts,
	UBT_CurrencySymbols,
	UBT_CombiningDiacriticalMarksforSymbols,
	UBT_LetterlikeSymbols,
	UBT_NumberForms,
	UBT_Arrows,
	UBT_MathematicalOperators,
	UBT_MiscellaneousTechnical,
	UBT_ControlPictures,
	UBT_OpticalCharacterRecognition,
	UBT_EnclosedAlphanumerics,
	UBT_BoxDrawing,
	UBT_BlockElements,
	UBT_GeometricShapes,
	UBT_MiscellaneousSymbols,
	UBT_Dingbats,
	UBT_MiscellaneousMathematicalSymbols_A,
	UBT_SupplementalArrows_A,
	UBT_BraillePatterns,
	UBT_SupplementalArrows_B,
	UBT_MiscellaneousMathematicalSymbols_B,
	UBT_SupplementalMathematicalOperators,
	UBT_MiscellaneousSymbolsandArrows,
	UBT_Glagolitic,
	UBT_LatinExtended_C,
	UBT_Coptic,
	UBT_GeorgianSupplement,
	UBT_Tifinagh,
	UBT_EthiopicExtended,
	UBT_CyrillicExtended_A,
	UBT_SupplementalPunctuation,
	UBT_CJKRadicalsSupplement,
	UBT_KangxiRadicals,
	UBT_IdeographicDescriptionCharacters,
	UBT_CJKSymbolsandPunctuation,
	UBT_Hiragana,
	UBT_Katakana,
	UBT_Bopomofo,
	UBT_HangulCompatibilityJamo,
	UBT_Kanbun,
	UBT_BopomofoExtended,
	UBT_CJKStrokes,
	UBT_KatakanaPhoneticExtensions,
	UBT_EnclosedCJKLettersandMonths,
	UBT_CJKCompatibility,
	UBT_CJKUnifiedIdeographsExtensionA,
	UBT_YijingHexagramSymbols,
	UBT_CJKUnifiedIdeographs,
	UBT_YiSyllables,
	UBT_YiRadicals,
	UBT_Lisu,
	UBT_Vai,
	UBT_CyrillicExtended_B,
	UBT_Bamum,
	UBT_ModifierToneLetters,
	UBT_LatinExtended_D,
	UBT_SylotiNagri,
	UBT_CommonIndicNumberForms,
	UBT_Phags_pa,
	UBT_Saurashtra,
	UBT_DevanagariExtended,
	UBT_KayahLi,
	UBT_Rejang,
	UBT_HangulJamoExtended_A,
	UBT_Javanese,
	UBT_MyanmarExtended_B,
	UBT_Cham,
	UBT_MyanmarExtended_A,
	UBT_TaiViet,
	UBT_MeeteiMayekExtensions,
	UBT_EthiopicExtended_A,
	UBT_LatinExtended_E,
	UBT_CherokeeSupplement,
	UBT_MeeteiMayek,
	UBT_HangulSyllables,
	UBT_HangulJamoExtended_B,
	UBT_HighSurrogates,
	UBT_HighPrivateUseSurrogates,
	UBT_LowSurrogates,
	UBT_PrivateUseArea,
	UBT_CJKCompatibilityIdeographs,
	UBT_AlphabeticPresentationForms,
	UBT_ArabicPresentationForms_A,
	UBT_VariationSelectors,
	UBT_VerticalForms,
	UBT_CombiningHalfMarks,
	UBT_CJKCompatibilityForms,
	UBT_SmallFormVariants,
	UBT_ArabicPresentationForms_B,
	UBT_HalfwidthandFullwidthForms,
	UBT_Specials,
	UBT_LinearBSyllabary,
	UBT_LinearBIdeograms,
	UBT_AegeanNumbers,
	UBT_AncientGreekNumbers,
	UBT_AncientSymbols,
	UBT_PhaistosDisc,
	UBT_Lycian,
	UBT_Carian,
	UBT_CopticEpactNumbers,
	UBT_OldItalic,
	UBT_Gothic,
	UBT_OldPermic,
	UBT_Ugaritic,
	UBT_OldPersian,
	UBT_Deseret,
	UBT_Shavian,
	UBT_Osmanya,
	UBT_Osage,
	UBT_Elbasan,
	UBT_CaucasianAlbanian,
	UBT_LinearA,
	UBT_CypriotSyllabary,
	UBT_ImperialAramaic,
	UBT_Palmyrene,
	UBT_Nabataean,
	UBT_Hatran,
	UBT_Phoenician,
	UBT_Lydian,
	UBT_MeroiticHieroglyphs,
	UBT_MeroiticCursive,
	UBT_Kharoshthi,
	UBT_OldSouthArabian,
	UBT_OldNorthArabian,
	UBT_Manichaean,
	UBT_Avestan,
	UBT_InscriptionalParthian,
	UBT_InscriptionalPahlavi,
	UBT_PsalterPahlavi,
	UBT_OldTurkic,
	UBT_OldHungarian,
	UBT_RumiNumeralSymbols,
	UBT_Brahmi,
	UBT_Kaithi,
	UBT_SoraSompeng,
	UBT_Chakma,
	UBT_Mahajani,
	UBT_Sharada,
	UBT_SinhalaArchaicNumbers,
	UBT_Khojki,
	UBT_Multani,
	UBT_Khudawadi,
	UBT_Grantha,
	UBT_Newa,
	UBT_Tirhuta,
	UBT_Siddham,
	UBT_Modi,
	UBT_MongolianSupplement,
	UBT_Takri,
	UBT_Ahom,
	UBT_WarangCiti,
	UBT_PauCinHau,
	UBT_Bhaiksuki,
	UBT_Marchen,
	UBT_Cuneiform,
	UBT_CuneiformNumbersandPunctuation,
	UBT_EarlyDynasticCuneiform,
	UBT_EgyptianHieroglyphs,
	UBT_AnatolianHieroglyphs,
	UBT_BamumSupplement,
	UBT_Mro,
	UBT_BassaVah,
	UBT_PahawhHmong,
	UBT_Miao,
	UBT_IdeographicSymbolsandPunctuation,
	UBT_Tangut,
	UBT_TangutComponents,
	UBT_KanaSupplement,
	UBT_Duployan,
	UBT_ShorthandFormatControls,
	UBT_ByzantineMusicalSymbols,
	UBT_MusicalSymbols,
	UBT_AncientGreekMusicalNotation,
	UBT_TaiXuanJingSymbols,
	UBT_CountingRodNumerals,
	UBT_MathematicalAlphanumericSymbols,
	UBT_SuttonSignWriting,
	UBT_GlagoliticSupplement,
	UBT_MendeKikakui,
	UBT_Adlam,
	UBT_ArabicMathematicalAlphabeticSymbols,
	UBT_MahjongTiles,
	UBT_DominoTiles,
	UBT_PlayingCards,
	UBT_EnclosedAlphanumericSupplement,
	UBT_EnclosedIdeographicSupplement,
	UBT_MiscellaneousSymbolsandPictographs,
	UBT_Emoticons,
	UBT_OrnamentalDingbats,
	UBT_TransportandMapSymbols,
	UBT_AlchemicalSymbols,
	UBT_GeometricShapesExtended,
	UBT_SupplementalArrows_C,
	UBT_SupplementalSymbolsandPictographs,
	UBT_CJKUnifiedIdeographsExtensionB,
	UBT_CJKUnifiedIdeographsExtensionC,
	UBT_CJKUnifiedIdeographsExtensionD,
	UBT_CJKUnifiedIdeographsExtensionE,
	UBT_CJKCompatibilityIdeographsSupplement,
	UBT_Tags,
	UBT_VariationSelectorsSupplement,
	UBT_SupplementaryPrivateUseArea_A,
	UBT_SupplementaryPrivateUseArea_B,
	UBT_Count
};

struct UnicodeBlockInfo {
	unsigned int low;
	unsigned int high;
	UnicodeBlockType blockType;
};

UnicodeBlockInfo* GetUnicodeBlocks();

// TODO: Binary search opt?
UnicodeBlockType GetBlockTypeOfCodePoint(unsigned int c);

#endif
