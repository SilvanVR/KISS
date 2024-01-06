#include "stdafx_common.h"
#include "color.h"

#ifdef USE_RGBA_FORMAT

#define R_COMPONENT_INDEX 0
#define G_COMPONENT_INDEX 1
#define B_COMPONENT_INDEX 2
#define A_COMPONENT_INDEX 3

#else

#define B_COMPONENT_INDEX 0
#define G_COMPONENT_INDEX 1
#define R_COMPONENT_INDEX 2
#define A_COMPONENT_INDEX 3

#endif

#define RED_COMPONENT(x)      ( (x >> (R_COMPONENT_INDEX * 8)) & 0xFF )
#define GREEN_COMPONENT(x)    ( (x >> (G_COMPONENT_INDEX * 8)) & 0xFF )
#define BLUE_COMPONENT(x)     ( (x >> (B_COMPONENT_INDEX * 8)) & 0xFF )
#define ALPHA_COMPONENT(x)    ( (x >> (A_COMPONENT_INDEX * 8)) & 0xFF )

//---------------------------------------------------------------------------
//  Statics
//---------------------------------------------------------------------------

Color Color::WHITE(255, 255, 255);
Color Color::BLACK(0, 0, 0);
Color Color::RED(255, 0, 0);
Color Color::GREEN(0, 255, 0);
Color Color::BLUE(0, 0, 255);
Color Color::YELLOW(255, 255, 0);
Color Color::VIOLET(255, 0, 255);
Color Color::TURQUOISE(0, 255, 255);
Color Color::GREY(127, 127, 127);
Color Color::ORANGE(255, 127, 0);

//---------------------------------------------------------------------------
Color::Color() : m_componentsAsInt(~0)
{
}

//---------------------------------------------------------------------------
Color::Color(uint32 color, bool setAlphaToMax)
  : m_componentsAsInt(color)
{
  if (setAlphaToMax)
    SetAlpha(255);
}

//---------------------------------------------------------------------------
Color::Color(const f32* data, bool normalized)
{
  byte r = normalized ? (byte)(data[0] * 255.0f) : (byte)data[0];
  byte g = normalized ? (byte)(data[1] * 255.0f) : (byte)data[1];
  byte b = normalized ? (byte)(data[2] * 255.0f) : (byte)data[2];
  byte a = normalized ? (byte)(data[3] * 255.0f) : (byte)data[3];
  SetRGBA(r, g, b, a);
}

//---------------------------------------------------------------------------
Color::Color(const string& hex)
{
  if (hex[0] != '#' || (hex.size() != 7 && hex.size() != 9))
  {
    // string is not a valid hex number
    *this = Color::WHITE;
  }
  else
  {
    SetRed((byte)_HexToRGB(hex.substr(1, 2)));
    SetGreen((byte)_HexToRGB(hex.substr(3, 2)));
    SetBlue((byte)_HexToRGB(hex.substr(5, 2)));
    hex.size() != 9 ? SetAlpha(255) : SetAlpha((byte)_HexToRGB(hex.substr(7, 2)));
  }
}

//---------------------------------------------------------------------------
Color::Color(byte r, byte g, byte b, byte a)
{
  SetRGBA(r, g, b, a);
}

//---------------------------------------------------------------------------
void Color::SetRGBA(byte r, byte g, byte b, byte a)
{
  SetRed(r);
  SetGreen(g);
  SetBlue(b);
  SetAlpha(a);
}

//---------------------------------------------------------------------------
void Color::SetRed(byte red)
{
  m_components[R_COMPONENT_INDEX] = red;
}

//---------------------------------------------------------------------------
void Color::SetGreen(byte green)
{
  m_components[G_COMPONENT_INDEX] = green;
}

//---------------------------------------------------------------------------
void Color::SetBlue(byte blue)
{
  m_components[B_COMPONENT_INDEX] = blue;
}

//---------------------------------------------------------------------------
void Color::SetAlpha(byte alpha)
{
  m_components[A_COMPONENT_INDEX] = alpha;
}

//---------------------------------------------------------------------------
byte Color::GetRed() const
{
  return RED_COMPONENT(m_componentsAsInt);
}

//---------------------------------------------------------------------------
byte Color::GetGreen() const
{
  return GREEN_COMPONENT(m_componentsAsInt);
}

//---------------------------------------------------------------------------
byte Color::GetBlue() const
{
  return BLUE_COMPONENT(m_componentsAsInt);
}

//---------------------------------------------------------------------------
byte Color::GetAlpha() const
{
  return ALPHA_COMPONENT(m_componentsAsInt);
}

//---------------------------------------------------------------------------
byte Color::GetMax() const
{
  byte max;

  byte red = GetRed();
  byte green = GetGreen();
  byte blue = GetBlue();

  max = red > green ? red : green;
  max = max > blue ? max : blue;

  return max;
}

//---------------------------------------------------------------------------
f32 Color::GetLuminance() const
{
  return (0.2126f * GetRed() + 0.7152f * GetGreen() + 0.0722f * GetBlue());
}

//---------------------------------------------------------------------------
bool Color::operator==(const Color& c) const
{
  return (m_componentsAsInt == c.m_componentsAsInt);
}

//---------------------------------------------------------------------------
bool Color::operator!=(const Color& c) const
{
  return !(*this == c);
}

//---------------------------------------------------------------------------
Color Color::operator*(const Color& c) const
{
  auto thisNormalized = Normalized();
  auto cNormalized = c.Normalized();

  Color result;
  result.SetRed(static_cast<byte>((thisNormalized[0] * cNormalized[0]) * 255));
  result.SetGreen(static_cast<byte>((thisNormalized[1] * cNormalized[1]) * 255));
  result.SetBlue(static_cast<byte>((thisNormalized[2] * cNormalized[2]) * 255));
  result.SetAlpha(static_cast<byte>((thisNormalized[3] * cNormalized[3]) * 255));
  return result;
}

//---------------------------------------------------------------------------
Color& Color::operator*=(const Color& c)
{
  *this = *this * c;
  return *this;
}

//---------------------------------------------------------------------------
string Color::ToString(bool includeAlpha) const
{
  assert(false);
  //string result = "(" + TS(getRed()) + ", " + TS(getGreen()) + ", " + TS(getBlue());

  //if (includeAlpha)
  //  result += ", " + TS(getAlpha());

  //result += ")";

  //return result;
  return "";
}

//**********************************************************************
// PUBLIC - STATIC
//**********************************************************************

//---------------------------------------------------------------------------
Color Color::Lerp(Color a, Color b, f32 lerp)
{
  return Color(byte(a.GetRed() * (1.0f - lerp) + b.GetRed() * lerp + 0.5f),
    byte(a.GetGreen() * (1.0f - lerp) + b.GetGreen() * lerp + 0.5f),
    byte(a.GetBlue() * (1.0f - lerp) + b.GetBlue() * lerp + 0.5f),
    byte(a.GetAlpha() * (1.0f - lerp) + b.GetAlpha() * lerp + 0.5f));
}

//**********************************************************************
// PRIVATE
//**********************************************************************

//---------------------------------------------------------------------------
f32 Color::_HexToRGB(const string& hex)
{
  f32 dec = 0;
  for (int32 charIndex = 0; charIndex < hex.size(); charIndex++)
  {
    char c = hex[charIndex];

    if (c >= 48 && c <= 57)
      c -= 48;
    else if (c >= 65 && c <= 70)
      c -= 55;

    dec += c * powf(16.0f, (((f32)hex.size() - charIndex) - 1));
  }
  return dec;
}