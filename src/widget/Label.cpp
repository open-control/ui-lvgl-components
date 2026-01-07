#include <oc/ui/lvgl/widget/Label.hpp>

namespace oc::ui::lvgl {

// =============================================================================
// Construction / Destruction
// =============================================================================

Label::Label(lv_obj_t* parent) {
    createWidgets(parent);
}

Label::~Label() {
    stopScrollAnimation();
    cleanup();
}

Label::Label(Label&& other) noexcept
    : container_(other.container_),
      label_(other.label_),
      pending_timer_(other.pending_timer_),
      auto_scroll_enabled_(other.auto_scroll_enabled_),
      anim_running_(other.anim_running_),
      owns_lvgl_objects_(other.owns_lvgl_objects_),
      overflow_amount_(other.overflow_amount_),
      alignment_(other.alignment_),
      scroll_duration_ms_(other.scroll_duration_ms_),
      pause_duration_ms_(other.pause_duration_ms_) {
    // Update timer's user_data to point to new object
    if (pending_timer_) {
        lv_timer_set_user_data(pending_timer_, this);
    }
    other.container_ = nullptr;
    other.label_ = nullptr;
    other.pending_timer_ = nullptr;
    other.anim_running_ = false;
}

Label& Label::operator=(Label&& other) noexcept {
    if (this != &other) {
        stopScrollAnimation();
        cleanup();

        container_ = other.container_;
        label_ = other.label_;
        pending_timer_ = other.pending_timer_;
        auto_scroll_enabled_ = other.auto_scroll_enabled_;
        anim_running_ = other.anim_running_;
        owns_lvgl_objects_ = other.owns_lvgl_objects_;
        overflow_amount_ = other.overflow_amount_;
        alignment_ = other.alignment_;
        scroll_duration_ms_ = other.scroll_duration_ms_;
        pause_duration_ms_ = other.pause_duration_ms_;

        // Update timer's user_data to point to new object
        if (pending_timer_) {
            lv_timer_set_user_data(pending_timer_, this);
        }

        other.container_ = nullptr;
        other.label_ = nullptr;
        other.pending_timer_ = nullptr;
        other.anim_running_ = false;
    }
    return *this;
}

void Label::createWidgets(lv_obj_t* parent) {
    // Container that clips overflow
    container_ = lv_obj_create(parent);
    // Default size: 100% width, content height
    // Works with flex layouts and grid rows using LV_GRID_CONTENT
    lv_obj_set_size(container_, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(container_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container_, 0, 0);
    lv_obj_set_style_pad_all(container_, 0, 0);
    lv_obj_clear_flag(container_, LV_OBJ_FLAG_SCROLLABLE);
    // Clip overflow for scroll animation
    lv_obj_clear_flag(container_, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    // Bubble events to parent for click handling
    lv_obj_add_flag(container_, LV_OBJ_FLAG_EVENT_BUBBLE);
    // Recalculate alignment on resize
    lv_obj_add_event_cb(container_, sizeChangedCallback, LV_EVENT_SIZE_CHANGED, this);

    // The actual label - full width, content height
    label_ = lv_label_create(container_);
    lv_label_set_text(label_, "");
    lv_obj_set_width(label_, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(label_, 0, 0);
    lv_label_set_long_mode(label_, LV_LABEL_LONG_CLIP);
    lv_obj_add_flag(label_, LV_OBJ_FLAG_EVENT_BUBBLE);
}

void Label::cleanup() {
    // Cancel any pending timer to prevent use-after-free
    if (pending_timer_) {
        lv_timer_delete(pending_timer_);
        pending_timer_ = nullptr;
    }
    if (container_ && owns_lvgl_objects_) {
        lv_obj_delete(container_);
    }
    container_ = nullptr;
    label_ = nullptr;
}

// =============================================================================
// Fluent Configuration
// =============================================================================

Label& Label::autoScroll(bool enabled) {
    auto_scroll_enabled_ = enabled;
    return *this;
}

Label& Label::alignment(lv_text_align_t align) {
    alignment_ = align;
    return *this;
}

Label& Label::flexGrow(bool enabled) {
    if (container_) {
        if (enabled) {
            lv_obj_set_width(container_, 0);
            lv_obj_set_flex_grow(container_, 1);
        } else {
            lv_obj_set_flex_grow(container_, 0);
            lv_obj_set_width(container_, LV_SIZE_CONTENT);
        }
    }
    return *this;
}

Label& Label::color(uint32_t c) {
    if (label_) {
        lv_obj_set_style_text_color(label_, lv_color_hex(c), 0);
    }
    return *this;
}

Label& Label::font(const lv_font_t* f) {
    if (label_ && f) {
        lv_obj_set_style_text_font(label_, f, 0);
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
        // Always STRETCH horizontally - LV_PCT(100) containers don't work with GRID_ALIGN_END
        // Use alignment() for text positioning (LEFT/CENTER/RIGHT)
        lv_obj_set_grid_cell(container_, LV_GRID_ALIGN_STRETCH, col, colSpan, vAlign, row, rowSpan);
    }
    return *this;
}

// =============================================================================
// Data Setters
// =============================================================================

void Label::setText(const std::string& text) {
    setText(text.c_str());
}

void Label::setText(int value, const char* prefix, const char* suffix) {
    if (!label_) return;
    stopScrollAnimation();
    lv_label_set_text_fmt(label_, "%s%d%s", prefix, value, suffix);
    if (auto_scroll_enabled_) {
        checkOverflowAndScroll();
    } else {
        applyStaticAlignment();
    }
}

void Label::setText(float value, uint8_t decimals, const char* prefix, const char* suffix) {
    if (!label_) return;
    stopScrollAnimation();
    char fmt[16];
    lv_snprintf(fmt, sizeof(fmt), "%%s%%.%uf%%s", decimals);
    lv_label_set_text_fmt(label_, fmt, prefix, value, suffix);
    if (auto_scroll_enabled_) {
        checkOverflowAndScroll();
    } else {
        applyStaticAlignment();
    }
}

void Label::setText(const char* text) {
    if (!label_) return;

    stopScrollAnimation();
    lv_label_set_text(label_, text);

    // Cancel any previous pending timer
    if (pending_timer_) {
        lv_timer_delete(pending_timer_);
        pending_timer_ = nullptr;
    }

    // Defer overflow check to next frame when layout is ready
    pending_timer_ = lv_timer_create([](lv_timer_t* t) {
        auto* self = static_cast<Label*>(lv_timer_get_user_data(t));
        if (self) {
            self->pending_timer_ = nullptr;  // Clear before callback
            self->checkOverflowAndScroll();
        }
    }, 0, this);
    lv_timer_set_repeat_count(pending_timer_, 1);
}

// =============================================================================
// Scroll Animation
// =============================================================================

void Label::checkOverflowAndScroll() {
    if (!label_ || !container_) return;

    // Update full layout hierarchy to ensure dimensions are computed
    lv_obj_t* parent = lv_obj_get_parent(container_);
    if (parent) {
        lv_obj_t* grandparent = lv_obj_get_parent(parent);
        if (grandparent) {
            lv_obj_update_layout(grandparent);
        } else {
            lv_obj_update_layout(parent);
        }
    }
    lv_obj_update_layout(container_);

    lv_coord_t container_width = lv_obj_get_width(container_);

    // Still no width? Schedule retry - layout not ready yet
    if (container_width <= 0) {
        if (!pending_timer_) {
            pending_timer_ = lv_timer_create([](lv_timer_t* t) {
                auto* self = static_cast<Label*>(lv_timer_get_user_data(t));
                if (self) {
                    self->pending_timer_ = nullptr;
                    self->checkOverflowAndScroll();
                }
            }, 10, this);  // Retry after 10ms
            lv_timer_set_repeat_count(pending_timer_, 1);
        }
        return;
    }

    // Measure text width
    lv_label_set_long_mode(label_, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label_, LV_SIZE_CONTENT);
    lv_obj_update_layout(label_);
    lv_coord_t text_width = lv_obj_get_width(label_);

    // Restore clip mode
    lv_label_set_long_mode(label_, LV_LABEL_LONG_CLIP);
    overflow_amount_ = text_width - container_width;

    if (overflow_amount_ > 0) {
        // Text overflows: align left and scroll
        lv_obj_set_x(label_, 0);
        if (auto_scroll_enabled_) {
            startScrollAnimation();
        }
    } else {
        // Text fits: apply alignment
        lv_coord_t offset = 0;
        switch (alignment_) {
            case LV_TEXT_ALIGN_CENTER:
                offset = (container_width - text_width) / 2;
                break;
            case LV_TEXT_ALIGN_RIGHT:
                offset = container_width - text_width;
                break;
            default:
                offset = 0;
                break;
        }
        lv_obj_set_x(label_, offset);
    }
}

void Label::startScrollAnimation() {
    if (!label_ || anim_running_ || overflow_amount_ <= 0) return;

    lv_anim_init(&scroll_anim_);
    lv_anim_set_var(&scroll_anim_, this);
    lv_anim_set_exec_cb(&scroll_anim_, scrollAnimCallback);
    lv_anim_set_values(&scroll_anim_, 0, -overflow_amount_);
    lv_anim_set_duration(&scroll_anim_, scroll_duration_ms_);
    lv_anim_set_delay(&scroll_anim_, base_theme::animation::SCROLL_START_DELAY_MS);
    lv_anim_set_path_cb(&scroll_anim_, lv_anim_path_ease_in_out);
    lv_anim_set_completed_cb(&scroll_anim_, [](lv_anim_t* a) {
        auto* self = static_cast<Label*>(a->var);
        lv_timer_t* timer = lv_timer_create(pauseTimerCallback, self->pause_duration_ms_, self);
        lv_timer_set_repeat_count(timer, 1);
    });

    lv_anim_start(&scroll_anim_);
    anim_running_ = true;
}

void Label::stopScrollAnimation() {
    if (anim_running_) {
        lv_anim_delete(this, nullptr);
        anim_running_ = false;
    }
}

void Label::applyStaticAlignment() {
    if (!label_ || !container_) return;

    lv_align_t align;
    switch (alignment_) {
        case LV_TEXT_ALIGN_RIGHT:
            align = LV_ALIGN_RIGHT_MID;
            break;
        case LV_TEXT_ALIGN_CENTER:
            align = LV_ALIGN_CENTER;
            break;
        default:
            align = LV_ALIGN_LEFT_MID;
            break;
    }
    lv_obj_align(label_, align, 0, 0);
}

void Label::scrollAnimCallback(void* var, int32_t value) {
    auto* self = static_cast<Label*>(var);
    if (self->label_) {
        lv_obj_set_x(self->label_, value);
    }
}

void Label::pauseTimerCallback(lv_timer_t* timer) {
    auto* self = static_cast<Label*>(lv_timer_get_user_data(timer));
    if (!self || !self->label_) return;

    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, self);
    lv_anim_set_exec_cb(&anim, scrollAnimCallback);
    lv_anim_set_values(&anim, -self->overflow_amount_, 0);
    lv_anim_set_duration(&anim, self->scroll_duration_ms_);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_set_completed_cb(&anim, [](lv_anim_t* a) {
        auto* self = static_cast<Label*>(a->var);
        self->anim_running_ = false;
    });

    lv_anim_start(&anim);
}

void Label::sizeChangedCallback(lv_event_t* e) {
    auto* self = static_cast<Label*>(lv_event_get_user_data(e));
    if (self) {
        self->checkOverflowAndScroll();
    }
}

}  // namespace oc::ui::lvgl
