# This file is part of the Avitab project. See the README and LICENSE for details.

project(LVGL
    VERSION 6.1.2
    DESCRIPTION "Light and Versatile Graphics Library"
    LANGUAGES C
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
find_package(Threads REQUIRED)
include(Prefixer)

set(lvgl_SOURCES
    "lv_core/lv_debug.c"
    "lv_core/lv_disp.c"
    "lv_core/lv_group.c"
    "lv_core/lv_indev.c"
    "lv_core/lv_obj.c"
    "lv_core/lv_refr.c"
    "lv_core/lv_style.c"
    "lv_draw/lv_draw.c"
    "lv_draw/lv_draw_arc.c"
    "lv_draw/lv_draw_basic.c"
    "lv_draw/lv_draw_img.c"
    "lv_draw/lv_draw_label.c"
    "lv_draw/lv_draw_line.c"
    "lv_draw/lv_draw_rect.c"
    "lv_draw/lv_draw_triangle.c"
    "lv_draw/lv_img_cache.c"
    "lv_draw/lv_img_decoder.c"
    "lv_font/lv_font.c"
    "lv_font/lv_font_fmt_txt.c"
    "lv_font/lv_font_roboto_12.c"
    "lv_font/lv_font_roboto_16.c"
    "lv_font/lv_font_roboto_22.c"
    "lv_font/lv_font_roboto_28.c"
    "lv_font/lv_font_unscii_8.c"
    "lv_hal/lv_hal_disp.c"
    "lv_hal/lv_hal_indev.c"
    "lv_hal/lv_hal_tick.c"
    "lv_misc/lv_anim.c"
    "lv_misc/lv_area.c"
    "lv_misc/lv_async.c"
    "lv_misc/lv_bidi.c"
    "lv_misc/lv_circ.c"
    "lv_misc/lv_color.c"
    "lv_misc/lv_fs.c"
    "lv_misc/lv_gc.c"
    "lv_misc/lv_ll.c"
    "lv_misc/lv_log.c"
    "lv_misc/lv_math.c"
    "lv_misc/lv_mem.c"
    "lv_misc/lv_printf.c"
    "lv_misc/lv_task.c"
    "lv_misc/lv_templ.c"
    "lv_misc/lv_txt.c"
    "lv_misc/lv_utils.c"
    "lv_objx/lv_arc.c"
    "lv_objx/lv_bar.c"
    "lv_objx/lv_btn.c"
    "lv_objx/lv_btnm.c"
    "lv_objx/lv_calendar.c"
    "lv_objx/lv_canvas.c"
    "lv_objx/lv_cb.c"
    "lv_objx/lv_chart.c"
    "lv_objx/lv_cont.c"
    "lv_objx/lv_cpicker.c"
    "lv_objx/lv_ddlist.c"
    "lv_objx/lv_gauge.c"
    "lv_objx/lv_img.c"
    "lv_objx/lv_imgbtn.c"
    "lv_objx/lv_kb.c"
    "lv_objx/lv_label.c"
    "lv_objx/lv_led.c"
    "lv_objx/lv_line.c"
    "lv_objx/lv_list.c"
    "lv_objx/lv_lmeter.c"
    "lv_objx/lv_mbox.c"
    "lv_objx/lv_objx_templ.c"
    "lv_objx/lv_page.c"
    "lv_objx/lv_preload.c"
    "lv_objx/lv_roller.c"
    "lv_objx/lv_slider.c"
    "lv_objx/lv_spinbox.c"
    "lv_objx/lv_sw.c"
    "lv_objx/lv_ta.c"
    "lv_objx/lv_table.c"
    "lv_objx/lv_tabview.c"
    "lv_objx/lv_tileview.c"
    "lv_objx/lv_win.c"
    "lv_themes/lv_theme.c"
    "lv_themes/lv_theme_alien.c"
    "lv_themes/lv_theme_default.c"
    "lv_themes/lv_theme_material.c"
    "lv_themes/lv_theme_mono.c"
    "lv_themes/lv_theme_nemo.c"
    "lv_themes/lv_theme_night.c"
    "lv_themes/lv_theme_templ.c"
    "lv_themes/lv_theme_zen.c"
)
PrefixSources(lvgl_SOURCES "${AVITAB_lvgl_SOURCE_DIR}/lvgl/src")

add_library(lvgl STATIC
    ${lvgl_SOURCES}
    "${CMAKE_CURRENT_LIST_DIR}/lv_font_roboto_20.c"
)

target_include_directories(lvgl
    PRIVATE "${AVITAB_lvgl_SOURCE_DIR}/lvgl"
)

# Cache settings to be used when BUILD_AVITAB_THIRDPARTY is OFF
set(AVITAB_lvgl_PKGLIBS lvgl CACHE INTERNAL "")
if(MSVC)
    set(AVITAB_lvgl_LOCATION "${AVITAB_LIBS3RD_DIR}/lvgl.lib" CACHE INTERNAL "")
else()
    set(AVITAB_lvgl_LOCATION "${AVITAB_LIBS3RD_DIR}/liblvgl.a" CACHE INTERNAL "")
endif()
set(AVITAB_lvgl_INCDIRS "${AVITAB_lvgl_SOURCE_DIR}/lvgl" CACHE INTERNAL "")
