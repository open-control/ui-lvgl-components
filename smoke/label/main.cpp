#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <memory>
#include <utility>
#include <oc/ui/lvgl/widget/Label.hpp>

using oc::ui::lvgl::Label;

static unsigned timerCount() {
    unsigned count = 0;
    for (auto* timer = lv_timer_get_next(nullptr); timer; timer = lv_timer_get_next(timer)) ++count;
    return count;
}

static void advance(unsigned milliseconds) {
    for (unsigned elapsed = 0; elapsed < milliseconds; elapsed += 10) {
        lv_tick_inc(10);
        lv_timer_handler();
    }
}

int main(int argc, char** argv) {
    assert(argc == 2);
    lv_init();
    auto* display = lv_display_create(320, 240);
    std::array<uint16_t, 320 * 240> pixels{};
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(display, pixels.data(), nullptr, sizeof(pixels), LV_DISPLAY_RENDER_MODE_DIRECT);
    lv_display_set_flush_cb(display, [](lv_display_t* d, const lv_area_t*, uint8_t*) { lv_display_flush_ready(d); });
    auto* parent = lv_obj_create(lv_screen_active());
    lv_obj_remove_style_all(parent);
    lv_obj_set_size(parent, 160, 100);
    const unsigned idleTimers = timerCount();
    auto label = std::make_unique<Label>(parent);
    label->width(40).alignment(LV_TEXT_ALIGN_LEFT);
    constexpr auto longText = "A long lane name that must scroll";
    label->setText(longText);
    advance(1400);

    const char* test = argv[1];
    if (std::strcmp(test, "unchanged") == 0) {
        const auto x = lv_obj_get_x(label->getLabel());
        assert(x < 0);
        label->setText(longText);
        advance(10);
        assert(lv_obj_get_x(label->getLabel()) <= x); // No jump/restart on identical content.
        assert(timerCount() == idleTimers);
    } else if (std::strcmp(test, "pause") == 0) {
        advance(1600); // At the far end, waiting before the return leg.
        label.reset();
        assert(timerCount() == idleTimers); // No callback may outlive the widget.
        advance(3000);
    } else if (std::strcmp(test, "disable") == 0) {
        label->autoScroll(false);
        advance(50); // LVGL applies the new static position in its normal frame.
        const auto x = lv_obj_get_x(label->getLabel());
        assert(x == 0);
        advance(500);
        assert(lv_obj_get_x(label->getLabel()) == x);
        assert(lv_anim_count_running() == 0);
    } else if (std::strcmp(test, "move") == 0) {
        auto moved = std::make_unique<Label>(std::move(*label));
        label.reset();
        advance(500);
        assert(lv_obj_get_x(moved->getLabel()) < 0);
        moved->width(160);
        advance(100);
        moved.reset();
        assert(lv_anim_count_running() == 0);
        assert(timerCount() == idleTimers);
        advance(3000);
    } else if (std::strcmp(test, "borrowed") == 0) {
        label->ownsLvglObjects(false);
        auto* element = label->getElement();
        label.reset();
        assert(lv_obj_get_event_count(element) == 0);
        lv_obj_set_width(element, 80); // Resize must not call the destroyed wrapper.
        advance(100);
        assert(timerCount() == idleTimers);
    } else if (std::strcmp(test, "parent_first") == 0) {
        label->ownsLvglObjects(false);
        lv_obj_delete(parent);
        parent = nullptr;
        assert(label->getElement() == nullptr);
        assert(label->getLabel() == nullptr);
        label.reset();
        advance(3000);
    } else if (std::strcmp(test, "layout") == 0) {
        label->autoScroll(false).alignment(LV_TEXT_ALIGN_RIGHT).width(140);
        label->setText(127);
        lv_obj_update_layout(parent);
        advance(50);
        const auto numericX = lv_obj_get_x(label->getLabel());
        label->setText("127");
        lv_obj_update_layout(parent);
        advance(50);
        assert(lv_obj_get_x(label->getLabel()) == numericX);
        assert(numericX + lv_obj_get_width(label->getLabel()) == 140);
        label->setText(12.5f, 1);
        advance(50);
        assert(std::strcmp(lv_label_get_text(label->getLabel()), "12.5") == 0);
        assert(timerCount() == idleTimers);
    } else {
        assert(false);
    }
    label.reset();
    assert(timerCount() == idleTimers);
    lv_display_delete(display);
    lv_deinit();
    std::printf("label %s: passed\n", test);
}
