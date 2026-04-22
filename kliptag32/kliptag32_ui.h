#pragma once

#include "esphome.h"

using namespace esphome;

namespace kliptag32_ui {

using namespace display;

constexpr int SCREEN_W = 128;
constexpr int HEADER_BOTTOM_Y = 12;
constexpr int LIST_FIRST_ROW_Y = 14;
constexpr int LIST_ROW_HEIGHT = 10;

inline void draw_progress_bar(Display &it, int x, int y, int w, int h, int pct) {
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  it.rectangle(x, y, w, h);
  int fill_w = (w - 2) * pct / 100;
  if (fill_w > 0) it.filled_rectangle(x + 1, y + 1, fill_w, h - 2);
}

inline void draw_list_item(Display &it, int row, bool selected, const char *text) {
  int y = LIST_FIRST_ROW_Y + row * LIST_ROW_HEIGHT;
  if (selected) {
    it.filled_rectangle(0, y, SCREEN_W, LIST_ROW_HEIGHT);
    it.printf(2, y - 1, font_sm, COLOR_OFF, "%s", text);
  } else {
    it.printf(2, y - 1, font_sm, "%s", text);
  }
}

inline void paginate(int count, int visible) {
  if (count <= 0) {
    menu_index->value() = 0;
    menu_scroll->value() = 0;
    return;
  }
  int idx = ((menu_index->value() % count) + count) % count;
  menu_index->value() = idx;
  int scroll = menu_scroll->value();
  if (idx < scroll) scroll = idx;
  if (idx >= scroll + visible) scroll = idx - visible + 1;
  if (scroll < 0) scroll = 0;
  if (scroll > count - visible && count > visible) scroll = count - visible;
  if (count <= visible) scroll = 0;
  menu_scroll->value() = scroll;
}

inline void draw_home(Display &it) {
  it.printf(0, 0, font_sm, "KlipTag32");
  char rhs[24];
  snprintf(rhs, sizeof(rhs), "M:%s S:%s",
           moonraker_connected->value() ? "ok" : "--",
           spoolman_connected->value() ? "ok" : "--");
  it.printf(SCREEN_W, 0, font_sm, TextAlign::TOP_RIGHT, "%s", rhs);
  it.line(0, HEADER_BOTTOM_Y, SCREEN_W, HEADER_BOTTOM_Y);
  static const char *items[] = {"Active Spool", "Write NFC", "Spool Info",
                                "Status", "Actions", "Scale", "System Health"};
  const int count = static_cast<int>(sizeof(items) / sizeof(items[0]));
  const int visible = 5;
  paginate(count, visible);
  int scroll = menu_scroll->value();
  for (int i = 0; i < visible && (scroll + i) < count; i++) {
    draw_list_item(it, i, (scroll + i) == menu_index->value(), items[scroll + i]);
  }
}

inline void draw_spool_list(Display &it) {
  const auto &opts = spool_select->traits.get_options();
  int count = (int) opts.size();
  it.printf(0, 0, font_sm, "Active Spool (%d)", count);
  it.line(0, HEADER_BOTTOM_Y, SCREEN_W, HEADER_BOTTOM_Y);
  const int visible = 5;
  paginate(count, visible);
  int scroll = menu_scroll->value();
  for (int i = 0; i < visible && (scroll + i) < count; i++) {
    draw_list_item(it, i, (scroll + i) == menu_index->value(), opts[scroll + i]);
  }
  if (count == 0) it.printf(2, LIST_FIRST_ROW_Y, font_sm, "loading...");
}

inline void draw_write_pick(Display &it) {
  const auto &opts = spoolman_spools_select->traits.get_options();
  int count = (int) opts.size();
  it.printf(0, 0, font_sm, "Write NFC: pick");
  it.line(0, HEADER_BOTTOM_Y, SCREEN_W, HEADER_BOTTOM_Y);
  const int visible = 5;
  paginate(count, visible);
  int scroll = menu_scroll->value();
  for (int i = 0; i < visible && (scroll + i) < count; i++) {
    draw_list_item(it, i, (scroll + i) == menu_index->value(), opts[scroll + i]);
  }
  if (count == 0) it.printf(2, LIST_FIRST_ROW_Y, font_sm, "loading...");
}

inline void draw_status(Display &it) {
  it.printf(0, 0, font_sm, "Status");
  it.line(0, HEADER_BOTTOM_Y, SCREEN_W, HEADER_BOTTOM_Y);
  it.printf(0, 14, font_sm, "Moonraker: %s",
            moonraker_connected->value() ? "connected" : "offline");
  it.printf(0, 24, font_sm, "Spoolman:  %s",
            spoolman_connected->value() ? "connected" : "offline");
  std::string cs = current_spool_text->state;
  if (cs.empty()) cs = "(none)";
  it.printf(0, 36, font_sm, "Spool:");
  it.printf(0, 48, font_sm, "%s", cs.c_str());
}

inline void draw_actions(Display &it) {
  it.printf(0, 0, font_sm, "Actions");
  it.line(0, HEADER_BOTTOM_Y, SCREEN_W, HEADER_BOTTOM_Y);
  static const char *items[] = {"Refresh Moonraker", "Refresh Spoolman",
                                "Calibrate Scale", "Restart Device"};
  const int count = static_cast<int>(sizeof(items) / sizeof(items[0]));
  paginate(count, count);
  for (int i = 0; i < count; i++) {
    draw_list_item(it, i, i == menu_index->value(), items[i]);
  }
}

inline void draw_write_armed(Display &it) {
  it.printf(0, 0, font_sm, "Write NFC: armed");
  it.line(0, HEADER_BOTTOM_Y, SCREEN_W, HEADER_BOTTOM_Y);
  std::string s = nfc_write_status_text->state;
  if (s.empty()) s = "preparing...";
  it.printf(0, 16, font_sm, "Status:");
  it.printf(0, 28, font_sm, "%s", s.c_str());
  it.printf(0, 44, font_sm, "Spool id: %d", nfc_write_spool_id->value());
  it.printf(0, 54, font_sm, "Present tag to reader");
}

inline void draw_filament_list(Display &it) {
  const auto &all_opts = spool_select->traits.get_options();
  const std::string &flt = filament_filter_material->value();
  std::vector<std::string> opts;
  opts.reserve(all_opts.size());
  for (const char *cstr : all_opts) {
    std::string o = cstr;
    if (flt.empty()) { opts.push_back(std::move(o)); continue; }
    size_t p1 = o.find(" | ");
    if (p1 == std::string::npos) continue;
    size_t p2 = o.find(" | ", p1 + 3);
    if (p2 == std::string::npos) continue;
    if (o.substr(p1 + 3, p2 - p1 - 3) == flt) opts.push_back(std::move(o));
  }
  int count = (int) opts.size();
  const char *flt_label = flt.empty() ? "All" : flt.c_str();
  it.printf(0, 0, font_sm, "Filament [%s] (%d)", flt_label, count);
  it.line(0, HEADER_BOTTOM_Y, SCREEN_W, HEADER_BOTTOM_Y);
  const int visible = 5;
  paginate(count, visible);
  int scroll = menu_scroll->value();
  for (int i = 0; i < visible && (scroll + i) < count; i++) {
    draw_list_item(it, i, (scroll + i) == menu_index->value(), opts[scroll + i].c_str());
  }
  if (count == 0) {
    it.printf(2, 14, font_sm, flt.empty() ? "loading..." : "no spools match");
    it.printf(2, 26, font_sm, "press Home to cycle");
  }
}

inline void draw_filament_detail(Display &it) {
  it.printf(SCREEN_W / 2, 0, font_md, TextAlign::TOP_CENTER, "%s",
            viewing_filament_line_1->state.c_str());
  it.line(0, 16, SCREEN_W, 16);
  it.printf(SCREEN_W / 2, 18, font_sm, TextAlign::TOP_CENTER, "%s",
            viewing_filament_line_2->state.c_str());

  const int pct = viewing_remaining_pct->value();
  draw_progress_bar(it, 4, 30, 88, 10, pct);
  it.printf(96, 30, font_sm, TextAlign::TOP_LEFT, "%d%%", pct);

  it.printf(SCREEN_W / 2, 42, font_sm, TextAlign::TOP_CENTER, "%s",
            viewing_filament_line_3->state.c_str());
  it.printf(SCREEN_W / 2, 52, font_sm, TextAlign::TOP_CENTER, "%s",
            viewing_filament_line_4->state.c_str());
}

inline void draw_scale(Display &it) {
  it.printf(0, 0, font_sm, "Scale");
  it.printf(SCREEN_W, 0, font_sm, TextAlign::TOP_RIGHT, "click=tare");
  if (scale_raw->has_state()) {
    it.printf(SCREEN_W / 2, 36, font_lg, TextAlign::CENTER, "%.1f g", scale_weight->state);
  } else {
    it.printf(SCREEN_W / 2, 36, font_sm, TextAlign::CENTER, "warming up...");
  }
}

inline void draw_scale_calib(Display &it) {
  it.printf(0, 0, font_sm, "Calibrate Scale");
  it.line(0, HEADER_BOTTOM_Y, SCREEN_W, HEADER_BOTTOM_Y);
  const int step = scale_cal_step->value();
  const float raw = scale_raw->has_state() ? scale_raw->state : 0.0f;

  if (step == 0) {
    it.printf(0, 16, font_sm, "1. Empty the scale");
    it.printf(0, 28, font_sm, "Raw: %.0f", raw);
    it.printf(0, 44, font_sm, "Click: capture zero");
  } else if (step == 1) {
    it.printf(0, 14, font_sm, "2. Place known mass");
    it.printf(0, 26, font_sm, "Mass: %d g", scale_cal_known_mass->value());
    it.printf(0, 38, font_sm, "Raw - tare: %.0f",
              raw - scale_tare_offset->value());
    it.printf(0, 52, font_sm, "Turn=adj  Click=ok");
  } else {
    it.printf(0, 16, font_sm, "Done");
    it.printf(0, 28, font_sm, "%.2f cnt/g", scale_calibration->value());
    it.printf(0, 44, font_sm, "Click to exit");
  }
}

inline void draw_system_health(Display &it, const char *project_version) {
  it.printf(0, 0, font_sm, "System Health");
  it.line(0, HEADER_BOTTOM_Y, SCREEN_W, HEADER_BOTTOM_Y);

  if (wifi_rssi->has_state())
    it.printf(0, 14, font_sm, "WiFi: %.0f dBm", wifi_rssi->state);
  else
    it.printf(0, 14, font_sm, "WiFi: --");

  uint32_t s = uptime_seconds->has_state() ? (uint32_t) uptime_seconds->state : 0;
  uint32_t d = s / 86400; s %= 86400;
  uint32_t h = s / 3600;  s %= 3600;
  uint32_t m = s / 60;    s %= 60;
  if (d > 0)
    it.printf(0, 24, font_sm, "Up: %ud %uh %um", d, h, m);
  else if (h > 0)
    it.printf(0, 24, font_sm, "Up: %uh %um %us", h, m, s);
  else
    it.printf(0, 24, font_sm, "Up: %um %us", m, s);

  std::string ip = wifi_info_ip->state;
  if (ip.empty()) ip = "(no link)";
  it.printf(0, 34, font_sm, "IP: %s", ip.c_str());

  it.printf(0, 44, font_sm, "Ver: %s", project_version);
  it.printf(0, 54, font_sm, "click to return");
}

inline bool draw_error_flash(Display &it) {
  if (error_flash_until_ms->value() == 0) return false;
  if (millis() >= error_flash_until_ms->value()) return false;
  it.printf(SCREEN_W / 2, 4, font_md, TextAlign::TOP_CENTER, "ERROR");
  it.line(0, 22, SCREEN_W, 22);
  std::string err = change_spool_error_text->state;
  if (err.empty()) err = "Spool change blocked";
  it.printf(0, 26, font_sm, "%s", err.c_str());
  it.printf(0, 50, font_sm, "Printer state: %s", printer_state->value().c_str());
  return true;
}

inline bool draw_nfc_flash(Display &it) {
  if (flash_until_ms->value() == 0) return false;
  if (millis() >= flash_until_ms->value()) return false;
  it.printf(0, 2, font_sm, "%s", current_spool_text->state.c_str());
  std::string d = current_spool_details_text->state;
  // current_spool_details_text uses "   |   " as separator
  size_t p1 = d.find("   |   ");
  size_t p2 = (p1 != std::string::npos) ? d.find("   |   ", p1 + 7) : std::string::npos;
  if (p2 != std::string::npos) {
    it.printf(0, 18, font_sm, "%s", d.substr(0, p2).c_str());
    it.printf(0, 30, font_sm, "%s", d.substr(p2 + 7).c_str());
  } else if (p1 != std::string::npos) {
    it.printf(0, 18, font_sm, "%s", d.substr(0, p1).c_str());
    it.printf(0, 30, font_sm, "%s", d.substr(p1 + 7).c_str());
  } else {
    it.printf(0, 18, font_sm, "%s", d.c_str());
  }
  it.printf(0, 50, font_sm, "SPOOL:%d FILAMENT:%d",
            current_spool_id->value(), current_filament_id->value());
  return true;
}

inline void update_scale_poller(int page) {
  static int last_page = -1;
  if (page == last_page) return;
  const bool now_on   = (page == 8 || page == 10);
  const bool was_on   = (last_page == 8 || last_page == 10);
  if (now_on && !was_on) {
    scale_raw->start_poller();
    ESP_LOGI("scale", "HX711 poller started (entered SCALE/CALIB page)");
  } else if (was_on && !now_on) {
    scale_raw->stop_poller();
    ESP_LOGI("scale", "HX711 poller stopped (left SCALE/CALIB page)");
  }
  last_page = page;
}

inline void display_content(Display &it, int page, const char *project_version) {
  switch (page) {
    case 0: draw_home(it); break;
    case 1: draw_spool_list(it); break;
    case 2: draw_write_pick(it); break;
    case 3: draw_status(it); break;
    case 4: draw_actions(it); break;
    case 5: draw_write_armed(it); break;
    case 6: draw_filament_list(it); break;
    case 7: draw_filament_detail(it); break;
    case 8: draw_scale(it); break;
    case 9: draw_system_health(it, project_version); break;
    case 10: draw_scale_calib(it); break;
    default:
      menu_page->value() = 0;
      break;
  }
}

inline void display_menu(Display &it, const char *project_version) {
  const int page = menu_page->value();
  update_scale_poller(page);
  if (draw_error_flash(it)) return;
  if (draw_nfc_flash(it)) return;
  display_content(it, page, project_version);
}

}
