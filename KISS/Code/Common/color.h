#pragma once

#include "ProjectDefines.h"

class Color
{
  union
  {
    byte    m_components[4];
    uint32  m_componentsAsInt;
  };

public:
  Color();
  explicit Color(byte r, byte g, byte b, byte a = 255);

  //----------------------------------------------------------------------
  // @Params:
  // "data": Float array (MUST AT LEAST 16 BYTES). Each float corresponds to rgba.
  // "normalized": If true data is expected to be in range 0.0f - 1.0f, otherwise 0.0f - 255.0f.
  //----------------------------------------------------------------------
  explicit Color(const f32* data, bool normalized = true);

  //----------------------------------------------------------------------
  // Construct a new color based on the internal representation.
  // Default is BGRA, that means 0xFF0000 = RED, 0x0000FF = BLUE etc.
  //----------------------------------------------------------------------
  explicit Color(uint32 color, bool setAlphaToMax = true);

  //----------------------------------------------------------------------
  // Constructs a new color from a hexadecimal representation.
  // 0xFF0000 = RED, 0x000000FF = ALPHA set to 1
  // This constructor is independent of the underlying format (BGRA or RGBA).
  //----------------------------------------------------------------------
  explicit Color(const string& hex);

  //----------------------------------------------------------------------
  // Return the greatest of the three R-G-B components (Alpha excluded).
  // @Return:
  //  Value representing the largest component. Range: [0 - 255]
  //----------------------------------------------------------------------
  byte GetMax() const;

  //----------------------------------------------------------------------
  // Calculate the luminance values based on the RGB-Components.
  // @Return:
  //  Luminance value represented by the RGB-Components.
  //----------------------------------------------------------------------
  f32 GetLuminance() const;

  //----------------------------------------------------------------------
  // Return the color as an array of 4 normalized floats. [0.0f - 1.0f]
  // @Return:
  //  4 Floats with all components mapped to the range 0.0f - 1.0f
  //----------------------------------------------------------------------
  std::array<f32, 4> Normalized() const { return { (f32)GetRed() / 255.0f, (f32)GetGreen() / 255.0f, (f32)GetBlue() / 255.0f, (f32)GetAlpha() / 255.0f }; }

  // Set all components. Range: [0 - 255]
  void SetRGBA(byte r, byte g, byte b, byte a = 255);

  // Set single components. Range: [0 - 255]
  void SetRed(byte red);
  void SetGreen(byte green);
  void SetBlue(byte blue);
  void SetAlpha(byte alpha);

  byte GetRed() const;
  byte GetGreen() const;
  byte GetBlue() const;
  byte GetAlpha() const;

  bool    operator == (const Color& c) const;
  bool    operator != (const Color& c) const;
  Color   operator *  (const Color& c) const;
  Color& operator *= (const Color& c);

  // Converts to "(r, g, b)" or "(r, g, b, a)"
  string ToString(bool includeAlpha = false) const;

  //----------------------------------------------------------------------
  // Linearly interpolate between color a and b.
  //----------------------------------------------------------------------
  static Color Lerp(Color a, Color b, f32 lerp);

private:
  f32 _HexToRGB(const string& hex);

public:
  static Color WHITE;
  static Color BLACK;
  static Color RED;
  static Color GREEN;
  static Color BLUE;
  static Color YELLOW;
  static Color VIOLET;
  static Color TURQUOISE;
  static Color GREY;
  static Color ORANGE;
};