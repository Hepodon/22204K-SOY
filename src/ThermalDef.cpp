#include "Custom/Thermal.hpp"
#include "pros/apix.h"
#include "pros/motors.hpp"
#include "pros/rtos.hpp"
#include <array>
#include <cmath>
#include <string>

constexpr int PORT_M1 = 1;
constexpr int PORT_M2 = 2;
constexpr int PORT_M3 = 3;
constexpr int PORT_M4 = 4;

constexpr double LVL2_C = 45.0;
constexpr double LVL3_C = 55.0;
constexpr double LVL4_C = 65.0;
constexpr double ARC_MIN_K = 293.15;
constexpr double ARC_MAX_K = 353.15;

constexpr uint32_t UI_UPDATE_MS = 250;

struct MotorUI {
  pros::Motor motor;
  lv_obj_t *arc;
  lv_obj_t *label_title;
  lv_obj_t *label_temp;
  lv_obj_t *label_level;
};

static std::array<MotorUI, 4> g_ui = {
    MotorUI{pros::Motor(PORT_M1), nullptr, nullptr, nullptr, nullptr},
    MotorUI{pros::Motor(PORT_M2), nullptr, nullptr, nullptr, nullptr},
    MotorUI{pros::Motor(PORT_M3), nullptr, nullptr, nullptr, nullptr},
    MotorUI{pros::Motor(PORT_M4), nullptr, nullptr, nullptr, nullptr}};

static lv_obj_t *g_screen = nullptr;

static double clamp(double v, double lo, double hi) {
  return std::fmax(lo, std::fmin(v, hi));
}

static double c_to_k(double c) { return c + 273.15; }

static int temp_to_level(double c) {
  if (c < LVL2_C)
    return 1;
  if (c < LVL3_C)
    return 2;
  if (c < LVL4_C)
    return 3;
  return 4;
}

static lv_color_t level_color(int lvl) {
  switch (lvl) {
  case 1:
    return lv_color_hex(0x2ecc71); // green
  case 2:
    return lv_color_hex(0xf1c40f); // yellow
  case 3:
    return lv_color_hex(0xe67e22); // orange
  default:
    return lv_color_hex(0xe74c3c); // red
  }
}

static void create_motor_slot(MotorUI &slot, const char *title,
                              lv_align_t align, int x_ofs, int y_ofs) {
  lv_obj_t *cont = lv_obj_create(g_screen);
  lv_obj_set_size(cont, 180, 140);
  lv_obj_set_style_pad_all(cont, 6, 0);
  lv_obj_set_style_radius(cont, 16, 0);
  lv_obj_set_style_bg_opa(cont, LV_OPA_20, 0);
  lv_obj_set_style_bg_color(cont, lv_color_hex(0x1F2937), 0);
  lv_obj_align(cont, align, x_ofs, y_ofs);

  // Title
  slot.label_title = lv_label_create(cont);
  lv_label_set_text(slot.label_title, title);
  lv_obj_align(slot.label_title, LV_ALIGN_TOP_LEFT, 4, 2);

  // 180° Arc
  slot.arc = lv_arc_create(cont);
  lv_obj_set_size(slot.arc, 120, 120);
  lv_obj_align(slot.arc, LV_ALIGN_TOP_RIGHT, -8, 8);
  lv_arc_set_rotation(slot.arc, 180);     // Rotate so half arc sits nicely
  lv_arc_set_bg_angles(slot.arc, 0, 180); // background half
  lv_arc_set_range(slot.arc, (int)ARC_MIN_K, (int)ARC_MAX_K);
  lv_arc_set_value(slot.arc, (int)ARC_MIN_K);
  lv_obj_remove_flag(slot.arc, LV_OBJ_FLAG_CLICKABLE); // display only

  // Style the indicator
  lv_obj_set_style_arc_width(slot.arc, 14, LV_PART_INDICATOR);
  lv_obj_set_style_arc_width(slot.arc, 14, LV_PART_MAIN);
  lv_obj_set_style_arc_color(slot.arc, lv_color_hex(0x374151),
                             LV_PART_MAIN); // muted bg

  // Kelvin readout (big)
  slot.label_temp = lv_label_create(cont);
  lv_obj_set_style_text_font(slot.label_temp, &lv_font_montserrat_20, 0);
  lv_label_set_text(slot.label_temp, "--.- K");
  lv_obj_align(slot.label_temp, LV_ALIGN_BOTTOM_LEFT, 6, -26);

  // Level label
  slot.label_level = lv_label_create(cont);
  lv_label_set_text(slot.label_level, "Level: -/4");
  lv_obj_align(slot.label_level, LV_ALIGN_BOTTOM_LEFT, 6, -4);
}

/** Generate the text "Mx (Port N)" */
static std::string make_title(const char *name, int port) {
  return std::string(name) + " (Port " + std::to_string(std::abs(port)) + ")";
}

/** Timer callback: poll motors and refresh UI */
static void ui_timer_cb(lv_timer_t * /*t*/) {
  for (size_t i = 0; i < g_ui.size(); ++i) {
    auto &m = g_ui[i];

    // If a motor is physically missing or disconnected, PROS returns NAN
    // sometimes; handle gracefully.
    double celsius = m.motor.get_temperature(); // °C (double)
    bool ok = std::isfinite(celsius);
    double kelvin = ok ? c_to_k(celsius) : NAN;

    // Compute level and arc color
    int lvl = ok ? temp_to_level(celsius) : 0;
    lv_color_t col = level_color(
        lvl == 0 ? 4 : lvl); // if unknown, show red to draw attention

    // Update arc value within range
    double k_for_arc = ok ? clamp(kelvin, ARC_MIN_K, ARC_MAX_K) : ARC_MIN_K;
    lv_arc_set_value(m.arc, static_cast<int>(std::round(k_for_arc)));
    lv_obj_set_style_arc_color(m.arc, col, LV_PART_INDICATOR);

    // Update labels
    if (ok) {
      char buf[48];
      std::snprintf(buf, sizeof(buf), "%.1f K", kelvin);
      lv_label_set_text(m.label_temp, buf);

      char lvlbuf[24];
      std::snprintf(lvlbuf, sizeof(lvlbuf), "Level: %d/4", lvl);
      lv_label_set_text(m.label_level, lvlbuf);

      lv_obj_set_style_text_color(m.label_level, col, 0);
    } else {
      lv_label_set_text(m.label_temp, "No data");
      lv_label_set_text(m.label_level, "Level: -/4");
      lv_obj_set_style_text_color(m.label_level, lv_color_hex(0xe74c3c), 0);
    }
  }
}

/** Build the whole screen UI */
static void build_ui() {
  g_screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(g_screen, lv_color_hex(0x0B1220), 0);
  lv_obj_set_style_bg_opa(g_screen, LV_OPA_COVER, 0);

  // Create the 4 corners
  create_motor_slot(g_ui[0], make_title("M1", PORT_M1).c_str(),
                    LV_ALIGN_TOP_LEFT, 6, 6);
  create_motor_slot(g_ui[1], make_title("M2", PORT_M2).c_str(),
                    LV_ALIGN_TOP_RIGHT, -6, 6);
  create_motor_slot(g_ui[2], make_title("M3", PORT_M3).c_str(),
                    LV_ALIGN_BOTTOM_LEFT, 6, -6);
  create_motor_slot(g_ui[3], make_title("M4", PORT_M4).c_str(),
                    LV_ALIGN_BOTTOM_RIGHT, -6, -6);

  lv_screen_load(g_screen);

  // Start periodic updates
  lv_timer_create(ui_timer_cb, UI_UPDATE_MS, nullptr);
}