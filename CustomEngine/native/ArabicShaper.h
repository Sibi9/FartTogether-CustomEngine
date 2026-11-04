#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <algorithm>

namespace ArabicShaper {

struct GlyphForm {
    uint16_t isolated;
    uint16_t final_form;
    uint16_t initial;
    uint16_t medial;
    bool connects_next;
};

inline bool GetGlyph(uint32_t c, GlyphForm& gf) {
    switch (c) {
        case 0x0621: gf = { 0xFE80, 0xFE80, 0xFE80, 0xFE80, false }; return true; // hamza
        case 0x0622: gf = { 0xFE81, 0xFE82, 0xFE81, 0xFE82, false }; return true; // alef madda
        case 0x0623: gf = { 0xFE83, 0xFE84, 0xFE83, 0xFE84, false }; return true; // alef hamza above
        case 0x0624: gf = { 0xFE85, 0xFE86, 0xFE85, 0xFE86, false }; return true; // waw hamza
        case 0x0625: gf = { 0xFE87, 0xFE88, 0xFE87, 0xFE88, false }; return true; // alef hamza below
        case 0x0626: gf = { 0xFE89, 0xFE8A, 0xFE8B, 0xFE8C, true  }; return true; // yeh hamza
        case 0x0627: gf = { 0xFE8D, 0xFE8E, 0xFE8D, 0xFE8E, false }; return true; // alef
        case 0x0628: gf = { 0xFE8F, 0xFE90, 0xFE91, 0xFE92, true  }; return true; // beh
        case 0x0629: gf = { 0xFE93, 0xFE94, 0xFE93, 0xFE94, false }; return true; // teh marbuta
        case 0x062A: gf = { 0xFE95, 0xFE96, 0xFE97, 0xFE98, true  }; return true; // teh
        case 0x062B: gf = { 0xFE99, 0xFE9A, 0xFE9B, 0xFE9C, true  }; return true; // theh
        case 0x062C: gf = { 0xFE9D, 0xFE9E, 0xFE9F, 0xFEA0, true  }; return true; // jeem
        case 0x062D: gf = { 0xFEA1, 0xFEA2, 0xFEA3, 0xFEA4, true  }; return true; // hah
        case 0x062E: gf = { 0xFEA5, 0xFEA6, 0xFEA7, 0xFEA8, true  }; return true; // khah
        case 0x062F: gf = { 0xFEA9, 0xFEAA, 0xFEA9, 0xFEAA, false }; return true; // dal
        case 0x0630: gf = { 0xFEAB, 0xFEAC, 0xFEAB, 0xFEAC, false }; return true; // thal
        case 0x0631: gf = { 0xFEAD, 0xFEAE, 0xFEAD, 0xFEAE, false }; return true; // reh
        case 0x0632: gf = { 0xFEAF, 0xFEB0, 0xFEAF, 0xFEB0, false }; return true; // zain
        case 0x0633: gf = { 0xFEB1, 0xFEB2, 0xFEB3, 0xFEB4, true  }; return true; // seen
        case 0x0634: gf = { 0xFEB5, 0xFEB6, 0xFEB7, 0xFEB8, true  }; return true; // sheen
        case 0x0635: gf = { 0xFEB9, 0xFEBA, 0xFEBB, 0xFEBC, true  }; return true; // sad
        case 0x0636: gf = { 0xFEBD, 0xFEBE, 0xFEBF, 0xFEC0, true  }; return true; // dad
        case 0x0637: gf = { 0xFEC1, 0xFEC2, 0xFEC3, 0xFEC4, true  }; return true; // tah
        case 0x0638: gf = { 0xFEC5, 0xFEC6, 0xFEC7, 0xFEC8, true  }; return true; // zah
        case 0x0639: gf = { 0xFEC9, 0xFECA, 0xFECB, 0xFECC, true  }; return true; // ain
        case 0x063A: gf = { 0xFECD, 0xFECE, 0xFECF, 0xFED0, true  }; return true; // ghain
        case 0x0641: gf = { 0xFED1, 0xFED2, 0xFED3, 0xFED4, true  }; return true; // feh
        case 0x0642: gf = { 0xFED5, 0xFED6, 0xFED7, 0xFED8, true  }; return true; // qaf
        case 0x0643: gf = { 0xFED9, 0xFEDA, 0xFEDB, 0xFEDC, true  }; return true; // kaf
        case 0x0644: gf = { 0xFEDD, 0xFEDE, 0xFEDF, 0xFEE0, true  }; return true; // lam
        case 0x0645: gf = { 0xFEE1, 0xFEE2, 0xFEE3, 0xFEE4, true  }; return true; // meem
        case 0x0646: gf = { 0xFEE5, 0xFEE6, 0xFEE7, 0xFEE8, true  }; return true; // noon
        case 0x0647: gf = { 0xFEE9, 0xFEEA, 0xFEEB, 0xFEEC, true  }; return true; // heh
        case 0x0648: gf = { 0xFEED, 0xFEEE, 0xFEED, 0xFEEE, false }; return true; // waw
        case 0x0649: gf = { 0xFEEF, 0xFEF0, 0xFEEF, 0xFEF0, false }; return true; // alef maksura
        case 0x064A: gf = { 0xFEF1, 0xFEF2, 0xFEF3, 0xFEF4, true  }; return true; // yeh
        default: return false;
    }
}

inline bool IsArabicCodepoint(uint32_t c) {
    return (c >= 0x0600 && c <= 0x06FF);
}

inline bool IsPresentationForm(uint32_t c) {
    return (c >= 0xFE70 && c <= 0xFEFC);
}

inline std::vector<uint32_t> DecodeUtf8(const std::string& str) {
    std::vector<uint32_t> cps;
    size_t i = 0, n = str.size();
    while (i < n) {
        uint8_t c = (uint8_t)str[i];
        if (c < 0x80) {
            cps.push_back(c);
            i += 1;
        } else if ((c & 0xE0) == 0xC0 && i + 1 < n) {
            cps.push_back(((c & 0x1F) << 6) | ((uint8_t)str[i + 1] & 0x3F));
            i += 2;
        } else if ((c & 0xF0) == 0xE0 && i + 2 < n) {
            cps.push_back(((c & 0x0F) << 12) | (((uint8_t)str[i + 1] & 0x3F) << 6) | ((uint8_t)str[i + 2] & 0x3F));
            i += 3;
        } else if ((c & 0xF8) == 0xF0 && i + 3 < n) {
            cps.push_back(((c & 0x07) << 18) | (((uint8_t)str[i + 1] & 0x3F) << 12) | (((uint8_t)str[i + 2] & 0x3F) << 6) | ((uint8_t)str[i + 3] & 0x3F));
            i += 4;
        } else {
            cps.push_back(c);
            i += 1;
        }
    }
    return cps;
}

inline std::string EncodeUtf8(const std::vector<uint32_t>& cps) {
    std::string out;
    for (uint32_t cp : cps) {
        if (cp <= 0x7F) {
            out.push_back((char)cp);
        } else if (cp <= 0x7FF) {
            out.push_back((char)(0xC0 | ((cp >> 6) & 0x1F)));
            out.push_back((char)(0x80 | (cp & 0x3F)));
        } else if (cp <= 0xFFFF) {
            out.push_back((char)(0xE0 | ((cp >> 12) & 0x0F)));
            out.push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back((char)(0x80 | (cp & 0x3F)));
        } else {
            out.push_back((char)(0xF0 | ((cp >> 18) & 0x07)));
            out.push_back((char)(0x80 | ((cp >> 12) & 0x3F)));
            out.push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back((char)(0x80 | (cp & 0x3F)));
        }
    }
    return out;
}

inline std::vector<uint32_t> ShapeWord(const std::vector<uint32_t>& w) {
    struct Token {
        uint32_t cp;
        bool isLig;
        uint16_t isoLig, finLig;
    };
    std::vector<Token> tokens;
    size_t i = 0, n = w.size();
    while (i < n) {
        if (w[i] == 0x0644 && i + 1 < n) {
            uint32_t nxt = w[i + 1];
            if (nxt == 0x0622) { tokens.push_back({ 0, true, 0xFEF5, 0xFEF6 }); i += 2; continue; }
            if (nxt == 0x0623) { tokens.push_back({ 0, true, 0xFEF7, 0xFEF8 }); i += 2; continue; }
            if (nxt == 0x0625) { tokens.push_back({ 0, true, 0xFEF9, 0xFEFA }); i += 2; continue; }
            if (nxt == 0x0627) { tokens.push_back({ 0, true, 0xFEFB, 0xFEFC }); i += 2; continue; }
        }
        tokens.push_back({ w[i], false, 0, 0 });
        i++;
    }

    std::vector<uint32_t> shaped;
    for (size_t idx = 0; idx < tokens.size(); idx++) {
        const auto& tok = tokens[idx];
        if (tok.isLig) {
            bool prevConnects = false;
            if (idx > 0 && !tokens[idx - 1].isLig) {
                GlyphForm prevGf;
                if (GetGlyph(tokens[idx - 1].cp, prevGf)) prevConnects = prevGf.connects_next;
            }
            shaped.push_back(prevConnects ? tok.finLig : tok.isoLig);
        } else {
            GlyphForm gf;
            if (GetGlyph(tok.cp, gf)) {
                bool prevConnects = false;
                if (idx > 0 && !tokens[idx - 1].isLig) {
                    GlyphForm prevGf;
                    if (GetGlyph(tokens[idx - 1].cp, prevGf)) prevConnects = prevGf.connects_next;
                }
                bool nextConnects = false;
                if (idx + 1 < tokens.size()) {
                    if (tokens[idx + 1].isLig) nextConnects = true;
                    else {
                        GlyphForm nextGf;
                        if (GetGlyph(tokens[idx + 1].cp, nextGf)) nextConnects = true;
                    }
                }

                if (prevConnects && nextConnects && gf.connects_next) shaped.push_back(gf.medial);
                else if (prevConnects) shaped.push_back(gf.final_form);
                else if (nextConnects && gf.connects_next) shaped.push_back(gf.initial);
                else shaped.push_back(gf.isolated);
            } else {
                shaped.push_back(tok.cp);
            }
        }
    }
    return shaped;
}

inline std::string Reshape(const std::string& input) {
    if (input.empty()) return input;
    std::vector<uint32_t> cps = DecodeUtf8(input);
    bool hasArabic = false;
    bool hasPresentation = false;
    for (uint32_t cp : cps) {
        if (IsArabicCodepoint(cp)) hasArabic = true;
        if (IsPresentationForm(cp)) hasPresentation = true;
    }
    if (hasPresentation || !hasArabic) return input; // already shaped or pure latin

    // Split into whitespace-separated tokens
    std::vector<std::vector<uint32_t>> words;
    std::vector<uint32_t> curWord;
    for (uint32_t cp : cps) {
        if (cp == ' ' || cp == '\t') {
            if (!curWord.empty()) { words.push_back(curWord); curWord.clear(); }
        } else {
            curWord.push_back(cp);
        }
    }
    if (!curWord.empty()) words.push_back(curWord);

    std::vector<std::vector<uint32_t>> processedWords;
    for (const auto& w : words) {
        bool wHasAr = false;
        for (uint32_t cp : w) {
            if (IsArabicCodepoint(cp)) { wHasAr = true; break; }
        }

        if (wHasAr) {
            std::vector<uint32_t> resWord;
            std::vector<uint32_t> curAr;
            for (uint32_t ch : w) {
                if (IsArabicCodepoint(ch)) {
                    curAr.push_back(ch);
                } else {
                    if (!curAr.empty()) {
                        auto sh = ShapeWord(curAr);
                        resWord.insert(resWord.end(), sh.begin(), sh.end());
                        curAr.clear();
                    }
                    // Mirror brackets
                    if (ch == '(') resWord.push_back(')');
                    else if (ch == ')') resWord.push_back('(');
                    else if (ch == '[') resWord.push_back(']');
                    else if (ch == ']') resWord.push_back('[');
                    else if (ch == '<') resWord.push_back('>');
                    else if (ch == '>') resWord.push_back('<');
                    else resWord.push_back(ch);
                }
            }
            if (!curAr.empty()) {
                auto sh = ShapeWord(curAr);
                resWord.insert(resWord.end(), sh.begin(), sh.end());
            }
            // Reverse token order so Arabic glyphs and punctuation render properly in LTR engine
            std::reverse(resWord.begin(), resWord.end());
            processedWords.push_back(resWord);
        } else {
            processedWords.push_back(w);
        }
    }

    // BiDi sentence reverse
    std::reverse(processedWords.begin(), processedWords.end());

    std::vector<uint32_t> finalCps;
    for (size_t idx = 0; idx < processedWords.size(); idx++) {
        if (idx > 0) finalCps.push_back(' ');
        finalCps.insert(finalCps.end(), processedWords[idx].begin(), processedWords[idx].end());
    }

    return EncodeUtf8(finalCps);
}

} // namespace ArabicShaper

