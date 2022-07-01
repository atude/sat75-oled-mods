#include <stdio.h>
#include <string.h>

#define min(x, y) (((x) >= (y)) ? (y) : (x))

static void l_a(uint8_t x, uint8_t y, bool on);
static void l_m(uint8_t x, uint8_t y, bool on);
static void l_p(uint8_t x, uint8_t y, bool on);
static void l_p(uint8_t x, uint8_t y, bool on);
static void n_colon(uint8_t x, uint8_t y, bool on);
static void n_zero(uint8_t x, uint8_t y, bool on);
static void n_one(uint8_t x, uint8_t y, bool on);
static void n_two(uint8_t x, uint8_t y, bool on);
static void n_three(uint8_t x, uint8_t y, bool on);
static void n_four(uint8_t x, uint8_t y, bool on);
static void n_five(uint8_t x, uint8_t y, bool on);
static void n_six(uint8_t x, uint8_t y, bool on);
static void n_seven(uint8_t x, uint8_t y, bool on);
static void n_eight(uint8_t x, uint8_t y, bool on);
static void n_nine(uint8_t x, uint8_t y, bool on);

/**
 * Generic helpers
 */

static void draw_line_h(uint8_t x, uint8_t y, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        oled_write_pixel(i + x, y, true);
    }
}

static void draw_line_v(uint8_t x, uint8_t y, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        oled_write_pixel(x, i + y, true);
    }
}

static void draw_rectangle(uint8_t x, uint8_t y, uint8_t x_len, uint8_t y_len, bool on) {
    for (uint8_t i = 0; i < x_len; i++) {
        for (uint8_t j = 0; j < y_len; j++) {
            oled_write_pixel(i + x, j + y, on);
        }
    }
}

// Draws from bottom y to top y. Only cursor x controllable
static void draw_wpm_bar(uint8_t cursor_x, uint8_t wpm, char* date) {
    uint8_t x     = (cursor_x * 5) + cursor_x - 1;
    uint8_t y     = 5;
    uint8_t y_len = min(wpm / 6, 22);
    for (uint8_t i = 0; i < 13; i++) {
        for (uint8_t j = 0; j < y_len; j++) {
            oled_write_pixel(i + x, 22 - j + y, true);
        }
    }
    draw_line_v(x - 3, y, 24);
    draw_line_v(x - 2, y - 1, 26);
    draw_line_v(x + 14, y - 1, 26);
    draw_line_v(x + 15, y, 24);
    draw_line_h(x - 1, y - 2, 15);
    draw_line_h(x - 2, y - 1, 16);
    draw_line_h(x - 2, y + 24, 16);
    draw_line_h(x - 1, y + 25, 15);

    // Embed date in wpm bar
    if (wpm < 20) {
        char date_first_section[3]  = {date[0], date[1], '\0'};
        char date_second_section[3] = {date[3], date[4], '\0'};
        oled_set_cursor(cursor_x, 1);
        oled_write(date_first_section, false);
        oled_set_cursor(cursor_x, 2);
        oled_write(date_second_section, false);
    }
}

/**
 * Feature helpers
 */

// Mods square (S, C, A, G)
static void draw_mods_square(uint8_t mod_state, int8_t enc_turn_state, bool show_enc_turn, uint8_t cursor_x, uint8_t cursor_y) {
    uint8_t x = (cursor_x * 5) + cursor_x - 1;
    uint8_t y = cursor_y * 8;
    oled_set_cursor(cursor_x, cursor_y);
    draw_line_h(x, y - 1, 7);
    draw_line_h(x, y + 7, 7);
    draw_line_v(x - 1, y, 7);
    draw_line_v(x + 7, y, 7);
    if ((mod_state & MOD_MASK_SHIFT) || (mod_state & MOD_MASK_CTRL) || (mod_state & MOD_MASK_ALT) || (mod_state & MOD_MASK_GUI) || (enc_turn_state != 0 && show_enc_turn)) {
        draw_rectangle(x, y - 1, 7, 9, true);
        if (mod_state & MOD_MASK_SHIFT) {
            oled_write_P(PSTR("S"), true);
        } else if (mod_state & MOD_MASK_CTRL) {
            oled_write_P(PSTR("C"), true);
        } else if (mod_state & MOD_MASK_ALT) {
            oled_write_P(PSTR("A"), true);
        } else if (mod_state & MOD_MASK_GUI) {
            oled_write_P(PSTR("G"), true);
        } else if (enc_turn_state >= 1) {
            oled_write_P(PSTR("+"), true);
        } else if (enc_turn_state <= 1) {
            oled_write_P(PSTR("-"), true);
        }
    }
}

// Info panel (vol, caps, layer) - horizontal line
static void draw_info_panel(led_t led_state, uint8_t layer_state, char* enc_mode, uint8_t cursor_x, uint8_t cursor_y, bool is_long_caps) {
    uint8_t x               = (cursor_x * 5) + cursor_x - 1;
    uint8_t y               = cursor_y * 8;
    uint8_t long_caps_x_add = is_long_caps ? 6 : 0;

    oled_set_cursor(cursor_x, cursor_y);
    if (enc_press_state > 0) {
        draw_rectangle(x, y - 1, 19, 10, true);
        draw_line_v(x - 1, y, 8);
        draw_line_v(x + 19, y, 8);
        oled_write(enc_mode, true);
    } else {
        oled_write(enc_mode, false);
    }
    oled_advance_char();
    if (led_state.caps_lock) {
        draw_rectangle(x + 24, y - 1, 19 + long_caps_x_add, 10, true);
        draw_line_v(x + 24 - 1, y, 8);
        draw_line_v(x + 24 + 19 + long_caps_x_add, y, 8);
        oled_write_P(PSTR(is_long_caps ? "CAPS" : "CAP"), true);
    } else {
        oled_write_P(PSTR(is_long_caps ? "CAPS" : "CAP"), false);
    }
    oled_advance_char();
    if (biton32(layer_state) > 0) {
        draw_rectangle(x + 48 + long_caps_x_add, y - 1, 12, 10, true);
        draw_line_v(x + 48 - 1 + long_caps_x_add, y, 8);
        draw_line_v(x + 48 + 12 + long_caps_x_add, y, 8);
        oled_write_P(PSTR("L"), true);
        oled_write_char(get_highest_layer(layer_state) + 0x30, true);
    } else {
        oled_write_P(PSTR("L"), false);
        oled_write_char(get_highest_layer(layer_state) + 0x30, false);
    }
}

// Big clock
static void draw_big_clock(uint16_t last_minute, uint8_t x, uint8_t y, bool is_24hr) {
    uint8_t  hour         = last_minute / 60;
    uint8_t  hour_digit   = 0;
    uint16_t minute       = last_minute % 60;
    uint16_t minute_digit = 0;
    int8_t   digit_w      = 10;
    int8_t   digit_h      = 14;
    bool     is_pm        = (hour / 12) > 0;

    if (!is_24hr) {
        hour = hour % 12;
        if (hour == 0) {
            hour = 12;
        }
    }

    // Hour First Digit
    if (hour < 10) {
        n_zero(x, y, true);
        hour_digit = hour;
    } else if (hour < 20) {
        n_one(x, y, true);
        hour_digit = hour - 10;
    } else if (hour < 24) {
        n_two(x, y, true);
        hour_digit = hour - 20;
    } else if (hour == 24) {
        n_zero(x, y, true);
        n_zero(x + digit_w, y, true);
    }
    // Hour Second Digit
    if (hour_digit == 1) {
        n_one(x + digit_w, y, true);
    } else if (hour_digit == 2) {
        n_two(x + digit_w, y, true);
    } else if (hour_digit == 3) {
        n_three(x + digit_w, y, true);
    } else if (hour_digit == 4) {
        n_four(x + digit_w, y, true);
    } else if (hour_digit == 5) {
        n_five(x + digit_w, y, true);
    } else if (hour_digit == 6) {
        n_six(x + digit_w, y, true);
    } else if (hour_digit == 7) {
        n_seven(x + digit_w, y, true);
    } else if (hour_digit == 8) {
        n_eight(x + digit_w, y, true);
    } else if (hour_digit == 9) {
        n_nine(x + digit_w, y, true);
    } else if (hour_digit == 0) {
        n_zero(x + digit_w, y, true);
    }
    // Colon
    n_colon(x + digit_w * 2, y, true);
    // Minute First Digit
    if (minute < 10) {
        n_zero(x + digit_w * 3, y, true);
        minute_digit = minute;
    } else if (minute < 20) {
        n_one(x + digit_w * 3, y, true);
        minute_digit = minute - 10;
    } else if (minute < 30) {
        n_two(x + digit_w * 3, y, true);
        minute_digit = minute - 20;
    } else if (minute < 30) {
        n_two(x + digit_w * 3, y, true);
        minute_digit = minute - 20;
    } else if (minute < 40) {
        n_three(x + digit_w * 3, y, true);
        minute_digit = minute - 30;
    } else if (minute < 50) {
        n_four(x + digit_w * 3, y, true);
        minute_digit = minute - 40;
    } else if (minute < 50) {
        n_four(x + digit_w * 3, y, true);
        minute_digit = minute - 40;
    } else if (minute < 60) {
        n_five(x + digit_w * 3, y, true);
        minute_digit = minute - 50;
    } else if (minute == 60) {
        n_zero(x + digit_w * 3, y, true);
    }
    // Minute Second Digit
    if (minute % 10 == 0) {
        n_zero(x + digit_w * 4, y, true);
    } else if (minute_digit == 1) {
        n_one(x + digit_w * 4, y, true);
    } else if (minute_digit == 2) {
        n_two(x + digit_w * 4, y, true);
    } else if (minute_digit == 3) {
        n_three(x + digit_w * 4, y, true);
    } else if (minute_digit == 4) {
        n_four(x + digit_w * 4, y, true);
    } else if (minute_digit == 5) {
        n_five(x + digit_w * 4, y, true);
    } else if (minute_digit == 6) {
        n_six(x + digit_w * 4, y, true);
    } else if (minute_digit == 7) {
        n_seven(x + digit_w * 4, y, true);
    } else if (minute_digit == 8) {
        n_eight(x + digit_w * 4, y, true);
    } else if (minute_digit == 9) {
        n_nine(x + digit_w * 4, y, true);
    }

    int8_t l_w = 12;
    int8_t l_h = 10;

    if (!is_24hr) {
        if (is_pm) {
            l_p(x + digit_w * 5 + 4, y + digit_h - l_h, true);
            l_m(x + digit_w * 5 + 4 + l_w, y + digit_h - l_h, true);
        } else {
            l_a(x + digit_w * 5 + 4, y + digit_h - l_h, true);
            l_m(x + digit_w * 5 + 4 + l_w, y + digit_h - l_h, true);
        }
    }
}

/**
 * Big clock helpers
 */
static void l_a(uint8_t x, uint8_t y, bool on) {
    draw_rectangle(x, y + 2, 2, 8, on);
    draw_rectangle(x + 8, y + 2, 2, 8, on);
    draw_rectangle(x + 2, y, 6, 2, on);
    draw_rectangle(x + 2, y + 6, 6, 2, on);
}

static void l_m(uint8_t x, uint8_t y, bool on) {
    draw_rectangle(x, y, 2, 10, on);
    draw_rectangle(x + 8, y, 2, 10, on);
    draw_rectangle(x + 2, y + 2, 2, 2, on);
    draw_rectangle(x + 6, y + 2, 2, 2, on);
    draw_rectangle(x + 4, y + 4, 2, 2, on);
}

static void l_p(uint8_t x, uint8_t y, bool on) {
    draw_rectangle(x, y, 2, 10, on);
    draw_rectangle(x + 2, y, 6, 2, on);
    draw_rectangle(x + 2, y + 6, 6, 2, on);
    draw_rectangle(x + 8, y + 2, 2, 4, on);
}

static void n_colon(uint8_t x, uint8_t y, bool on) {
    draw_rectangle(x + 3, y + 3, 2, 2, on);
    draw_rectangle(x + 3, y + 9, 2, 2, on);
}

static void n_zero(uint8_t x, uint8_t y, bool on) {
    draw_rectangle(x + 2, y, 4, 14, on);
    draw_rectangle(x, y + 2, 8, 10, on);
    draw_rectangle(x + 2, y + 2, 4, 10, !on);
}

static void n_one(uint8_t x, uint8_t y, bool on) {
    draw_rectangle(x + 4, y, 2, 12, on);
    draw_rectangle(x + 2, y + 2, 2, 2, on);
    draw_rectangle(x + 2, y + 12, 6, 2, on);
}

static void n_two(uint8_t x, uint8_t y, bool on) {
    draw_rectangle(x + 2, y, 4, 2, on);
    draw_rectangle(x, y + 2, 2, 2, on);
    draw_rectangle(x + 6, y + 2, 2, 4, on);
    draw_rectangle(x + 4, y + 6, 2, 2, on);
    draw_rectangle(x + 2, y + 8, 2, 2, on);
    draw_rectangle(x, y + 10, 2, 2, on);
    draw_rectangle(x, y + 12, 8, 2, on);
}

static void n_three(uint8_t x, uint8_t y, bool on) {
    draw_rectangle(x + 2, y, 4, 2, on);
    draw_rectangle(x, y + 2, 2, 2, on);
    draw_rectangle(x + 6, y + 2, 2, 4, on);
    draw_rectangle(x + 2, y + 6, 4, 2, on);
    draw_rectangle(x + 6, y + 8, 2, 4, on);
    draw_rectangle(x, y + 10, 2, 2, on);
    draw_rectangle(x + 2, y + 12, 4, 2, on);
}

static void n_four(uint8_t x, uint8_t y, bool on) {
    draw_rectangle(x, y, 2, 6, on);
    draw_rectangle(x + 2, y + 6, 4, 2, on);
    draw_rectangle(x + 6, y, 2, 14, on);
}

static void n_five(uint8_t x, uint8_t y, bool on) {
    draw_rectangle(x, y, 8, 2, on);
    draw_rectangle(x, y + 2, 2, 2, on);
    draw_rectangle(x, y + 4, 6, 2, on);
    draw_rectangle(x + 6, y + 6, 2, 6, on);
    draw_rectangle(x, y + 10, 2, 2, on);
    draw_rectangle(x + 2, y + 12, 4, 2, on);
}

static void n_six(uint8_t x, uint8_t y, bool on) {
    draw_rectangle(x + 2, y, 4, 2, on);
    draw_rectangle(x + 6, y + 2, 2, 2, on);
    draw_rectangle(x, y + 2, 2, 10, on);
    draw_rectangle(x + 2, y + 6, 4, 2, on);
    draw_rectangle(x + 6, y + 8, 2, 4, on);
    draw_rectangle(x + 2, y + 12, 4, 2, on);
}

static void n_seven(uint8_t x, uint8_t y, bool on) {
    draw_rectangle(x, y, 8, 2, on);
    draw_rectangle(x + 6, y + 2, 2, 4, on);
    draw_rectangle(x + 4, y + 6, 2, 4, on);
    draw_rectangle(x + 2, y + 10, 2, 4, on);
}

static void n_eight(uint8_t x, uint8_t y, bool on) {
    draw_rectangle(x + 2, y, 4, 14, on);
    draw_rectangle(x, y + 2, 8, 4, on);
    draw_rectangle(x, y + 8, 8, 4, on);
    draw_rectangle(x + 2, y + 2, 4, 4, !on);
    draw_rectangle(x + 2, y + 8, 4, 4, !on);
}

static void n_nine(uint8_t x, uint8_t y, bool on) {
    draw_rectangle(x + 2, y, 4, 14, on);
    draw_rectangle(x, y + 2, 8, 10, on);
    draw_rectangle(x + 2, y + 2, 4, 4, !on);
    draw_rectangle(x, y + 6, 2, 4, !on);
    draw_rectangle(x + 2, y + 8, 4, 4, !on);
}
