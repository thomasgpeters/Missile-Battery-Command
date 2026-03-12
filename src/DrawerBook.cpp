#include "DrawerBook.h"
#include <algorithm>

#if USE_COCOS2DX
// ============================================================================
// Cocos2d-x DrawerBook
// ============================================================================

DrawerBook* DrawerBook::create(DrawerSide side, float drawerW, float drawerH)
{
    auto* ret = new (std::nothrow) DrawerBook();
    if (ret && ret->init(side, drawerW, drawerH)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool DrawerBook::init(DrawerSide side, float drawerW, float drawerH)
{
    if (!Node::init()) return false;

    side_ = side;
    drawerW_ = drawerW;
    drawerH_ = drawerH;
    isOpen_ = false;
    currentPage_ = 0;

    panelNode_ = cocos2d::DrawNode::create();
    handleNode_ = cocos2d::DrawNode::create();
    contentNode_ = cocos2d::Node::create();

    addChild(panelNode_, 0);
    addChild(handleNode_, 1);
    addChild(contentNode_, 2);

    drawPanel();
    drawHandle();

    // Set initial closed position — handle barely visible at screen edge
    float handleW = 28.0f;
    if (side_ == DrawerSide::LEFT) {
        closedX_ = -drawerW_;
        openX_ = 0.0f;
    } else {
        auto visibleSize = cocos2d::Director::getInstance()->getVisibleSize();
        closedX_ = visibleSize.width;
        openX_ = visibleSize.width - drawerW_;
    }

    setPositionX(closedX_);
    return true;
}

void DrawerBook::drawPanel()
{
    panelNode_->clear();

    // Dark panel background
    panelNode_->drawSolidRect(
        cocos2d::Vec2(0, 0),
        cocos2d::Vec2(drawerW_, drawerH_),
        cocos2d::Color4F(0.08f, 0.09f, 0.08f, 0.95f));

    // Border
    panelNode_->drawRect(
        cocos2d::Vec2(0, 0),
        cocos2d::Vec2(drawerW_, drawerH_),
        cocos2d::Color4F(0.30f, 0.40f, 0.35f, 0.8f));

    // Inner border highlight
    panelNode_->drawRect(
        cocos2d::Vec2(3, 3),
        cocos2d::Vec2(drawerW_ - 3, drawerH_ - 3),
        cocos2d::Color4F(0.20f, 0.28f, 0.22f, 0.5f));

    // Binding spine edge (the edge that attaches to the screen side)
    float spineX = (side_ == DrawerSide::LEFT) ? 0.0f : drawerW_;
    float spineW = 6.0f;
    panelNode_->drawSolidRect(
        cocos2d::Vec2(spineX - spineW * 0.5f, 0),
        cocos2d::Vec2(spineX + spineW * 0.5f, drawerH_),
        cocos2d::Color4F(0.15f, 0.20f, 0.16f, 1.0f));
}

void DrawerBook::drawHandle()
{
    handleNode_->clear();

    float handleW = 28.0f;
    float handleH = 80.0f;
    float handleY = drawerH_ * 0.5f - handleH * 0.5f;

    float hx;
    if (side_ == DrawerSide::LEFT) {
        hx = drawerW_;
    } else {
        hx = -handleW;
    }

    // Handle tab background
    handleNode_->drawSolidRect(
        cocos2d::Vec2(hx, handleY),
        cocos2d::Vec2(hx + handleW, handleY + handleH),
        cocos2d::Color4F(0.35f, 0.45f, 0.38f, 0.9f));

    // Handle border
    handleNode_->drawRect(
        cocos2d::Vec2(hx, handleY),
        cocos2d::Vec2(hx + handleW, handleY + handleH),
        cocos2d::Color4F(0.50f, 0.60f, 0.52f, 0.8f));

    // Handle grip lines
    float midX = hx + handleW * 0.5f;
    float midY = handleY + handleH * 0.5f;
    cocos2d::Color4F gripColor(0.60f, 0.68f, 0.58f, 0.6f);
    for (int i = -2; i <= 2; i++) {
        handleNode_->drawLine(
            cocos2d::Vec2(midX - 6, midY + i * 6),
            cocos2d::Vec2(midX + 6, midY + i * 6),
            gripColor);
    }

    // Arrow indicator
    const char* arrow = (side_ == DrawerSide::LEFT) ? ">" : "<";
    if (isOpen_) {
        arrow = (side_ == DrawerSide::LEFT) ? "<" : ">";
    }
    auto* arrowLabel = cocos2d::Label::createWithSystemFont(arrow, "Courier", 14);
    arrowLabel->setPosition(cocos2d::Vec2(midX, midY + 28));
    arrowLabel->setTextColor(cocos2d::Color4B(200, 210, 190, 220));
    handleNode_->addChild(arrowLabel);
}

void DrawerBook::redrawContent()
{
    contentNode_->removeAllChildren();

    if (pages_.empty()) return;

    const auto& page = pages_[currentPage_];
    float margin = 20.0f;
    float topY = drawerH_ - 25.0f;
    float contentLeft = (side_ == DrawerSide::LEFT) ? 14.0f + margin : margin;
    float contentW = drawerW_ - 2 * margin - 14.0f;

    // Page title
    auto* titleLabel = cocos2d::Label::createWithSystemFont(
        page.title, "Courier", 12);
    titleLabel->setPosition(cocos2d::Vec2(drawerW_ * 0.5f, topY));
    titleLabel->setTextColor(cocos2d::Color4B(255, 220, 80, 255));
    titleLabel->setAlignment(cocos2d::TextHAlignment::CENTER);
    contentNode_->addChild(titleLabel);

    // Separator line
    auto* sepNode = cocos2d::DrawNode::create();
    sepNode->drawLine(
        cocos2d::Vec2(contentLeft, topY - 12),
        cocos2d::Vec2(contentLeft + contentW, topY - 12),
        cocos2d::Color4F(0.40f, 0.50f, 0.40f, 0.6f));
    contentNode_->addChild(sepNode);

    // Text content
    float lineY = topY - 28.0f;
    float lineSpacing = 13.0f;

    for (const auto& line : page.lines) {
        if (lineY < 60.0f) break;  // stop before page nav area

        cocos2d::Color4B textColor;
        float fontSize = 8.0f;
        switch (line.color) {
            case TextColor::YELLOW:
                textColor = cocos2d::Color4B(255, 220, 80, 240);
                break;
            case TextColor::GREEN:
                textColor = cocos2d::Color4B(80, 220, 80, 240);
                break;
            case TextColor::RED:
                textColor = cocos2d::Color4B(220, 80, 80, 240);
                break;
            case TextColor::CYAN:
                textColor = cocos2d::Color4B(80, 200, 220, 240);
                break;
            default:
                textColor = cocos2d::Color4B(210, 210, 200, 220);
                break;
        }

        if (line.bold) fontSize = 9.0f;

        if (line.text.empty()) {
            lineY -= lineSpacing * 0.5f;
            continue;
        }

        auto* label = cocos2d::Label::createWithSystemFont(
            line.text, "Courier", fontSize);
        label->setPosition(cocos2d::Vec2(contentLeft + 4, lineY));
        label->setAnchorPoint(cocos2d::Vec2(0, 0.5f));
        label->setTextColor(textColor);
        label->setMaxLineWidth(contentW - 8);
        contentNode_->addChild(label);

        // Estimate line wrapping
        float textW = label->getContentSize().width;
        int numLines = std::max(1, (int)(textW / (contentW - 8)) + 1);
        lineY -= lineSpacing * numLines;
    }

    // === Page navigation at bottom ===
    auto* navNode = cocos2d::DrawNode::create();

    // Nav background strip
    navNode->drawSolidRect(
        cocos2d::Vec2(contentLeft, 12),
        cocos2d::Vec2(contentLeft + contentW, 48),
        cocos2d::Color4F(0.12f, 0.14f, 0.12f, 0.8f));

    contentNode_->addChild(navNode);

    // Page indicator
    std::string pageStr = "Page " + std::to_string(currentPage_ + 1) +
                          " / " + std::to_string((int)pages_.size());
    auto* pageLabel = cocos2d::Label::createWithSystemFont(pageStr, "Courier", 8);
    pageLabel->setPosition(cocos2d::Vec2(drawerW_ * 0.5f, 30));
    pageLabel->setTextColor(cocos2d::Color4B(160, 170, 150, 200));
    contentNode_->addChild(pageLabel);

    // Prev button
    if (currentPage_ > 0) {
        auto* prevLabel = cocos2d::Label::createWithSystemFont("<< PREV", "Courier", 8);
        prevLabel->setPosition(cocos2d::Vec2(contentLeft + 35, 30));
        prevLabel->setTextColor(cocos2d::Color4B(80, 220, 80, 200));
        contentNode_->addChild(prevLabel);
    }

    // Next button
    if (currentPage_ < (int)pages_.size() - 1) {
        auto* nextLabel = cocos2d::Label::createWithSystemFont("NEXT >>", "Courier", 8);
        nextLabel->setPosition(cocos2d::Vec2(contentLeft + contentW - 35, 30));
        nextLabel->setTextColor(cocos2d::Color4B(80, 220, 80, 200));
        contentNode_->addChild(nextLabel);
    }

    // Update handle arrow direction
    drawHandle();
}

void DrawerBook::toggle()
{
    if (isOpen_) close();
    else open();
}

void DrawerBook::open()
{
    if (isOpen_) return;
    isOpen_ = true;
    if (onOpenCallback_) onOpenCallback_();
    redrawContent();
    animateToPosition(openX_);
}

void DrawerBook::close()
{
    if (!isOpen_) return;
    isOpen_ = false;
    drawHandle();
    animateToPosition(closedX_);
}

void DrawerBook::animateToPosition(float targetX)
{
    auto moveTo = cocos2d::MoveTo::create(0.25f, cocos2d::Vec2(targetX, getPositionY()));
    auto ease = cocos2d::EaseQuadraticActionOut::create(moveTo);
    runAction(ease);
}

void DrawerBook::nextPage()
{
    if (currentPage_ < (int)pages_.size() - 1) {
        currentPage_++;
        redrawContent();
    }
}

void DrawerBook::prevPage()
{
    if (currentPage_ > 0) {
        currentPage_--;
        redrawContent();
    }
}

void DrawerBook::addPage(const BookPage& page)
{
    pages_.push_back(page);
}

bool DrawerBook::handleContainsPoint(const cocos2d::Vec2& worldPoint) const
{
    auto localPos = convertToNodeSpace(worldPoint);

    float handleW = 28.0f;
    float handleH = 80.0f;
    float handleY = drawerH_ * 0.5f - handleH * 0.5f;

    float hx;
    if (side_ == DrawerSide::LEFT) {
        hx = drawerW_;
    } else {
        hx = -handleW;
    }

    return (localPos.x >= hx && localPos.x <= hx + handleW &&
            localPos.y >= handleY && localPos.y <= handleY + handleH);
}

DrawerBook::HitZone DrawerBook::hitTest(const cocos2d::Vec2& worldPoint) const
{
    auto localPos = convertToNodeSpace(worldPoint);

    // Check if in page nav area (bottom strip)
    if (localPos.y >= 12 && localPos.y <= 48 &&
        localPos.x >= 0 && localPos.x <= drawerW_) {
        float midX = drawerW_ * 0.5f;
        if (localPos.x < midX && currentPage_ > 0) return HitZone::PREV_PAGE;
        if (localPos.x > midX && currentPage_ < (int)pages_.size() - 1) return HitZone::NEXT_PAGE;
    }

    return HitZone::NONE;
}

#else
// ============================================================================
// Stub DrawerBook (no cocos2d-x)
// ============================================================================

DrawerBook* DrawerBook::create(DrawerSide side, float drawerW, float drawerH)
{
    auto* ret = new DrawerBook();
    if (ret->init(side, drawerW, drawerH)) return ret;
    delete ret;
    return nullptr;
}

bool DrawerBook::init(DrawerSide side, float drawerW, float drawerH)
{
    side_ = side;
    drawerW_ = drawerW;
    drawerH_ = drawerH;
    isOpen_ = false;
    currentPage_ = 0;
    return true;
}

void DrawerBook::toggle()
{
    if (isOpen_) close();
    else open();
}

void DrawerBook::open()
{
    if (isOpen_) return;
    isOpen_ = true;
    if (onOpenCallback_) onOpenCallback_();
}

void DrawerBook::close()
{
    isOpen_ = false;
}

void DrawerBook::addPage(const BookPage& page)
{
    pages_.push_back(page);
}

void DrawerBook::nextPage()
{
    if (currentPage_ < (int)pages_.size() - 1) currentPage_++;
}

void DrawerBook::prevPage()
{
    if (currentPage_ > 0) currentPage_--;
}

#endif
