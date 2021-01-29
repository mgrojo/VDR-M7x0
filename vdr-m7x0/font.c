/*
 * font.c: Font handling for the DVB On Screen Display
 *
 * See the main source file 'vdr.c' for copyright information and
 * how to reach the author.
 *
 */

#include "config.h"

#include <ctype.h>
#include "font.h"
#include "tools.h"


#include "fontfix-iso8859-1.c"
#include "fontosd-iso8859-1.c"
#include "fontsml-iso8859-1.c"

#ifndef DISABLE_ISO8859_2
#include "fontfix-iso8859-2.c"
#include "fontosd-iso8859-2.c"
#include "fontsml-iso8859-2.c"
#endif

#ifndef DISABLE_ISO8859_5
#include "fontfix-iso8859-5.c"
#include "fontosd-iso8859-5.c"
#include "fontsml-iso8859-5.c"
#endif

#ifndef DISABLE_ISO8859_7
#include "fontfix-iso8859-7.c"
#include "fontosd-iso8859-7.c"
#include "fontsml-iso8859-7.c"
#endif

#ifndef DISABLE_ISO8859_13
#include "fontfix-iso8859-13.c"
#include "fontosd-iso8859-13.c"
#include "fontsml-iso8859-13.c"
#endif

#include "fontfix-iso8859-15.c"
#include "fontosd-iso8859-15.c"
#include "fontsml-iso8859-15.c"

#include "fontsym.c"

// --- cFont -----------------------------------------------------------------

static const void *const FontData[eDvbCodeSize][eDvbFontSize] = {
  { FontOsd_iso8859_1,  FontFix_iso8859_1,  FontSml_iso8859_1,  FontSym },
#ifndef DISABLE_ISO8859_2
  { FontOsd_iso8859_2,  FontFix_iso8859_2,  FontSml_iso8859_2,  FontSym },
#endif
#ifndef DISABLE_ISO8859_5
  { FontOsd_iso8859_5,  FontFix_iso8859_5,  FontSml_iso8859_5,  FontSym },
#endif
#ifndef DISABLE_ISO8859_7
  { FontOsd_iso8859_7,  FontFix_iso8859_7,  FontSml_iso8859_7,  FontSym },
#endif
#ifndef DISABLE_ISO8859_13
  { FontOsd_iso8859_13, FontFix_iso8859_13, FontSml_iso8859_13, FontSym },
#endif
  { FontOsd_iso8859_15, FontFix_iso8859_15, FontSml_iso8859_15, FontSym },
  };

static const char *FontCode[eDvbCodeSize] = {
  "iso8859-1",
#ifndef DISABLE_ISO8859_2
  "iso8859-2",
#endif
#ifndef DISABLE_ISO8859_5
  "iso8859-5",
#endif
#ifndef DISABLE_ISO8859_7
  "iso8859-7",
#endif
#ifndef DISABLE_ISO8859_13
  "iso8859-13",
#endif
  "iso8859-15",
  };

eDvbCode cFont::code = code_iso8859_1;

cFont *cFont::fonts[eDvbFontSize] = { NULL };

cFont::cFont(const void *Data)
{
  SetData(Data);
}

void cFont::SetData(const void *Data)
{
  if (Data) {
     height = ((tCharData *)Data)->height;
     for (int i = 0; i < NUMCHARS; i++)
         data[i] = (tCharData *)&((tPixelData *)Data)[(i < 32 ? 0 : i - 32) * (height + 2)];
     }
  else
     height = 0;
}

int cFont::Width(const char *s) const
{
  int w = 0;
  while (s && *s)
        w += Width(*s++);
  return w;
}

int cFont::Height(const char *s) const
{
  int h = 0;
  if (s && *s)
     h = height; // all characters have the same height!
  return h;
}

bool cFont::SetCode(const char *Code)
{
  for (int i = 0; i < eDvbCodeSize; i++) {
      if (strcmp(Code, FontCode[i]) == 0) {
         SetCode(eDvbCode(i));
         return true;
         }
      }
  return false;
}

void cFont::SetCode(eDvbCode Code)
{
  if (code != Code) {
     code = Code;
     for (int i = 0; i < eDvbFontSize; i++) {
         if (fonts[i])
            fonts[i]->SetData(FontData[code][i]);
         }
     }
}

void cFont::SetFont(eDvbFont Font, const void *Data)
{
  delete fonts[Font];
  fonts[Font] = new cFont(Data ? Data : FontData[code][Font]);
}

const cFont *cFont::GetFont(eDvbFont Font)
{
  if (Setup.UseSmallFont == 0 && Font == fontSml)
     Font = fontOsd;
  else if (Setup.UseSmallFont == 2)
     Font = fontSml;
  if (!fonts[Font])
     SetFont(Font);
  return fonts[Font];
}

// --- cTextWrapper ----------------------------------------------------------

cTextWrapper::cTextWrapper(void)
{
  text = eol = NULL;
  lines = 0;
  lastLine = -1;
}

cTextWrapper::cTextWrapper(const char *Text, const cFont *Font, int Width)
{
  text = NULL;
  Set(Text, Font, Width);
}

cTextWrapper::~cTextWrapper()
{
  free(text);
}

void cTextWrapper::Set(const char *Text, const cFont *Font, int Width)
{
  free(text);
  text = Text ? strdup(Text) : NULL;
  eol = NULL;
  lines = 0;
  lastLine = -1;
  if (!text)
     return;
  lines = 1;
  if (Width <= 0)
     return;

  char *Blank = NULL;
  char *Delim = NULL;
  int w = 0;

  stripspace(text); // strips trailing newlines

  for (char *p = text; *p; ) {
      if (*p == '\n') {
         lines++;
         w = 0;
         Blank = Delim = NULL;
         p++;
         continue;
         }
      else if (isspace(*p))
         Blank = p;
      int cw = Font->Width(*p);
      if (w + cw > Width) {
         if (Blank) {
            *Blank = '\n';
            p = Blank;
            continue;
            }
         else if (w > 0) { // there has to be at least one character before the newline
            // Here's the ugly part, where we don't have any whitespace to
            // punch in a newline, so we need to make room for it:
            if (Delim)
               p = Delim + 1; // let's fall back to the most recent delimiter
            char *s = MALLOC(char, strlen(text) + 2); // The additional '\n' plus the terminating '\0'
            int l = p - text;
            strncpy(s, text, l);
            s[l] = '\n';
            strcpy(s + l + 1, p);
            free(text);
            text = s;
            p = text + l;
            continue;
            }
         }
      w += cw;
      if (strchr("-.,:;!?_", *p)) {
         Delim = p;
         Blank = NULL;
         }
      p++;
      }
}

const char *cTextWrapper::Text(void)
{
  if (eol) {
     *eol = '\n';
     eol = NULL;
     }
  return text;
}

const char *cTextWrapper::GetLine(int Line)
{
  char *s = NULL;
  if (Line < lines) {
     if (eol) {
        *eol = '\n';
        if (Line == lastLine + 1)
           s = eol + 1;
        eol = NULL;
        }
     if (!s) {
        s = text;
        for (int i = 0; i < Line; i++) {
            s = strchr(s, '\n');
            if (s)
               s++;
            else
               break;
            }
        }
     if (s) {
        if ((eol = strchr(s, '\n')) != NULL)
           *eol = 0;
        }
     lastLine = Line;
     }
  return s;
}
