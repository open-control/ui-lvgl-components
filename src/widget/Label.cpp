#include <oc/ui/lvgl/widget/Label.hpp>

#include <cstring>
#include <utility>

#include <config/PlatformCompat.hpp>
#include <oc/type/TextFormat.hpp>

namespace oc::ui::lvgl {

Label::Label(lv_obj_t* parent) {
    createWidgets(parent);
}

Label::~Label() {
    cleanup();
}

Label::Label(Label&& other) noexcept {
    *this = std::move(other);
}

Label& Label::operator=(Label&& other) noexcept {
    if (this == &other) return *this;
    cleanup();
    container_ = std::exchange(other.container_, nullptr);
    label_ = std::exchange(other.label_, nullptr);
    auto_scroll_enabled_ = other.auto_scroll_enabled_;
    owns_lvgl_objects_ = other.owns_lvgl_objects_;
    overflow_amount_ = other.overflow_amount_;
    alignment_ = other.alignment_;
    if (auto* animation = lv_anim_get(&other, scrollAnimCallback)) {
        lv_anim_set_var(animation, this);
    }
    bindEvents(&other);
    return *this;
}

FLASHMEM void Label::createWidgets(lv_obj_t* parent) {
    container_ = lv_obj_create(parent);
    lv_obj_set_size(container_, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(container_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container_, 0, 0);
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_clear_flag(container_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(container_, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_add_flag(container_, LV_OBJ_FLAG_EVENT_BUBBLE);

    label_ = lv_label_create(container_);
    lv_label_set_text(label_, "");
    lv_obj_set_width(label_, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(label_, 0, 0);
    lv_label_set_long_mode(label_, LV_LABEL_LONG_CLIP);
    lv_obj_add_flag(label_, LV_OBJ_FLAG_EVENT_BUBBLE);
    bindEvents();
}

void Label::bindEvents(Label* previousOwner) {
    for (auto* object : {container_, label_}) {
        if (!object) continue;
        if (previousOwner) lv_obj_remove_event_cb_with_user_data(object, geometryCallback, previousOwner);
        lv_obj_add_event_cb(object, geometryCallback, LV_EVENT_SIZE_CHANGED, this);
    }
    if (container_) lv_obj_add_event_cb(container_, geometryCallback, LV_EVENT_DELETE, this);
}

void Label::cleanup() {
    stopScrollAnimation();
    for (auto* object : {container_, label_}) {
        if (object) lv_obj_remove_event_cb_with_user_data(object, geometryCallback, this);
    }
    if (container_ && owns_lvgl_objects_) lv_obj_delete(container_);
    container_ = nullptr;
    label_ = nullptr;
}

Label& Label::autoScroll(bool enabled) {
    if (auto_scroll_enabled_ == enabled) return *this;
    auto_scroll_enabled_ = enabled;
    stopScrollAnimation();
    checkOverflowAndScroll();
    return *this;
}

Label& Label::alignment(lv_text_align_t align) {
    if (alignment_ == align) return *this;
    alignment_ = align;
    checkOverflowAndScroll();
    return *this;
}

Label& Label::flexGrow(bool enabled) {
    if (container_) {
        lv_obj_set_flex_grow(container_, enabled ? 1 : 0);
        lv_obj_set_width(container_, enabled ? 0 : LV_SIZE_CONTENT);
    }
    return *this;
}

Label& Label::color(uint32_t c) {
    if (label_) lv_obj_set_style_text_color(label_, lv_color_hex(c), 0);
    return *this;
}

Label& Label::font(const lv_font_t* f) {
    if (label_ && f) {
        lv_obj_set_style_text_font(label_, f, 0);
        checkOverflowAndScroll();
    }
    return *this;
}

Label& Label::width(lv_coord_t w) {
    if (container_) {
        lv_obj_set_flex_grow(container_, 0);
        lv_obj_set_width(container_, w);
    }
    return *this;
}

Label& Label::ownsLvglObjects(bool owns) {
    owns_lvgl_objects_ = owns;
    return *this;
}

Label& Label::gridCell(uint8_t col, uint8_t colSpan, uint8_t row, uint8_t rowSpan,
                       lv_grid_align_t vAlign) {
    if (container_) {
        lv_obj_set_grid_cell(container_, LV_GRID_ALIGN_STRETCH, col, colSpan, vAlign, row, rowSpan);
    }
    return *this;
}

void Label::setText(const std::string& text) {
    setText(text.c_str());
}

void Label::setText(int value, const char* prefix, const char* suffix) {
    char text[32];
    size_t pos = oc::type::text::appendString(text, sizeof(text), 0, prefix ? prefix : "");
    pos = oc::type::text::appendSigned(text, sizeof(text), pos, value);
    pos = oc::type::text::appendString(text, sizeof(text), pos, suffix ? suffix : "");
    oc::type::text::terminate(text, sizeof(text), pos);
    setText(text);
}

void Label::setText(float value, uint8_t decimals, const char* prefix, const char* suffix) {
    char numeric[24];
    oc::type::text::formatFixed(numeric, sizeof(numeric), value, decimals);
    char text[40];
    size_t pos = oc::type::text::appendString(text, sizeof(text), 0, prefix ? prefix : "");
    pos = oc::type::text::appendString(text, sizeof(text), pos, numeric);
    pos = oc::type::text::appendString(text, sizeof(text), pos, suffix ? suffix : "");
    oc::type::text::terminate(text, sizeof(text), pos);
    setText(text);
}

void Label::setText(const char* text) {
    if (!label_) return;
    // LVGL owns the only text copy. Repeated values must not restart scrolling.
    if (text && std::strcmp(lv_label_get_text(label_), text) == 0) return;
    stopScrollAnimation();
    lv_label_set_text(label_, text);
    checkOverflowAndScroll();
}

void Label::checkOverflowAndScroll() {
    if (!label_ || !container_) return;
    const lv_coord_t width = lv_obj_get_content_width(container_);
    // LVGL's intrinsic size cache measures the label, not the entire screen.
    // Parent/label size events settle alignment naturally in the normal layout.
    const lv_coord_t textWidth = lv_obj_get_self_width(label_);
    const lv_coord_t overflow = textWidth - width;
    if (overflow != overflow_amount_) stopScrollAnimation();
    overflow_amount_ = overflow;
    if (width <= 0) return;

    if (overflow > 0) {
        if (!lv_anim_get(this, scrollAnimCallback)) lv_obj_set_x(label_, 0);
        if (auto_scroll_enabled_) startScrollAnimation();
    } else {
        lv_coord_t offset = 0;
        if (alignment_ == LV_TEXT_ALIGN_CENTER) offset = (width - textWidth) / 2;
        else if (alignment_ == LV_TEXT_ALIGN_RIGHT) offset = width - textWidth;
        lv_obj_set_x(label_, offset);
    }
}

void Label::startScrollAnimation() {
    if (!label_ || overflow_amount_ <= 0 || lv_anim_get(this, scrollAnimCallback)) return;
    // One native animation owns forward, pause and return; no orphan pause timer.
    lv_anim_t animation;
    lv_anim_init(&animation);
    lv_anim_set_var(&animation, this);
    lv_anim_set_exec_cb(&animation, scrollAnimCallback);
    lv_anim_set_values(&animation, 0, -overflow_amount_);
    lv_anim_set_duration(&animation, 2000);
    lv_anim_set_delay(&animation, base_theme::animation::SCROLL_START_DELAY_MS);
    lv_anim_set_reverse_duration(&animation, 2000);
    lv_anim_set_reverse_delay(&animation, 1000);
    lv_anim_set_path_cb(&animation, lv_anim_path_ease_in_out);
    lv_anim_start(&animation);
}

void Label::stopScrollAnimation() {
    lv_anim_delete(this, scrollAnimCallback);
}

void Label::scrollAnimCallback(void* var, int32_t value) {
    auto* self = static_cast<Label*>(var);
    if (self->label_) lv_obj_set_x(self->label_, value);
}

void Label::geometryCallback(lv_event_t* event) {
    auto* self = static_cast<Label*>(lv_event_get_user_data(event));
    if (lv_event_get_code(event) == LV_EVENT_DELETE) {
        self->stopScrollAnimation();
        self->container_ = nullptr;
        self->label_ = nullptr;
    } else {
        self->checkOverflowAndScroll();
    }
}

}  // namespace oc::ui::lvgl
