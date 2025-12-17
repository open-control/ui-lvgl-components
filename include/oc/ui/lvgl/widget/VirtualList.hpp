#pragma once

/**
 * @file VirtualList.hpp
 * @brief Virtual scrolling list widget with slot pooling
 *
 * Renders only visible items using a fixed pool of reusable slots.
 * Supports lists of arbitrary size with O(1) rendering performance.
 *
 * Features:
 * - Auto-sizing: calculates item height from container dimensions
 * - Two scroll modes: PageBased (fixed pages) or CenterLocked (selection stays centered)
 * - Optional smooth scroll animation
 * - Fluent configuration API
 *
 * Usage:
 * @code
 *   auto list = VirtualList(parent)
 *       .visibleCount(5)
 *       .scrollMode(ScrollMode::CenterLocked)
 *       .onBindSlot([](VirtualSlot& slot, int index, bool selected) {
 *           // Create/update widgets in slot.container for items[index]
 *       });
 *   list.setTotalCount(64);
 *   list.setSelectedIndex(0);
 *   list.show();
 * @endcode
 */

#include <functional>
#include <vector>

#include <lvgl.h>

#include <oc/ui/lvgl/IComponent.hpp>

namespace oc::ui::lvgl::widget {

// ══════════════════════════════════════════════════════════════════════════════
// Types
// ══════════════════════════════════════════════════════════════════════════════

/**
 * @brief Scroll behavior mode for VirtualList
 */
enum class ScrollMode {
    PageBased,    ///< Window shows fixed pages, cursor moves within page
    CenterLocked  ///< Selected item stays centered, list scrolls around it
};

/**
 * @brief A reusable slot in the VirtualList
 */
struct VirtualSlot {
    lv_obj_t* container = nullptr;  ///< LVGL container (created by VirtualList)
    int boundIndex = -1;            ///< Currently bound logical index (-1 = unbound)
    void* userData = nullptr;       ///< Free pointer for owner (reusable widgets)
};

/**
 * @brief Callback to bind a slot to a logical index
 *
 * @param slot       The slot to bind (container + userData)
 * @param index      The logical index of the item to display
 * @param isSelected true if this item is currently selected
 *
 * The owner MUST:
 * - Reuse/update existing widgets in slot.userData
 * - OR create widgets if slot.userData == nullptr (first bind)
 * - Apply highlighted style if isSelected == true
 */
using BindSlotCallback = std::function<void(VirtualSlot& slot, int index, bool isSelected)>;

/**
 * @brief Optional callback to update only the highlight state
 *
 * Called when only the selection state changes (not the bound index).
 * If not provided, onBindSlot is called instead.
 */
using UpdateHighlightCallback = std::function<void(VirtualSlot& slot, bool isSelected)>;

// ══════════════════════════════════════════════════════════════════════════════
// VirtualList
// ══════════════════════════════════════════════════════════════════════════════

/**
 * @brief Virtual scrolling list with slot pooling
 *
 * Only renders visible items (visibleCount slots), reusing them as the
 * selection moves through a list of totalCount items.
 */
class VirtualList : public IComponent {
public:
    // ══════════════════════════════════════════════════════════════════
    // Construction
    // ══════════════════════════════════════════════════════════════════

    /**
     * @param parent Parent LVGL object
     */
    explicit VirtualList(lv_obj_t* parent);
    ~VirtualList() override;

    // Non-copyable, moveable
    VirtualList(const VirtualList&) = delete;
    VirtualList& operator=(const VirtualList&) = delete;
    VirtualList(VirtualList&&) noexcept;
    VirtualList& operator=(VirtualList&&) noexcept;

    // ══════════════════════════════════════════════════════════════════
    // Fluent Configuration
    // ══════════════════════════════════════════════════════════════════

    /**
     * @brief Set number of visible slots
     * @param count Number of items visible at once (default: 5)
     */
    VirtualList& visibleCount(int count);

    /**
     * @brief Set explicit item height (disables auto-sizing)
     * @param height Fixed height in pixels for each item
     */
    VirtualList& itemHeight(int height);

    /**
     * @brief Set explicit container size (disables auto-sizing)
     * @param width Container width (use LV_PCT(100) for full width)
     * @param height Container height in pixels
     */
    VirtualList& size(lv_coord_t width, lv_coord_t height);

    /**
     * @brief Set the scroll behavior mode
     * @param mode PageBased (default) or CenterLocked
     */
    VirtualList& scrollMode(ScrollMode mode);

    /**
     * @brief Enable/disable smooth scroll animation
     * @param enabled true to animate page transitions (default: false)
     */
    VirtualList& animateScroll(bool enabled);

    /**
     * @brief Set padding around the list content
     * @param pad Padding in pixels (default: BaseTheme::Layout::LIST_PAD)
     */
    VirtualList& padding(int16_t pad);

    /**
     * @brief Set gap between items
     * @param gap Gap in pixels (default: BaseTheme::Layout::LIST_ITEM_GAP)
     */
    VirtualList& itemGap(int16_t gap);

    /**
     * @brief Set horizontal margins
     * @param margin Margin in pixels (default: BaseTheme::Layout::MARGIN_MD)
     */
    VirtualList& marginH(int16_t margin);

    // ══════════════════════════════════════════════════════════════════
    // Callbacks
    // ══════════════════════════════════════════════════════════════════

    /**
     * @brief Set the callback to bind slots to logical indices
     * @param callback Function called when a slot needs to display an item
     */
    VirtualList& onBindSlot(BindSlotCallback callback);

    /**
     * @brief Set optional callback for highlight-only updates
     * @param callback Function called when only selection state changes
     */
    VirtualList& onUpdateHighlight(UpdateHighlightCallback callback);

    // ══════════════════════════════════════════════════════════════════
    // Data (called when data changes)
    // ══════════════════════════════════════════════════════════════════

    /**
     * @brief Set the total number of items in the list
     *
     * Triggers a full rebind if the visible window changes.
     */
    void setTotalCount(int count);
    int getTotalCount() const { return totalCount_; }

    // ══════════════════════════════════════════════════════════════════
    // Navigation (called on user input)
    // ══════════════════════════════════════════════════════════════════

    /**
     * @brief Set the selected index
     *
     * - If index is in visible window: updates highlight only
     * - If index moves out of window: rebinds slots (+ animation if enabled)
     */
    void setSelectedIndex(int index);
    int getSelectedIndex() const { return selectedIndex_; }

    /**
     * @brief Force a rebind of all visible slots
     *
     * Useful when underlying data changes without changing totalCount
     */
    void invalidate();

    /**
     * @brief Invalidate a single slot by logical index
     *
     * If the index is currently visible, rebinds that slot.
     */
    void invalidateIndex(int logicalIndex);

    // ══════════════════════════════════════════════════════════════════
    // Slot access (for debug or advanced cases)
    // ══════════════════════════════════════════════════════════════════

    /**
     * @brief Get the slot bound to a logical index, or nullptr if not visible
     */
    VirtualSlot* getSlotForIndex(int logicalIndex);

    /**
     * @brief Get all slots (for iteration)
     */
    const std::vector<VirtualSlot>& getSlots() const { return slots_; }

    /**
     * @brief Get the first visible logical index
     */
    int getWindowStart() const { return windowStart_; }

    // ══════════════════════════════════════════════════════════════════
    // IComponent
    // ══════════════════════════════════════════════════════════════════

    void show() override;
    void hide() override;
    bool isVisible() const override { return visible_; }
    lv_obj_t* getElement() const override { return container_; }

private:
    // Container & slots creation
    void createContainer();
    void createSlots();
    void recalculateItemHeight();

    // Core logic
    int calculateWindowStart() const;
    int logicalIndexToSlotIndex(int logicalIndex) const;
    void rebindAllSlots();
    void updateSelection(int oldIndex, int newIndex);
    void updateHighlightOnly(int oldIndex, int newIndex);
    void rebindSlot(VirtualSlot& slot, int newIndex);
    void updateSlotHighlight(VirtualSlot& slot, bool isSelected);

    // Animation
    void animateToWindowStart(int targetStart);
    static void scrollAnimCallback(void* var, int32_t value);

    // Event handlers
    static void sizeChangedCallback(lv_event_t* e);

    lv_obj_t* parent_ = nullptr;
    lv_obj_t* container_ = nullptr;

    std::vector<VirtualSlot> slots_;
    int visibleCount_ = 5;
    int itemHeight_ = 0;        // 0 = auto-calculate
    bool autoSizing_ = true;    // Calculate itemHeight from container size

    int totalCount_ = 0;
    int selectedIndex_ = 0;
    int previousSelectedIndex_ = -1;
    int windowStart_ = 0;

    BindSlotCallback onBindSlot_;
    UpdateHighlightCallback onUpdateHighlight_;

    ScrollMode scrollMode_ = ScrollMode::PageBased;
    bool animateScroll_ = false;
    bool visible_ = false;
    bool initialized_ = false;

    // Layout configuration
    int16_t padding_ = 4;       // LIST_PAD default
    int16_t itemGap_ = 2;       // LIST_ITEM_GAP default
    int16_t marginH_ = 8;       // MARGIN_MD default

    // Animation state
    lv_anim_t scrollAnim_;
    bool animRunning_ = false;
};

}  // namespace oc::ui::lvgl::widget
