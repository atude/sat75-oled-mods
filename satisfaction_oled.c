#include "satisfaction75.h"
#include "draw.h"
#include "frames.h"
#include "render_gif.h"
#include <stdio.h>

void draw_settings(void);  // set time/date and settings
void draw_matrix(void);
void draw_clock(void);
void draw_bongo_dynamic(void);
void draw_luna(void);
void draw_pusheen(void);
void draw_kirby(void);

#define min(x, y) (((x) >= (y)) ? (y) : (x))
#define max(x, y) (((x) >= (y)) ? (x) : (y))
#define wpm() get_current_wpm()

#ifdef OLED_ENABLE

/* Matrix display is 12 x 12 pixels - def position is 0, 0*/
#    define MATRIX_DISPLAY_X 1
#    define MATRIX_DISPLAY_Y 5
#    define MATRIX_SCALE 3

/* Shared defines */
#    define PRE_SLEEP_TIMEOUT (get_timeout() - 30000)

// Generic variables
uint32_t anim_timer     = 0;
uint8_t  prev_oled_mode = 0;
bool     space_pressed  = false;
bool     showed_jump    = true;

// Bongo frames and variables
uint8_t bongo_current_idle_frame     = 0;
uint8_t bongo_current_pre_idle_frame = 0;
uint8_t bongo_current_tap_frame      = 0;
uint8_t bongo_current_caps_frame     = 0;

// Luna frames and variables
uint8_t luna_current_frame = 0;

// Kirby frames and variables
uint8_t kirby_current_idle_frame      = 0;
uint8_t kirby_current_walk_frame      = 0;
uint8_t kirby_current_jump_frame      = 0;
uint8_t kirby_current_inhale_frame    = 0;
uint8_t kirby_current_exhale_frame    = 0;
uint8_t kirby_current_caps_idle_frame = 0;
uint8_t kirby_caps_state              = 0;  // 0 = no caps, 1 = in caps, 2 = exiting caps

// Pusheen frames and variables
uint8_t pusheen_current_frame = 0;
uint8_t pusheen_anim_type     = 0;

// Dynamic bongo variables
uint8_t bongo_state_tap = 0;

oled_rotation_t oled_init_kb(oled_rotation_t rotation) { return OLED_ROTATION_0; }

bool process_record_user(uint16_t keycode, keyrecord_t* record) {
    switch (keycode) {
        case KC_SPC:
            if (record->event.pressed) {
                space_pressed = true;
                // showed_jump   = false;
            } else {
                space_pressed = false;
            }
            break;
        case KC_F22:
        case KC_F23:
        case KC_F24:
            anim_timer = 99;  // forces an animation reset
            break;
        case KC_CAPS:
        case KC_LCTL:
        case KC_RCTL:
        case KC_LGUI:
        case KC_RGUI:
            if (oled_mode == OLED_PETS) {
                anim_timer = 99;  // forces an animation reset
            }
            return true;
    }
    if (record->event.pressed) {
        bongo_state_tap = 1;
    }
    return true;
}

static bool is_bongo_filled(void) { return bongo_mode == 0 ? false : true; }

static bool is_24hr_time(void) { return date_time_format_mode == 1 || date_time_format_mode == 3; }

static bool is_month_first_date(void) { return date_time_format_mode == 2 || date_time_format_mode == 3; }

static uint32_t get_timeout(void) {
    switch (timeout_mode) {
        default:
        case 0:
            return 90000;  // 1m30s
        case 1:
            return 120000;  // 2m
        case 2:
            return 300000;  // 5m
        case 3:
            return 60000;  // 1m
    }
}

static char* get_date_time_format_str(void) {
    static char format_str[13] = "";
    switch (date_time_format_mode) {
        default:
        case 0:
            sprintf(format_str, "12h dd/mm/yy");
            break;
        case 1:
            sprintf(format_str, "24h dd/mm/yy");
            break;
        case 2:
            sprintf(format_str, "12h mm/dd/yy");
            break;
        case 3:
            sprintf(format_str, "24h mm/dd/yy");
            break;
    }
    return format_str;
}

static char* get_timeout_str(void) {
    static char timeout_str[6] = "";
    switch (timeout_mode) {
        default:
        case 0:
            sprintf(timeout_str, "1m30s");  // 1m30s
            break;
        case 1:
            sprintf(timeout_str, "2m");  // 2m
            break;
        case 2:
            sprintf(timeout_str, "5m");  // 5m
            break;
        case 3:
            sprintf(timeout_str, "1m");  // 1m
            break;
    }
    return timeout_str;
}

static char* get_wpm_str(void) {
    static char wpm_str[5] = "";
    if (wpm() > 0) {
        sprintf(wpm_str, "%03d", wpm());
    } else {
        sprintf(wpm_str, "wpm");
    }
    return wpm_str;
}

void reset_animations(void) {
    anim_timer  = 0;
    showed_jump = true;
    oled_clear();
    oled_on();
}

void draw_current_pet(void) {
    switch (pet_mode) {
        default:
        case PET_LUNA:
            draw_luna();
            return;
        case PET_PUSHEEN:
            draw_pusheen();
            return;
        case PET_KIRBY:
            draw_kirby();
            return;
    }
}

static uint8_t get_frame_duration(void) { return max(200 - (wpm() * 0.65), 75); }

bool oled_task_kb(void) {
    if (!oled_task_user()) {
        return false;
    }
    if (!oled_task_needs_to_repaint()) {
        return false;
    }
    if (clock_set_mode) {
        oled_clear();
        draw_settings();
        return false;
    }
    if (prev_oled_mode != oled_mode) {
        reset_animations();
        prev_oled_mode = oled_mode;
    }
    switch (oled_mode) {
        default:
        case OLED_DEFAULT:
            oled_clear();
            draw_matrix();
            break;
        case OLED_CLOCK:
            oled_clear();
            draw_clock();
            break;
        case OLED_BONGO:
            draw_bongo_dynamic();
            break;
        case OLED_PETS:
            draw_current_pet();
            break;
        case OLD_GIF:
            draw_gif();
            break;
        case OLED_OFF:
            oled_off();
            break;
    }
    return false;
}

// Request a repaint of the OLED image without resetting the OLED sleep timer.
// Used for things like clock updates that should not keep the OLED turned on
// if there is no other activity.
void oled_request_repaint(void) {
    if (is_oled_on()) {
        oled_repaint_requested = true;
    }
}

// Request a repaint of the OLED image and reset the OLED sleep timer.
// Needs to be called after any activity that should keep the OLED turned on.
void oled_request_wakeup(void) { oled_wakeup_requested = true; }

// Check whether oled_task_user() needs to repaint the OLED image.  This
// function should be called at the start of oled_task_user(); it also handles
// the OLED sleep timer and the OLED_OFF mode.
bool oled_task_needs_to_repaint(void) {
    // In the OLED_OFF mode the OLED is kept turned off; any wakeup requests
    // are ignored.
    if ((oled_mode == OLED_OFF) && !clock_set_mode) {
        oled_wakeup_requested  = false;
        oled_repaint_requested = false;
        oled_off();
        return false;
    }

    // If OLED wakeup was requested, reset the sleep timer and do a repaint.
    if (oled_wakeup_requested) {
        oled_wakeup_requested  = false;
        oled_repaint_requested = false;
        oled_sleep_timer       = timer_read32() + get_timeout();
        oled_on();
        return true;
    }

    // If OLED repaint was requested, just do a repaint without touching the
    // sleep timer.
    if (oled_repaint_requested) {
        oled_repaint_requested = false;
        return true;
    }

    // If the OLED is currently off, skip the repaint (which would turn the
    // OLED on if the image is changed in any way).
    if (!is_oled_on()) {
        return false;
    }

    // If the sleep timer has expired while the OLED was on, turn the OLED off.
    if (timer_expired32(timer_read32(), oled_sleep_timer)) {
        oled_off();
        return false;
    }

    // Always perform a repaint if the OLED is currently on.  (This can
    // potentially be optimized to avoid unneeded repaints if all possible
    // state changes are covered by oled_request_repaint() or
    // oled_request_wakeup(), but then any missed calls to these functions
    // would result in displaying a stale image.)
    return true;
}

static char* get_enc_mode(void) {
    switch (encoder_mode) {
        default:
        case ENC_MODE_VOLUME:
            return "VOL";
        case ENC_MODE_MEDIA:
            return "MED";
        case ENC_MODE_SCROLL:
            return "SCR";
        case ENC_MODE_BRIGHTNESS:
            return "BRT";
        case ENC_MODE_BACKLIGHT:
            return "BKL";
        case ENC_MODE_CLOCK_SET:
            return "CLK";
        case ENC_MODE_CUSTOM0:
            return "CS0";
        case ENC_MODE_CUSTOM1:
            return "CS1";
        case ENC_MODE_CUSTOM2:
            return "CS2";
    }
}

static char* get_time(void) {
    uint8_t  hour   = last_minute / 60;
    uint16_t minute = last_minute % 60;

    if (encoder_mode == ENC_MODE_CLOCK_SET) {
        hour   = hour_config;
        minute = minute_config;
    }

    bool is_pm = (hour / 12) > 0;
    if (!is_24hr_time()) {
        hour = hour % 12;
        if (hour == 0) {
            hour = 12;
        }
    }

    static char time_str[11] = "";
    if (!is_24hr_time()) {
        sprintf(time_str, "%02d:%02d%s", hour, minute, is_pm ? "pm" : "am");
    } else {
        sprintf(time_str, "%02d:%02d", hour, minute);
    }

    return time_str;
}

static char* get_date(bool show_full) {
    int16_t year  = last_timespec.year + 1980;
    int8_t  month = last_timespec.month;
    int8_t  day   = last_timespec.day;

    if (encoder_mode == ENC_MODE_CLOCK_SET) {
        year  = year_config + 1980;
        month = month_config;
        day   = day_config;
    }

    static char date_str[15] = "";
    if (show_full) {
        if (!is_month_first_date()) {
            sprintf(date_str, "%02d/%02d/%04d", day, month, year);
        } else {
            sprintf(date_str, "%02d/%02d/%04d", month, day, year);
        }
    } else {
        if (!is_month_first_date()) {
            sprintf(date_str, "%02d/%02d/%02d", day, month, year % 100);
        } else {
            sprintf(date_str, "%02d/%02d/%02d", month, day, year % 100);
        }
    }

    return date_str;
}

void draw_pet_text_section(void) {
    led_t led_state = host_keyboard_led_state();
    switch (date_time_mode) {
        default:
        case 0:
            oled_set_cursor(is_24hr_time() ? 16 : 14, 1);
            oled_write(get_time(), false);
            break;
        case 1:
            oled_set_cursor(13, 1);
            oled_write(get_date(false), false);
            break;
        case 2:
            switch (pet_mode) {
                default:
                case PET_LUNA:
                    oled_set_cursor(17, 1);
                    oled_write_P(PSTR("LUNA"), false);
                    break;
                case PET_PUSHEEN:
                    oled_set_cursor(14, 1);
                    oled_write_P(PSTR("PUSHEEN"), false);
                    break;
                case PET_KIRBY:
                    oled_set_cursor(16, 1);
                    oled_write_P(PSTR("KIRBY"), false);
                    break;
            }
            break;
    }

    if (led_state.caps_lock) {
        oled_set_cursor(12, 3);
        oled_write_P(PSTR("CAPS"), false);
    } else {
        oled_set_cursor(13, 3);
        oled_write(get_enc_mode(), false);
    }
    oled_write_P(PSTR("   L"), false);
    draw_line_v(104, 24, 8);
    oled_write_char(get_highest_layer(layer_state) + 0x30, false);
}

// Custom matrix
void draw_matrix() {
    // matrix
    for (uint8_t x = 0; x < MATRIX_ROWS; x++) {
        for (uint8_t y = 0; y < MATRIX_COLS; y++) {
            bool on = (matrix_get_row(x) & (1 << y)) > 0;

            if (on) {
                if (matrix_is_on(5, 5)) {
                    // spacebar
                    draw_rectangle(MATRIX_DISPLAY_X + (7 * MATRIX_SCALE) - 3, MATRIX_DISPLAY_Y + (7 * MATRIX_SCALE), MATRIX_SCALE * 5, MATRIX_SCALE, true);
                }
                if (matrix_is_on(3, 0)) {
                    // caps lock
                    draw_rectangle(MATRIX_DISPLAY_X + ((0 + 2) * MATRIX_SCALE), MATRIX_DISPLAY_Y + ((3 + 2) * MATRIX_SCALE), MATRIX_SCALE * 2, MATRIX_SCALE, true);
                }
                if (matrix_is_on(4, 0)) {
                    // lshift
                    draw_rectangle(MATRIX_DISPLAY_X + ((0 + 2) * MATRIX_SCALE), MATRIX_DISPLAY_Y + ((4 + 2) * MATRIX_SCALE), MATRIX_SCALE * 2, MATRIX_SCALE, true);
                }
                if (matrix_is_on(1, 13)) {
                    // backspace and enter
                    draw_rectangle(-2 + MATRIX_DISPLAY_X + ((13 + 2) * MATRIX_SCALE), MATRIX_DISPLAY_Y + ((1 + 2) * MATRIX_SCALE), MATRIX_SCALE * 2, MATRIX_SCALE, true);
                }
                if (matrix_is_on(3, 13)) {
                    // backspace and enter
                    draw_rectangle(-2 + MATRIX_DISPLAY_X + ((13 + 2) * MATRIX_SCALE), MATRIX_DISPLAY_Y + ((3 + 2) * MATRIX_SCALE), MATRIX_SCALE * 2, MATRIX_SCALE, true);
                }
                if (matrix_is_on(5, 15)) {
                    // align right arrow key
                    draw_rectangle(-3 + MATRIX_DISPLAY_X + ((15 + 2) * MATRIX_SCALE), MATRIX_DISPLAY_Y + ((5 + 2) * MATRIX_SCALE), MATRIX_SCALE, MATRIX_SCALE, true);
                }
                // dont print misaligned parts
                if (!(x == 5 && y == 15)) {
                    // r est of characters
                    draw_rectangle(MATRIX_DISPLAY_X + ((y + 2) * MATRIX_SCALE), MATRIX_DISPLAY_Y + ((x + 2) * MATRIX_SCALE), MATRIX_SCALE, MATRIX_SCALE, true);
                }
            }
        }
    }

    // outline
    // top
    draw_line_h(MATRIX_DISPLAY_X + 2, MATRIX_DISPLAY_Y, 19 * MATRIX_SCALE - 3);
    draw_line_h(MATRIX_DISPLAY_X + 1, MATRIX_DISPLAY_Y + 1, 19 * MATRIX_SCALE - 1);
    // bottom
    draw_line_h(MATRIX_DISPLAY_X + 1, MATRIX_DISPLAY_Y - 1 + (9 * MATRIX_SCALE), 19 * MATRIX_SCALE - 1);
    draw_line_h(MATRIX_DISPLAY_X + 2, MATRIX_DISPLAY_Y + (9 * MATRIX_SCALE), 19 * MATRIX_SCALE - 3);
    // left
    draw_line_v(MATRIX_DISPLAY_X, MATRIX_DISPLAY_Y + 2, 9 * MATRIX_SCALE - 3);
    draw_line_v(MATRIX_DISPLAY_X + 1, MATRIX_DISPLAY_Y + 2, 9 * MATRIX_SCALE - 2);
    // right
    draw_line_v(MATRIX_DISPLAY_X + (19 * MATRIX_SCALE), MATRIX_DISPLAY_Y + 2, 9 * MATRIX_SCALE - 3);
    draw_line_v(MATRIX_DISPLAY_X - 1 + (19 * MATRIX_SCALE), MATRIX_DISPLAY_Y + 2, 9 * MATRIX_SCALE - 2);

    // encoder
    draw_rectangle(MATRIX_DISPLAY_X + 51, MATRIX_DISPLAY_Y + 8, 3, 4, true);
    draw_line_h(MATRIX_DISPLAY_X + 51, MATRIX_DISPLAY_Y + 12, 3);
    draw_line_v(MATRIX_DISPLAY_X + 50, MATRIX_DISPLAY_Y + 9, 3);
    draw_line_v(MATRIX_DISPLAY_X + 54, MATRIX_DISPLAY_Y + 9, 3);

    // oled location
    for (int i = -1; i < MATRIX_SCALE - 1; i++) {
        draw_line_h(2 + MATRIX_DISPLAY_X + (14 * MATRIX_SCALE), -1 + MATRIX_DISPLAY_Y + i + (2 * MATRIX_SCALE), 3 * MATRIX_SCALE);
    }

    // caps lock
    led_t   led_state = host_keyboard_led_state();
    uint8_t mod_state = get_mods();

    if (led_state.caps_lock) {
        draw_rectangle(MATRIX_DISPLAY_X + ((0 + 2) * MATRIX_SCALE), MATRIX_DISPLAY_Y + ((3 + 2) * MATRIX_SCALE), MATRIX_SCALE * 2, MATRIX_SCALE, true);
    }

    // draw wpm bar and outline
    // draw_wpm_bar(68, 5, 6, min(wpm() / 6, 22), true);

    draw_mods_square(mod_state, 11, 1);

    switch (date_time_mode) {
        default:
        case 0:
            oled_set_cursor(is_24hr_time() ? 16 : 14, 1);
            oled_write(get_time(), false);
            break;
        case 1:
            oled_set_cursor(13, 1);
            oled_write(get_date(false), false);
            break;
        case 2:
            oled_set_cursor(16, 1);
            oled_write_P(PSTR("SAT75"), false);
            break;
    }

    oled_set_cursor(11, 3);
    if (encoder_mode != ENC_MODE_VOLUME) {
        draw_rectangle(65, 23, 19, 10, true);
        draw_line_v(65 - 1, 24, 7);
        draw_line_v(65 + 19, 24, 7);
        oled_write(get_enc_mode(), true);
    } else {
        oled_write(get_enc_mode(), false);
    }
    oled_advance_char();
    if (led_state.caps_lock) {
        draw_rectangle(89, 23, 19, 10, true);
        draw_line_v(88, 24, 7);
        draw_line_v(89 + 19, 24, 7);
        oled_write_P(PSTR("CAP"), true);
    } else {
        oled_write_P(PSTR("CAP"), false);
    }
    oled_advance_char();
    if (biton32(layer_state) > 0) {
        draw_rectangle(113, 23, 12, 10, true);
        draw_line_v(113 - 1, 24, 7);
        draw_line_v(113 + 12, 24, 7);
        oled_write_P(PSTR("L"), true);
        oled_write_char(get_highest_layer(layer_state) + 0x30, true);
    } else {
        oled_write_P(PSTR("L"), false);
        oled_write_char(get_highest_layer(layer_state) + 0x30, false);
    }
}

void draw_clock() {
    // led_t   led_state = host_keyboard_led_state();
    uint8_t mod_state = get_mods();

    draw_mods_square(mod_state, 1, 3);
    draw_big_clock(last_minute, 5, 5, is_24hr_time());

    // Date, other details
    // oled_set_cursor(13, 1);
    // oled_write(get_date(false), false);
    // uint8_t x_start = 18;
    // oled_set_cursor(4, 3);
    // if (encoder_mode != ENC_MODE_VOLUME) {
    //     draw_rectangle(x_start, 23, 19, 10, true);
    //     draw_line_v(x_start - 1, 24, 7);
    //     draw_line_v(x_start + 19, 24, 7);
    //     oled_write(get_enc_mode(), true);
    // } else {
    //     oled_write(get_enc_mode(), false);
    // }
    // oled_advance_char();
    // if (led_state.caps_lock) {
    //     draw_rectangle(x_start + 24, 23, 19, 10, true);
    //     draw_line_v(x_start + 24 - 1, 24, 7);
    //     draw_line_v(x_start + 24 + 19, 24, 7);
    //     oled_write_P(PSTR("CAP"), true);
    // } else {
    //     oled_write_P(PSTR("CAP"), false);
    // }
    // oled_advance_char();
    // if (biton32(layer_state) > 0) {
    //     draw_rectangle(x_start + 48, 23, 12, 10, true);
    //     draw_line_v(x_start + 48 - 1, 24, 7);
    //     draw_line_v(x_start + 48 + 12, 24, 7);
    //     oled_write_P(PSTR("L"), true);
    //     oled_write_char(get_highest_layer(layer_state) + 0x30, true);
    // } else {
    //     oled_write_P(PSTR("L"), false);
    //     oled_write_char(get_highest_layer(layer_state) + 0x30, false);
    // }
};

void draw_bongo_dynamic() {
    led_t   led_state = host_keyboard_led_state();
    uint8_t mod_state = get_mods();

    // assumes 1 frame prep stage
    // mode 0 = default, mode 1 = pre idle
    void animation_phase(uint8_t mode) {
        if (led_state.caps_lock) {
            bongo_current_caps_frame = (bongo_current_caps_frame + 1) % BONGO_CAPS_FRAMES;
            oled_write_raw_P(is_bongo_filled() ? bongo_filled_caps[abs((BONGO_CAPS_FRAMES - 1) - bongo_current_caps_frame)] : bongo_caps[abs((BONGO_CAPS_FRAMES - 1) - bongo_current_caps_frame)], DEFAULT_ANIM_SIZE);
            return;
        }
        bongo_current_caps_frame = 0;
        if (mode == 1) {
            bongo_current_pre_idle_frame = (bongo_current_pre_idle_frame + 1) % BONGO_PRE_IDLE_FRAMES;
            oled_write_raw_P(is_bongo_filled() ? bongo_filled_pre_idle[abs((BONGO_PRE_IDLE_FRAMES - 1) - bongo_current_pre_idle_frame)] : bongo_pre_idle[abs((BONGO_PRE_IDLE_FRAMES - 1) - bongo_current_pre_idle_frame)], DEFAULT_ANIM_SIZE);
        } else {
            bongo_current_idle_frame = (bongo_current_idle_frame + 1) % BONGO_IDLE_FRAMES;
            oled_write_raw_P(is_bongo_filled() ? bongo_filled_idle[abs((BONGO_IDLE_FRAMES - 1) - bongo_current_idle_frame)] : bongo_idle[abs((BONGO_IDLE_FRAMES - 1) - bongo_current_idle_frame)], DEFAULT_ANIM_SIZE);
        }
    }

    if ((mod_state & MOD_MASK_CTRL) && bongo_state_tap != 1) {
        oled_write_raw_P(is_bongo_filled() ? bongo_filled_hiding[0] : bongo_hiding[0], DEFAULT_ANIM_SIZE);
        anim_timer      = timer_read32();
        bongo_state_tap = 2;
    } else if ((mod_state & MOD_MASK_GUI) && bongo_state_tap != 1) {
        oled_write_raw_P(is_bongo_filled() ? bongo_filled_blushing[0] : bongo_blushing[0], DEFAULT_ANIM_SIZE);
        anim_timer      = timer_read32();
        bongo_state_tap = 2;
    } else if (bongo_state_tap == 1) {
        if (led_state.caps_lock) {
            oled_write_raw_P(is_bongo_filled() ? bongo_filled_caps[1] : bongo_caps[1], DEFAULT_ANIM_SIZE);
        } else if (mod_state & MOD_MASK_CTRL) {
            bongo_current_tap_frame = (bongo_current_tap_frame + 1) % BONGO_TAP_FRAMES;
            oled_write_raw_P(is_bongo_filled() ? bongo_filled_hiding_tap[abs((BONGO_TAP_FRAMES - 1) - bongo_current_tap_frame)] : bongo_hiding_tap[abs((BONGO_TAP_FRAMES - 1) - bongo_current_tap_frame)], DEFAULT_ANIM_SIZE);
        } else if (mod_state & MOD_MASK_GUI) {
            bongo_current_tap_frame = (bongo_current_tap_frame + 1) % BONGO_TAP_FRAMES;
            oled_write_raw_P(is_bongo_filled() ? bongo_filled_blushing_tap[abs((BONGO_TAP_FRAMES - 1) - bongo_current_tap_frame)] : bongo_blushing_tap[abs((BONGO_TAP_FRAMES - 1) - bongo_current_tap_frame)], DEFAULT_ANIM_SIZE);
        } else if (wpm() > 140) {
            bongo_current_tap_frame = (bongo_current_tap_frame + 1) % BONGO_TAP_FRAMES;
            oled_write_raw_P(is_bongo_filled() ? bongo_filled_tap_cute[abs((BONGO_TAP_FRAMES - 1) - bongo_current_tap_frame)] : bongo_tap_cute[abs((BONGO_TAP_FRAMES - 1) - bongo_current_tap_frame)], DEFAULT_ANIM_SIZE);
        } else {
            bongo_current_tap_frame = (bongo_current_tap_frame + 1) % BONGO_TAP_FRAMES;
            oled_write_raw_P(is_bongo_filled() ? bongo_filled_tap[abs((BONGO_TAP_FRAMES - 1) - bongo_current_tap_frame)] : bongo_tap[abs((BONGO_TAP_FRAMES - 1) - bongo_current_tap_frame)], DEFAULT_ANIM_SIZE);
        }
        anim_timer      = timer_read32();
        bongo_state_tap = 2;
    } else if (timer_elapsed32(anim_timer) < 3000 && bongo_state_tap == 2) {
        if (led_state.caps_lock) {
            oled_write_raw_P(is_bongo_filled() ? bongo_filled_caps[0] : bongo_caps[0], DEFAULT_ANIM_SIZE);
        } else {
            oled_write_raw_P(is_bongo_filled() ? bongo_filled_prep[0] : bongo_prep[0], DEFAULT_ANIM_SIZE);
        }
    } else {
        bongo_state_tap = 0;
        if (get_timeout() + timer_elapsed32(oled_sleep_timer) > PRE_SLEEP_TIMEOUT && !led_state.caps_lock) {
            if (timer_elapsed32(anim_timer) > BONGO_SLEEP_FRAME_DURATION) {
                anim_timer = timer_read32();
                animation_phase(1);
            }
        } else if (timer_elapsed32(anim_timer) > BONGO_IDLE_FRAME_DURATION) {
            anim_timer = timer_read32();
            animation_phase(0);
        }
    }

    // Text drawing
    oled_set_cursor(0, 1);
    switch (date_time_mode) {
        case 0:
            oled_write(get_time(), false);
            break;
        case 1:
            oled_write(get_date(false), false);
            break;
    }

    // draw_rectangle(108, 6, 20, 11, false);
    if (biton32(layer_state) > 0) {
        oled_set_cursor(19, 2);
        oled_write_P(PSTR("L"), false);
        oled_write_char(get_highest_layer(layer_state) + 0x30, false);
    }

    if (led_state.caps_lock) {
        oled_set_cursor(0, 2);
        oled_write_P(PSTR("CAPS"), false);
    }

    if (encoder_mode != ENC_MODE_VOLUME) {
        oled_set_cursor(0, 3);
        oled_write(get_enc_mode(), false);
    }

    if (date_time_mode != 2) {
        oled_set_cursor(18, 3);
        oled_write(get_wpm_str(), false);
    }
}

void draw_luna() {
    led_t   led_state = host_keyboard_led_state();
    uint8_t mod_state = get_mods();

    void animate_luna(void) {
        luna_current_frame = (luna_current_frame + 1) % 2;

        if (led_state.caps_lock) {
            oled_write_raw_P(luna_bark[abs(1 - luna_current_frame)], DEFAULT_ANIM_SIZE);
        } else if (mod_state & MOD_MASK_CTRL) {
            oled_write_raw_P(luna_sneak[abs(1 - luna_current_frame)], DEFAULT_ANIM_SIZE);
        } else if (wpm() <= LUNA_MIN_WALK_SPEED) {
            oled_write_raw_P(luna_sit[abs(1 - luna_current_frame)], DEFAULT_ANIM_SIZE);
        } else if (wpm() <= LUNA_MIN_RUN_SPEED) {
            oled_write_raw_P(luna_walk[abs(1 - luna_current_frame)], DEFAULT_ANIM_SIZE);
        } else {
            oled_write_raw_P(luna_run[abs(1 - luna_current_frame)], DEFAULT_ANIM_SIZE);
        }
    }

    /* animation timer */
    if (space_pressed && !showed_jump && wpm() > LUNA_MIN_RUN_SPEED) {
        oled_write_raw_P(luna_jump[0], DEFAULT_ANIM_SIZE);
        showed_jump = true;
    } else if (timer_elapsed32(anim_timer) > ANIM_FRAME_DURATION) {
        anim_timer  = timer_read32();
        showed_jump = false;
        animate_luna();
    }

    draw_pet_text_section();
}

void draw_pusheen() {
    led_t   led_state = host_keyboard_led_state();
    uint8_t mod_state = get_mods();

    void animate_pusheen(void) {
        pusheen_current_frame = (pusheen_current_frame + 1) % 2;

        if (led_state.caps_lock) {
            oled_write_raw_P(pusheen_cool[abs(1 - pusheen_current_frame)], DEFAULT_ANIM_SIZE);
        } else if (mod_state & MOD_MASK_CTRL) {
            oled_write_raw_P(pusheen_idle[abs(1 - pusheen_current_frame)], DEFAULT_ANIM_SIZE);
        } else if (get_timeout() + timer_elapsed32(oled_sleep_timer) > PRE_SLEEP_TIMEOUT) {
            oled_write_raw_P(pusheen_sleep[abs(1 - pusheen_current_frame)], DEFAULT_ANIM_SIZE);
        } else if (wpm() < 40) {
            oled_write_raw_P(pusheen_wait[abs(1 - pusheen_current_frame)], DEFAULT_ANIM_SIZE);
        } else {
            // Filter random anims
            if (pusheen_anim_type == 1) {
                oled_write_raw_P(pusheen_eat[abs(1 - pusheen_current_frame)], DEFAULT_ANIM_SIZE);
            } else if (pusheen_anim_type == 2) {
                oled_write_raw_P(pusheen_drink[abs(1 - pusheen_current_frame)], DEFAULT_ANIM_SIZE);
            } else if (pusheen_anim_type == 3) {
                oled_write_raw_P(pusheen_walk[abs(1 - pusheen_current_frame)], DEFAULT_ANIM_SIZE);
            } else if (pusheen_anim_type == 4) {
                oled_write_raw_P(pusheen_lick[abs(1 - pusheen_current_frame)], DEFAULT_ANIM_SIZE);
            } else if (pusheen_anim_type == 5) {
                oled_write_raw_P(pusheen_play[abs(1 - pusheen_current_frame)], DEFAULT_ANIM_SIZE);
            }
        }
    }

    /* animation timer */
    if (wpm() < 40 && timer_elapsed32(anim_timer) > PUSHEEN_IDLE_FRAME_DURATION) {
        pusheen_anim_type = 0;
        anim_timer        = timer_read32();
        animate_pusheen();
    } else if (wpm() >= 40 && timer_elapsed32(anim_timer) > PUSHEEN_WALK_FRAME_DURATION) {
        if (pusheen_anim_type == 0) {
            // random animation based on timer
            pusheen_anim_type = (timer_elapsed32(anim_timer) % 5) + 1;
        }
        anim_timer = timer_read32();
        animate_pusheen();
    }

    draw_pet_text_section();
}

void draw_kirby() {
    led_t   led_state = host_keyboard_led_state();
    uint8_t mod_state = get_mods();

    void animate_kirby(void) {
        if (wpm() <= KIRBY_MIN_WALK_SPEED) {
            kirby_current_idle_frame = (kirby_current_idle_frame + 1) % KIRBY_IDLE_FRAMES;
            if (led_state.caps_lock) {
                oled_write_raw_P(kirby_caps_idle[abs((KIRBY_IDLE_FRAMES - 1) - kirby_current_idle_frame)], DEFAULT_ANIM_SIZE);
            } else {
                oled_write_raw_P(kirby_idle[abs((KIRBY_IDLE_FRAMES - 1) - kirby_current_idle_frame)], DEFAULT_ANIM_SIZE);
            }
        } else {
            if (led_state.caps_lock) {
                kirby_current_walk_frame = (kirby_current_walk_frame + 1) % KIRBY_CAPS_WALK_FRAMES;
                oled_write_raw_P(kirby_caps_walk[abs((KIRBY_CAPS_WALK_FRAMES - 1) - kirby_current_walk_frame)], DEFAULT_ANIM_SIZE);
            } else {
                kirby_current_walk_frame = (kirby_current_walk_frame + 1) % KIRBY_WALK_FRAMES;
                oled_write_raw_P(kirby_walk[abs((KIRBY_WALK_FRAMES - 1) - kirby_current_walk_frame)], DEFAULT_ANIM_SIZE);
            }
        }
    }

    if (kirby_caps_state == 1) {
        // Inhale - prepare caps
        if (timer_elapsed32(anim_timer) > KIRBY_INHALE_FRAME_DURATION) {
            oled_write_raw_P(kirby_inhale[min(kirby_current_inhale_frame, (KIRBY_INHALE_FRAMES - 1))], DEFAULT_ANIM_SIZE);
            kirby_current_inhale_frame = kirby_current_inhale_frame + 1;
            anim_timer                 = timer_read32();
            if (kirby_current_inhale_frame > KIRBY_INHALE_FRAMES - 1) {
                kirby_caps_state = 2;
            }
        }
    } else if (kirby_caps_state == 3) {
        // Exhale - exit caps
        if (timer_elapsed32(anim_timer) > KIRBY_EXHALE_FRAME_DURATION) {
            oled_write_raw_P(kirby_exhale[min(kirby_current_exhale_frame, (KIRBY_EXHALE_FRAMES - 1))], DEFAULT_ANIM_SIZE);
            kirby_current_exhale_frame = kirby_current_exhale_frame + 1;
            anim_timer                 = timer_read32();
            if (kirby_current_exhale_frame > KIRBY_EXHALE_FRAMES - 1) {
                kirby_caps_state = 0;
            }
        }
    } else if (!showed_jump) {
        // Jump
        if (timer_elapsed32(anim_timer) > KIRBY_JUMP_FRAME_DURATION) {
            oled_write_raw_P(kirby_jump[min(kirby_current_jump_frame, (KIRBY_JUMP_FRAMES - 1))], DEFAULT_ANIM_SIZE);
            kirby_current_jump_frame = kirby_current_jump_frame + 1;
            anim_timer               = timer_read32();
            if (kirby_current_jump_frame > KIRBY_JUMP_FRAMES - 1) {
                showed_jump = true;
            }
        }
    } else {
        if (led_state.caps_lock && kirby_caps_state == 0) {
            // Go into caps mode
            kirby_current_inhale_frame = 0;
            kirby_caps_state           = 1;
        } else if (!led_state.caps_lock && kirby_caps_state == 2) {
            // Exit caps mode
            kirby_current_exhale_frame = 0;
            kirby_caps_state           = 3;
        } else if (space_pressed && showed_jump && !led_state.caps_lock) {
            kirby_current_jump_frame = 0;
            showed_jump              = false;
        } else if (mod_state & MOD_MASK_CTRL && !led_state.caps_lock) {
            // 2 = crouched state in jump anim
            oled_write_raw_P(kirby_jump[2], DEFAULT_ANIM_SIZE);
        } else {
            if ((wpm() <= KIRBY_MIN_WALK_SPEED && timer_elapsed32(anim_timer) > KIRBY_IDLE_FRAME_DURATION) || (wpm() > KIRBY_MIN_WALK_SPEED && timer_elapsed32(anim_timer) > (led_state.caps_lock ? get_frame_duration() * 2 : get_frame_duration())) || (kirby_current_jump_frame > 0 && timer_elapsed32(anim_timer) > KIRBY_JUMP_FRAME_DURATION) || (kirby_current_inhale_frame > 0 && timer_elapsed32(anim_timer) > KIRBY_INHALE_FRAME_DURATION) || (kirby_current_exhale_frame > 0 && timer_elapsed32(anim_timer) > KIRBY_EXHALE_FRAME_DURATION)) {
                animate_kirby();
                anim_timer                 = timer_read32();
                kirby_current_jump_frame   = 0;
                kirby_current_inhale_frame = 0;
                kirby_current_exhale_frame = 0;
            }
        }
    }

    draw_pet_text_section();
}

void draw_settings() {
    oled_set_cursor(0, 0);
    oled_write_P(PSTR("SETTINGS"), false);
    oled_set_cursor(12, 0);
    oled_write_P(PSTR("atude v13"), false);
    oled_set_cursor(0, 2);
    if (time_config_idx >= 0 && time_config_idx < 2) {
        oled_write(get_time(), false);
        oled_set_cursor(17, 2);
        oled_write_P(PSTR("TIME"), false);
    } else if (time_config_idx >= 2 && time_config_idx < 5) {
        oled_write(get_date(true), false);
        oled_set_cursor(17, 2);
        oled_write_P(PSTR("DATE"), false);
    } else if (time_config_idx >= 5 && time_config_idx < 6) {
        oled_write(get_date_time_format_str(), false);
        oled_set_cursor(15, 2);
        oled_write_P(PSTR("FORMAT"), false);
    } else {
        oled_write(get_timeout_str(), false);
        oled_set_cursor(14, 2);
        oled_write_P(PSTR("TIMEOUT"), false);
    }

    if (clock_set_mode) {
        switch (time_config_idx) {
            case 0:  // hour
            default:
                draw_line_h(0, 24, 10);
                break;
            case 1:  // minute
                draw_line_h(18, 24, 10);
                break;
            case 2:  // year
                draw_line_h(36, 24, 24);
                break;
            case 3:  // month or day (depending on date_time_format_mode)
                if (!is_month_first_date()) {
                    draw_line_h(18, 24, 10);
                } else {
                    draw_line_h(0, 24, 10);
                }
                break;
            case 4:  // day or month (depending on date_time_format_mode)
                if (!is_month_first_date()) {
                    draw_line_h(0, 24, 10);
                } else {
                    draw_line_h(18, 24, 10);
                }
                break;
            case 5:  // date time format mode, timeout mode
            case 6:
                break;
        }
    }
}

#endif
