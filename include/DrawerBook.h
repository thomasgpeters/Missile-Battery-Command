#ifndef __DRAWER_BOOK_H__
#define __DRAWER_BOOK_H__

#include "GameTypes.h"
#include <vector>
#include <string>
#include <functional>

// ============================================================================
// DrawerBook — Pop-out reference book drawer with tabbed handle
//
// Slides out from the left or right edge of the screen when the handle is
// clicked.  Contains multiple pages of reference text that can be turned
// with next/prev buttons.  Only one DrawerBook may be open at a time;
// opening one closes the other.
//
// Content is structured as a vector of BookPage, each with a title and
// a vector of colored text lines (white, yellow, green).
// ============================================================================

enum class TextColor { WHITE, YELLOW, GREEN, RED, CYAN };

struct BookLine {
    std::string text;
    TextColor   color;
    bool        bold;

    BookLine(const std::string& t, TextColor c = TextColor::WHITE, bool b = false)
        : text(t), color(c), bold(b) {}
};

struct BookPage {
    std::string title;
    std::vector<BookLine> lines;
};

enum class DrawerSide { LEFT, RIGHT };

#if USE_COCOS2DX
#include "cocos2d.h"

class DrawerBook : public cocos2d::Node {
public:
    static DrawerBook* create(DrawerSide side, float drawerW, float drawerH);
    virtual bool init(DrawerSide side, float drawerW, float drawerH);

    void toggle();          // Open if closed, close if open
    void open();
    void close();
    bool isOpen() const { return isOpen_; }

    void addPage(const BookPage& page);
    void nextPage();
    void prevPage();
    int  getCurrentPage() const { return currentPage_; }
    int  getPageCount() const { return (int)pages_.size(); }

    // Callback when this drawer opens (so scene can close the other)
    void setOnOpen(std::function<void()> cb) { onOpenCallback_ = cb; }

    // Handle hit test (screen-space point)
    bool handleContainsPoint(const cocos2d::Vec2& worldPoint) const;

    // Content area hit test for page turn buttons
    enum class HitZone { NONE, PREV_PAGE, NEXT_PAGE, CLOSE };
    HitZone hitTest(const cocos2d::Vec2& worldPoint) const;

private:
    DrawerSide side_;
    float drawerW_;
    float drawerH_;
    bool isOpen_;
    int currentPage_;
    std::vector<BookPage> pages_;
    std::function<void()> onOpenCallback_;

    // Visual elements
    cocos2d::DrawNode* panelNode_;
    cocos2d::DrawNode* handleNode_;
    cocos2d::Node*     contentNode_;

    // Positions
    float closedX_;     // X position when closed (handle visible)
    float openX_;       // X position when open

    void redrawContent();
    void drawHandle();
    void drawPanel();
    void animateToPosition(float targetX);
};

#else
// ============================================================================
// Stub DrawerBook (no cocos2d-x)
// ============================================================================
class DrawerBook {
public:
    static DrawerBook* create(DrawerSide side, float drawerW, float drawerH);
    bool init(DrawerSide side, float drawerW, float drawerH);

    void toggle();
    void open();
    void close();
    bool isOpen() const { return isOpen_; }

    void addPage(const BookPage& page);
    void nextPage();
    void prevPage();
    int  getCurrentPage() const { return currentPage_; }
    int  getPageCount() const { return (int)pages_.size(); }

    void setOnOpen(std::function<void()> cb) { onOpenCallback_ = cb; }

private:
    DrawerSide side_ = DrawerSide::LEFT;
    float drawerW_ = 0;
    float drawerH_ = 0;
    bool isOpen_ = false;
    int currentPage_ = 0;
    std::vector<BookPage> pages_;
    std::function<void()> onOpenCallback_;
};
#endif

#endif // __DRAWER_BOOK_H__
