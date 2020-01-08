#include "color_scheme.h"
#include <ncurses.h>


void init_colors()
{
    start_color();
    use_default_colors();
    init_pair(ColorScheme::ColorWindow, COLOR_BLACK, COLOR_WHITE);
    init_pair(ColorScheme::ColorNegative, COLOR_WHITE, COLOR_BLACK);
    init_pair(ColorScheme::ColorError, COLOR_RED, COLOR_WHITE);
    init_pair(ColorScheme::ColorMultipleErrors, COLOR_WHITE, COLOR_RED);
    init_pair(ColorScheme::ColorGray, COLORS > 255 ? 251 : COLOR_BLACK, COLOR_WHITE); // https://jonasjacek.github.io/colors/
}
