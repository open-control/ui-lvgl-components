#include <oc/ui/lvgl/widget/VirtualList.hpp>

#include <algorithm>

#include <config/PlatformCompat.hpp>
#include <oc/ui/lvgl/theme/BaseTheme.hpp>

namespace oc::ui::lvgl::widget {

// ══════════════════════════════════════════════════════════════════════════════
// Construction / Destruction
// ══════════════════════════════════════════════════════════════════════════════

VirtualList::VirtualList(lv_obj_t* parent) {
    // Use theme defaults
    padding_ = base_theme::layout::LIST_PAD;
    itemGap_ = base_theme::layout::LIST_ITEM_GAP;
    marginH_ = base_theme::layout::MARGIN_MD;

    createContainer(parent);
    createSelectionCursor();
}

VirtualList::~VirtualList() {
    slots_.clear();

    if (container_) {
        lv_obj_delete(container_);
        container_ = nullptr;
        selectionCursor_ = nullptr;
    }
}

// ══════════════════════════════════════════════════════════════════════════════
// Fluent Configuration
// ══════════════════════════════════════════════════════════════════════════════

VirtualList& VirtualList::visibleCount(int count) {
    if (initialized_) return *this;

    if (count > 0 && count != visibleCount_) {
        visibleCount_ = count;
    }
    return *this;
}

VirtualList& VirtualList::itemHeight(int height) {
    if (height > 0) {
        itemHeight_ = height;
        autoSizing_ = false;
        if (initialized_) {
            // Update slot heights
            for (auto& slot : slots_) {
                if (slot.container) {
                    lv_obj_set_height(slot.container, itemHeight_);
                }
            }
        }
    }
    return *this;
}

VirtualList& VirtualList::size(lv_coord_t width, lv_coord_t height) {
    if (container_) {
        lv_obj_set_size(container_, width, height);
        if (autoSizing_) {
            recalculateItemHeight();
        }
    }
    return *this;
}

VirtualList& VirtualList::scrollMode(ScrollMode mode) {
    if (scrollMode_ != mode) {
        scrollMode_ = mode;
        windowStart_ = -1;  // Force recalculation
        if (visible_) {
            rebindAllSlots();
        }
    }
    return *this;
}

VirtualList& VirtualList::selectionCursorStyle(uint32_t color, lv_opa_t opa, lv_coord_t radius) {
    selectionCursorColor_ = color;
    selectionCursorOpa_ = opa;
    selectionCursorRadius_ = radius;
    if (selectionCursor_) {
        lv_obj_set_style_bg_color(selectionCursor_, lv_color_hex(selectionCursorColor_), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(selectionCursor_, selectionCursorOpa_, LV_STATE_DEFAULT);
        lv_obj_set_style_radius(selectionCursor_, selectionCursorRadius_, LV_STATE_DEFAULT);
    }
    return *this;
}

VirtualList& VirtualList::padding(int16_t pad) {
    padding_ = pad;
    if (container_) {
        lv_obj_set_style_pad_all(container_, padding_, LV_STATE_DEFAULT);
        if (autoSizing_) {
            recalculateItemHeight();
        }
    }
    return *this;
}

VirtualList& VirtualList::itemGap(int16_t gap) {
    itemGap_ = gap;
    if (container_) {
        lv_obj_set_style_pad_row(container_, itemGap_, LV_STATE_DEFAULT);
        if (autoSizing_) {
            recalculateItemHeight();
        }
    }
    return *this;
}

VirtualList& VirtualList::marginH(int16_t margin) {
    marginH_ = margin;
    if (container_) {
        lv_obj_set_style_margin_left(container_, marginH_, LV_STATE_DEFAULT);
        lv_obj_set_style_margin_right(container_, marginH_, LV_STATE_DEFAULT);
    }
    return *this;
}

// ══════════════════════════════════════════════════════════════════════════════
// Callbacks
// ══════════════════════════════════════════════════════════════════════════════

VirtualList& VirtualList::onBindSlot(BindSlotCallback callback) {
    onBindSlot_ = std::move(callback);
    return *this;
}

VirtualList& VirtualList::onUpdateHighlight(UpdateHighlightCallback callback) {
    onUpdateHighlight_ = std::move(callback);
    return *this;
}

// ══════════════════════════════════════════════════════════════════════════════
// Data
// ══════════════════════════════════════════════════════════════════════════════

bool VirtualList::setTotalCount(int count) {
    count = std::max(0, count);
    bool changed = (totalCount_ != count);
    totalCount_ = count;

    if (selectedIndex_ >= totalCount_) {
        selectedIndex_ = totalCount_ > 0 ? totalCount_ - 1 : 0;
    }

    if (changed) {
        windowStart_ = -1;  // Force recalculation
        rebindAllSlots();
    } else if (totalCount_ == 0) {
        hideSelectionCursor();
    }

    return changed;
}

// ══════════════════════════════════════════════════════════════════════════════
// Navigation
// ══════════════════════════════════════════════════════════════════════════════

void VirtualList::setSelectedIndex(int index) {
    if (totalCount_ == 0) {
        hideSelectionCursor();
        return;
    }

    index = std::clamp(index, 0, totalCount_ - 1);
    if (selectedIndex_ == index) return;

    int oldIndex = selectedIndex_;
    selectedIndex_ = index;

    if (visible_ && onBindSlot_) {
        updateSelection(oldIndex, index);
    }
}

void VirtualList::invalidate() {
    rebindAllSlots();
}

void VirtualList::invalidateIndex(int logicalIndex) {
    int slotIdx = logicalIndexToSlotIndex(logicalIndex);
    if (slotIdx >= 0 && slotIdx < static_cast<int>(slots_.size()) && onBindSlot_) {
        VirtualSlot& slot = slots_[slotIdx];
        bool isSelected = (logicalIndex == selectedIndex_);
        onBindSlot_(slot, logicalIndex, isSelected);
    }
}

VirtualSlot* VirtualList::getSlotForIndex(int logicalIndex) {
    int slotIdx = logicalIndexToSlotIndex(logicalIndex);
    if (slotIdx < 0 || slotIdx >= static_cast<int>(slots_.size())) return nullptr;
    return slots_[slotIdx].boundIndex == logicalIndex ? &slots_[slotIdx] : nullptr;
}

// ══════════════════════════════════════════════════════════════════════════════
// IComponent
// ══════════════════════════════════════════════════════════════════════════════

void VirtualList::prepare() {
    if (!container_ || initialized_) return;

    createSlots();
    initialized_ = true;
}

void VirtualList::show() {
    if (container_) {
        lv_obj_clear_flag(container_, LV_OBJ_FLAG_HIDDEN);
        visible_ = true;
        if (autoSizing_) recalculateItemHeight();
        prepare();

        rebindAllSlots();
        updateSelectionCursor();
    }
}

void VirtualList::hide() {
    if (container_) {
        lv_obj_add_flag(container_, LV_OBJ_FLAG_HIDDEN);
        visible_ = false;
        hideSelectionCursor();
    }
}

// ══════════════════════════════════════════════════════════════════════════════
// Private: Container & Slots Creation
// ══════════════════════════════════════════════════════════════════════════════

FLASHMEM void VirtualList::createContainer(lv_obj_t* parent) {
    container_ = lv_obj_create(parent);
    lv_obj_set_size(container_, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(container_, 1);

    lv_obj_set_style_bg_opa(container_, LV_OPA_TRANSP, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(container_, 0, LV_STATE_DEFAULT);

    lv_obj_set_style_pad_all(container_, padding_, LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(container_, itemGap_, LV_STATE_DEFAULT);
    lv_obj_set_style_margin_left(container_, marginH_, LV_STATE_DEFAULT);
    lv_obj_set_style_margin_right(container_, marginH_, LV_STATE_DEFAULT);

    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_clear_flag(container_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(container_, LV_OBJ_FLAG_HIDDEN);

    // Listen for size changes for auto-sizing
    lv_obj_add_event_cb(container_, sizeChangedCallback, LV_EVENT_SIZE_CHANGED, this);
    lv_obj_add_event_cb(container_, layoutChangedCallback, LV_EVENT_LAYOUT_CHANGED, this);
}

FLASHMEM void VirtualList::createSlots() {
    slots_.reserve(visibleCount_);

    // Calculate item height if auto-sizing and not yet calculated
    int height = itemHeight_;
    if (autoSizing_ && height == 0) {
        recalculateItemHeight();
        height = itemHeight_;
    }
    if (height == 0) {
        height = 32;  // Fallback default
    }

    for (int i = 0; i < visibleCount_; i++) {
        VirtualSlot slot;

        slot.container = lv_obj_create(container_);
        lv_obj_set_width(slot.container, LV_PCT(100));
        lv_obj_set_height(slot.container, height);

        lv_obj_set_style_bg_opa(slot.container, LV_OPA_TRANSP, LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(slot.container, 0, LV_STATE_DEFAULT);

        lv_obj_set_style_pad_left(slot.container, base_theme::layout::PAD_BUTTON_H, LV_STATE_DEFAULT);
        lv_obj_set_style_pad_right(slot.container, base_theme::layout::MARGIN_LG, LV_STATE_DEFAULT);
        lv_obj_set_style_pad_top(slot.container, base_theme::layout::PAD_BUTTON_V, LV_STATE_DEFAULT);
        lv_obj_set_style_pad_bottom(slot.container, base_theme::layout::PAD_BUTTON_V, LV_STATE_DEFAULT);
        lv_obj_set_style_pad_column(slot.container, base_theme::layout::MARGIN_MD, LV_STATE_DEFAULT);

        lv_obj_set_flex_flow(slot.container, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(slot.container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        lv_obj_clear_flag(slot.container, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(slot.container, LV_OBJ_FLAG_HIDDEN);

        slot.boundIndex = -1;

        slots_.push_back(slot);
    }

    if (selectionCursor_) {
        lv_obj_move_background(selectionCursor_);
    }
}

void VirtualList::recalculateItemHeight() {
    if (!autoSizing_ || !container_) return;

    // Get container content height
    lv_coord_t containerHeight = lv_obj_get_content_height(container_);
    if (containerHeight <= 0) {
        // Container not yet laid out, will recalculate on SIZE_CHANGED event
        return;
    }

    // Calculate: available = containerHeight - (padding * 2)
    // itemHeight = (available - (gaps * (count - 1))) / count
    int totalGaps = itemGap_ * (visibleCount_ - 1);
    int availableHeight = containerHeight - totalGaps;
    int calculatedHeight = availableHeight / visibleCount_;

    if (calculatedHeight > 0 && calculatedHeight != itemHeight_) {
        itemHeight_ = calculatedHeight;

        // Update existing slot heights
        for (auto& slot : slots_) {
            if (slot.container) {
                lv_obj_set_height(slot.container, itemHeight_);
            }
        }
    }
}

// ══════════════════════════════════════════════════════════════════════════════
// Private: Core Logic
// ══════════════════════════════════════════════════════════════════════════════

int VirtualList::calculateWindowStart() const {
    if (totalCount_ == 0) return 0;

    if (scrollMode_ == ScrollMode::CenterLocked) {
        // Center-locked: keep selected item at center position
        int centerPos = visibleCount_ / 2;
        int targetStart = selectedIndex_ - centerPos;

        // Clamp to valid range [0, maxStart]
        int maxStart = std::max(0, totalCount_ - visibleCount_);
        return std::clamp(targetStart, 0, maxStart);
    }

    // PageBased (default): selectedIndex determines page
    int pageIndex = selectedIndex_ / visibleCount_;
    return pageIndex * visibleCount_;
}

int VirtualList::logicalIndexToSlotIndex(int logicalIndex) const {
    if (logicalIndex < windowStart_ || logicalIndex >= windowStart_ + visibleCount_) {
        return -1;
    }
    return logicalIndex - windowStart_;
}

void VirtualList::rebindAllSlots() {
    if (!onBindSlot_ || totalCount_ == 0) {
        for (auto& slot : slots_) {
            slot.boundIndex = -1;
            if (slot.container) lv_obj_add_flag(slot.container, LV_OBJ_FLAG_HIDDEN);
        }
        hideSelectionCursor();
        return;
    }

    int newWindowStart = calculateWindowStart();
    windowStart_ = newWindowStart;

    for (int slotIdx = 0; slotIdx < visibleCount_ && slotIdx < static_cast<int>(slots_.size()); slotIdx++) {
        int logicalIndex = windowStart_ + slotIdx;
        VirtualSlot& slot = slots_[slotIdx];

        if (logicalIndex < totalCount_) {
            slot.boundIndex = logicalIndex;
            bool isSelected = (logicalIndex == selectedIndex_);
            onBindSlot_(slot, logicalIndex, isSelected);
            lv_obj_clear_flag(slot.container, LV_OBJ_FLAG_HIDDEN);
        } else {
            slot.boundIndex = -1;
            lv_obj_add_flag(slot.container, LV_OBJ_FLAG_HIDDEN);
        }
    }

    updateSelectionCursor();
}

void VirtualList::updateSelection(int oldIndex, int newIndex) {
    int newWindowStart = calculateWindowStart();

    if (newWindowStart != windowStart_) {
        rebindAllSlots();
    } else {
        // Same window: just update highlights
        updateHighlightOnly(oldIndex, newIndex);
    }
    updateSelectionCursor();
}

void VirtualList::updateHighlightOnly(int oldIndex, int newIndex) {
    // Deactivate old highlight (if visible)
    int oldSlotIdx = logicalIndexToSlotIndex(oldIndex);
    if (oldSlotIdx >= 0 && onUpdateHighlight_) {
        onUpdateHighlight_(slots_[oldSlotIdx], false);
    } else if (oldSlotIdx >= 0 && onBindSlot_) {
        onBindSlot_(slots_[oldSlotIdx], oldIndex, false);
    }

    // Activate new highlight (if visible)
    int newSlotIdx = logicalIndexToSlotIndex(newIndex);
    if (newSlotIdx >= 0 && onUpdateHighlight_) {
        onUpdateHighlight_(slots_[newSlotIdx], true);
    } else if (newSlotIdx >= 0 && onBindSlot_) {
        onBindSlot_(slots_[newSlotIdx], newIndex, true);
    }

}

void VirtualList::createSelectionCursor() {
    if (!container_ || selectionCursor_) return;

    selectionCursor_ = lv_obj_create(container_);
    lv_obj_remove_style_all(selectionCursor_);
    lv_obj_add_flag(selectionCursor_, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_clear_flag(selectionCursor_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(selectionCursor_, lv_color_hex(selectionCursorColor_), LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(selectionCursor_, selectionCursorOpa_, LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(selectionCursor_, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(selectionCursor_, selectionCursorRadius_, LV_STATE_DEFAULT);
    lv_obj_add_flag(selectionCursor_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_background(selectionCursor_);
}

void VirtualList::updateSelectionCursor() {
    if (!selectionCursor_ || totalCount_ <= 0 || !visible_) {
        hideSelectionCursor();
        return;
    }

    const int slotIdx = logicalIndexToSlotIndex(selectedIndex_);
    if (slotIdx < 0 || slotIdx >= static_cast<int>(slots_.size())) {
        hideSelectionCursor();
        return;
    }

    moveSelectionCursorToSlot(slots_[slotIdx]);
}

void VirtualList::moveSelectionCursorToSlot(VirtualSlot& slot) {
    if (!selectionCursor_ || !slot.container || lv_obj_has_flag(slot.container, LV_OBJ_FLAG_HIDDEN)) {
        hideSelectionCursor();
        return;
    }

    lv_coord_t nextX = lv_obj_get_x(slot.container);
    lv_coord_t nextY = lv_obj_get_y(slot.container);
    lv_coord_t nextW = lv_obj_get_width(slot.container);
    lv_coord_t nextH = lv_obj_get_height(slot.container);
    if (nextW <= 0 || nextH <= 0) {
        // A newly revealed retained surface may not have geometry until LVGL's
        // next refresh. Keep the cursor hidden rather than forcing a synchronous
        // layout of the entire screen from the caller's render path.
        hideSelectionCursor();
        return;
    }

    if (!selectionCursorVisible_) {
        lv_obj_clear_flag(selectionCursor_, LV_OBJ_FLAG_HIDDEN);
        selectionCursorVisible_ = true;
    }
    if (selectionCursorX_ != nextX || selectionCursorY_ != nextY) {
        lv_obj_set_pos(selectionCursor_, nextX, nextY);
        selectionCursorX_ = nextX;
        selectionCursorY_ = nextY;
    }
    if (selectionCursorWidth_ != nextW || selectionCursorHeight_ != nextH) {
        lv_obj_set_size(selectionCursor_, nextW, nextH);
        selectionCursorWidth_ = nextW;
        selectionCursorHeight_ = nextH;
    }
}

void VirtualList::hideSelectionCursor() {
    if (!selectionCursor_ || !selectionCursorVisible_) return;
    lv_obj_add_flag(selectionCursor_, LV_OBJ_FLAG_HIDDEN);
    selectionCursorVisible_ = false;
}

// ══════════════════════════════════════════════════════════════════════════════
// Private: Event Handlers
// ══════════════════════════════════════════════════════════════════════════════

void VirtualList::sizeChangedCallback(lv_event_t* e) {
    auto* self = static_cast<VirtualList*>(lv_event_get_user_data(e));
    if (!self) return;
    if (self->autoSizing_) self->recalculateItemHeight();
    self->updateSelectionCursor();
}

void VirtualList::layoutChangedCallback(lv_event_t* e) {
    auto* self = static_cast<VirtualList*>(lv_event_get_user_data(e));
    if (self) self->updateSelectionCursor();
}

}  // namespace oc::ui::lvgl::widget
