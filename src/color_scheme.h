#ifndef COLOR_SCHEME_H
#define COLOR_SCHEME_H

enum ColorScheme : unsigned int
{
    ColorWindow,
    ColorNegative,
    ColorError,
    ColorMultipleErrors,
    ColorGray
};

void init_colors();

#endif // COLOR_SCHEME_H
