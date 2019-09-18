/*
    File: gui.cpp
    Author: Philip Haynes
*/

#include "gui.hpp"
#include "globals.hpp"

namespace Int {

const vec3 colorBack = {1.0, 0.4, 0.1};
const vec3 colorHighlightLow = {0.2, 0.45, 0.5};
const vec3 colorHighlightMedium = {0.4, 0.9, 1.0};
const vec3 colorHighlightHigh = {0.9, 0.98, 1.0};

Gui::~Gui() {
    for (Widget* widget : allWidgets) {
        delete widget;
    }
}

void Gui::EventAssetInit() {
    globals->assets.filesToLoad.Append("DroidSans.ttf");
    // globals->assets.filesToLoad.Append("LiberationSerif-Regular.ttf");
    // globals->assets.filesToLoad.Append("OpenSans-Regular.ttf");
    // globals->assets.filesToLoad.Append("Literata[wght].ttf");
    globals->assets.filesToLoad.Append("gamma.tga");
}

void Gui::EventAssetAcquire() {
    fontIndex = globals->assets.FindMapping("DroidSans.ttf");
    // fontIndex = globals->assets.FindMapping("LiberationSerif-Regular.ttf");
    // fontIndex = globals->assets.FindMapping("OpenSans-Regular.ttf");
    // fontIndex = globals->assets.FindMapping("Literata[wght].ttf");
    texIndex = globals->assets.FindMapping("gamma.tga");
    font = &globals->assets.fonts[fontIndex];
}

void Gui::EventInitialize() {
    mainMenu.Initialize();
    settingsMenu.Initialize();
    playMenu.Initialize();
}

void Gui::EventSync() {
    mouseoverWidget = nullptr;
    mouseoverDepth = -1;
    switch (currentMenu) {
    case MENU_MAIN:
        mainMenu.Update();
        break;
    case MENU_SETTINGS:
        settingsMenu.Update();
        break;
    case MENU_PLAY:
        playMenu.Update();
        break;
    }
    if (currentMenu == MENU_PLAY) {
        globals->objects.simulationRate = min(1.0, globals->objects.simulationRate + globals->objects.timestep);
    } else {
        globals->objects.simulationRate = max(0.0, globals->objects.simulationRate - globals->objects.timestep);
    }
    readyForDraw = true;
}

void Gui::EventDraw(Array<Rendering::DrawingContext> &contexts) {
    switch (currentMenu) {
    case MENU_MAIN:
        mainMenu.Draw(contexts.Back());
        break;
    case MENU_SETTINGS:
        settingsMenu.Draw(contexts.Back());
        break;
    case MENU_PLAY:
        playMenu.Draw(contexts.Back());
        break;
    }
}

void AddWidget(Widget *parent, Widget *newWidget, bool deeper = false) {
    newWidget->depth = parent->depth + (deeper ? 1 : 0);
    if (newWidget->selectable) {
        parent->selectable = true;
    }
    parent->children.Append(newWidget);
    if (globals->gui.allWidgets.count(newWidget) == 0) {
        globals->gui.allWidgets.emplace(newWidget);
    }
}

//
//      Menu implementations
//

void MainMenu::Initialize() {
    ListV *listV = new ListV();
    listV->color = vec4(0.0);
    listV->highlight = vec4(0.0);

    Widget *spacer = new Widget();
    spacer->size.y = 0.3;
    AddWidget(listV, spacer);

    Text *title = new Text();
    title->alignH = Rendering::CENTER;
    title->bold = true;
    title->color = vec4(0.0, 0.0, 0.0, 1.0);
    title->colorOutline = vec4(1.0);
    title->outline = true;
    title->fontSize = 64.0;
    title->fontIndex = globals->gui.fontIndex;
    title->string = ToWString("AzCore Tower Defense");
    AddWidget(listV, title);

    spacer = new Widget();
    spacer->size.y = 0.4;
    AddWidget(listV, spacer);

    ListV *buttonList = new ListV();
    buttonList->fractionWidth = false;
    buttonList->size = vec2(500.0, 0.0);
    buttonList->padding = vec2(16.0);

    buttonStart = new Button();
    buttonStart->string = ToWString("Start");
    buttonStart->size.y = 64.0;
    buttonStart->fractionHeight = false;
    buttonStart->margin = vec2(16.0);
    AddWidget(buttonList, buttonStart);

    buttonSettings = new Button();
    buttonSettings->string = ToWString("Settings");
    buttonSettings->size.y = 64.0;
    buttonSettings->fractionHeight = false;
    buttonSettings->margin = vec2(16.0);
    AddWidget(buttonList, buttonSettings);

    buttonExit = new Button();
    buttonExit->string = ToWString("Exit");
    buttonExit->size.y = 64.0;
    buttonExit->fractionHeight = false;
    buttonExit->margin = vec2(16.0);
    buttonExit->highlightBG = vec4(colorBack, 0.9);
    AddWidget(buttonList, buttonExit);

    ListH *spacingList = new ListH();
    spacingList->color = vec4(0.0);
    spacingList->highlight = vec4(0.0);
    spacingList->size.y = 0.0;
    spacingList->selectionDefault = 1;

    spacer = new Widget();
    spacer->size.x = 0.5;
    AddWidget(spacingList, spacer);

    AddWidget(spacingList, buttonList);

    AddWidget(listV, spacingList);

    AddWidget(&screen, listV);
}

void MainMenu::Update() {
    screen.Update(vec2(0.0), true);
    if (buttonStart->state.Released()) {
        globals->gui.currentMenu = MENU_PLAY;
        buttonStart->string = ToWString("Continue");
    }
    if (buttonSettings->state.Released()) {
        globals->gui.currentMenu = MENU_SETTINGS;
    }
    if (buttonExit->state.Released()) {
        globals->exit = true;
    }
}

void MainMenu::Draw(Rendering::DrawingContext &context) {
    screen.Draw(context);
}

void SettingsMenu::Initialize() {
    ListV *listV = new ListV();
    listV->color = vec4(0.0);
    listV->highlight = vec4(0.0);

    Widget *spacer = new Widget();
    spacer->size.y = 0.3;
    AddWidget(listV, spacer);

    Text *title = new Text();
    title->alignH = Rendering::CENTER;
    title->bold = true;
    title->color = vec4(0.0, 0.0, 0.0, 1.0);
    title->colorOutline = vec4(1.0);
    title->outline = true;
    title->fontSize = 64.0;
    title->fontIndex = globals->gui.fontIndex;
    title->string = ToWString("Settings");
    AddWidget(listV, title);

    spacer = new Widget();
    spacer->size.y = 0.4;
    AddWidget(listV, spacer);

    ListV *actualList = new ListV();
    actualList->fractionWidth = false;
    actualList->size.x = 500.0;
    actualList->size.y = 0.0;
    actualList->padding = vec2(24.0);

    Text *settingTextTemplate = new Text();
    settingTextTemplate->fontIndex = globals->gui.fontIndex;
    settingTextTemplate->fontSize = 20.0;
    settingTextTemplate->fractionHeight = true;
    settingTextTemplate->size.y = 1.0;
    settingTextTemplate->alignV = Rendering::CENTER;

    checkFullscreen = new Checkbox();
    checkFullscreen->checked = globals->window.fullscreen;

    textboxFramerate = new TextBox();
    textboxFramerate->fontIndex = globals->gui.fontIndex;
    textboxFramerate->string = ToWString("144 frames per second");

    ListH *settingListTemplate = new ListH();
    settingListTemplate->size.y = 0.0;
    settingListTemplate->margin = vec2(8.0);
    settingListTemplate->padding = vec2(0.0);
    settingListTemplate->selectionDefault = 1;

    Array<Widget*> settingListItems = {
        checkFullscreen,
        textboxFramerate
    };
    const char *settingListNames[] = {
        "Fullscreen",
        "Framerate"
    };

    for (i32 i = 0; i < settingListItems.size; i++) {
        ListH *settingList = new ListH(*settingListTemplate);
        Text *settingText = new Text(*settingTextTemplate);
        settingText->string = ToWString(settingListNames[i]);
        AddWidget(settingList, settingText);
        AddWidget(settingList, settingListItems[i]);

        AddWidget(actualList, settingList);
    }

    ListH *buttonList = new ListH();
    buttonList->size.y = 0.0;
    buttonList->margin = vec2(0.0);
    buttonList->padding = vec2(0.0);
    buttonList->color = vec4(0.0);
    buttonList->highlight = vec4(0.0);
    buttonList->selectionDefault = 1;

    buttonBack = new Button();
    buttonBack->string = ToWString("Back");
    buttonBack->size.x = 1.0 / 2.0;
    buttonBack->size.y = 64.0;
    buttonBack->fractionHeight = false;
    buttonBack->margin = vec2(8.0);
    buttonBack->highlightBG = vec4(colorBack, 0.9);
    AddWidget(buttonList, buttonBack);

    buttonApply = new Button();
    buttonApply->string = ToWString("Apply");
    buttonApply->size.x = 1.0 / 2.0;
    buttonApply->size.y = 64.0;
    buttonApply->fractionHeight = false;
    buttonApply->margin = vec2(8.0);
    AddWidget(buttonList, buttonApply);

    AddWidget(actualList, buttonList);

    ListH *spacingList = new ListH();
    spacingList->color = vec4(0.0);
    spacingList->highlight = vec4(0.0);
    spacingList->size.y = 0.0;
    spacingList->selectionDefault = 1;

    spacer = new Widget();
    spacer->size.x = 0.5;
    AddWidget(spacingList, spacer);

    AddWidget(spacingList, actualList);

    AddWidget(listV, spacingList);

    AddWidget(&screen, listV);

    delete settingListTemplate;
    delete settingTextTemplate;
}

void SettingsMenu::Update() {
    screen.Update(vec2(0.0), true);
    if (buttonApply->state.Released()) {
        globals->window.Fullscreen(checkFullscreen->checked);
    }
    if (buttonBack->state.Released()) {
        globals->gui.currentMenu = MENU_MAIN;
    }
}

void SettingsMenu::Draw(Rendering::DrawingContext &context) {
    screen.Draw(context);
}

void PlayMenu::Initialize() {
    ListH *screenListH = new ListH();
    screenListH->fractionWidth = true;
    screenListH->size.x = 1.0;
    screenListH->padding = vec2(0.0);
    screenListH->margin = vec2(0.0);
    screenListH->color = 0.0;
    screenListH->highlight = 0.0;
    screenListH->occludes = false;
    AddWidget(&screen, screenListH);
    Widget *spacer = new Widget();
    spacer->fractionWidth = true;
    spacer->size.x = 1.0;
    AddWidget(screenListH, spacer);
    list = new ListV();
    list->fractionHeight = true;
    list->fractionWidth = false;
    list->margin = 0.0;
    list->size = vec2(300.0, 1.0);
    AddWidget(screenListH, list);

    Text *towerHeader = new Text();
    towerHeader->fontIndex = globals->gui.fontIndex;
    towerHeader->alignH = Rendering::CENTER;
    towerHeader->string = ToWString("Towers");
    AddWidget(list, towerHeader);

    ListH *gridBase = new ListH();
    gridBase->fractionWidth = true;
    gridBase->size.x = 1.0;
    gridBase->fractionHeight = false;
    gridBase->size.y = 0.0;
    gridBase->padding = vec2(0.0);
    gridBase->margin = vec2(0.0);
    gridBase->color = 0.0;
    gridBase->highlight = 0.0;

    Button *halfWidth = new Button();
    halfWidth->fractionWidth = true;
    halfWidth->size.x = 0.5;
    halfWidth->fractionHeight = false;
    halfWidth->size.y = 48.0;
    halfWidth->fontIndex = globals->gui.fontIndex;

    towerButtons.Resize(Entities::TOWER_MAX_RANGE + 1);
    for (i32 i = 0; i < towerButtons.size; i+=2) {
        ListH *grid = new ListH(*gridBase);
        for (i32 j = 0; j < 2; j++) {
            i32 index = i+j;
            if (index > towerButtons.size) break;
            towerButtons[index] = new Button(*halfWidth);
            towerButtons[index]->string = ToWString(Entities::towerStrings[index]);
            AddWidget(grid, towerButtons[index]);
        }
        AddWidget(list, grid);
    }

    spacer = new Widget();
    spacer->fractionHeight = true;
    spacer->size.y = 1.0;
    AddWidget(list, spacer);

    Button *fullWidth = new Button();
    fullWidth->fractionWidth = true;
    fullWidth->size.x = 1.0;
    fullWidth->fractionHeight = false;
    fullWidth->size.y = 48.0;
    fullWidth->fontIndex = globals->gui.fontIndex;

    buttonStartWave = new Button(*fullWidth);
    buttonStartWave->string = ToWString("Start Wave");
    AddWidget(list, buttonStartWave);

    info = new Text();
    info->size.x = 1.0;
    info->color = vec4(1.0);
    info->fontIndex = globals->gui.fontIndex;
    info->fontSize = 20.0;
    info->string = ToWString("Nothing");
    AddWidget(list, info);

    buttonMenu = new Button(*fullWidth);
    buttonMenu->string = ToWString("Menu");
    AddWidget(list, buttonMenu);

    delete gridBase;
    delete halfWidth;
    delete fullWidth;
}

void PlayMenu::Update() {
    info->string = ToWString("Wave: " + ToString(globals->entities.wave) + "\nWave Hitpoints Left: " + ToString(globals->entities.hitpointsLeft) + "\nLives: " + ToString(globals->entities.lives));
    screen.Update(vec2(0.0), true);
    if (buttonMenu->state.Released()) {
        globals->gui.currentMenu = MenuEnum::MENU_MAIN;
    }
}

void PlayMenu::Draw(Rendering::DrawingContext &context) {
    screen.Draw(context);
}

//
//      Widget implementations beyond this point
//

Widget::Widget() : children(), margin(8.0), size(1.0), fractionWidth(true), fractionHeight(true), sizeAbsolute(0.0), minSize(0.0), maxSize(-1.0), position(0.0), positionAbsolute(0.0), depth(0), selectable(false), highlighted(false), occludes(false) {}

void Widget::UpdateSize(vec2 container) {
    sizeAbsolute = vec2(0.0);
    if (size.x > 0.0) {
        sizeAbsolute.x = fractionWidth ? (container.x * size.x - margin.x * 2.0) : size.x;
    } else {
        sizeAbsolute.x = 0.0;
    }
    if (size.y > 0.0) {
        sizeAbsolute.y = fractionHeight ? (container.y * size.y - margin.y * 2.0) : size.y;
    } else {
        sizeAbsolute.y = 0.0;
    }
    LimitSize();
}

void Widget::LimitSize() {
    if (maxSize.x >= 0.0) {
        sizeAbsolute.x = median(minSize.x, sizeAbsolute.x, maxSize.x);
    } else {
        sizeAbsolute.x = max(minSize.x, sizeAbsolute.x);
    }
    if (maxSize.y >= 0.0) {
        sizeAbsolute.y = median(minSize.y, sizeAbsolute.y, maxSize.y);
    } else {
        sizeAbsolute.y = max(minSize.y, sizeAbsolute.y);
    }
}

void Widget::PushScissor(Rendering::DrawingContext &context) const {
    if (sizeAbsolute.x != 0.0 && sizeAbsolute.y != 0.0) {
        vec2i topLeft = vec2i(
            positionAbsolute.x * globals->gui.scale,
            positionAbsolute.y * globals->gui.scale
        );
        vec2i botRight = vec2i(
            ceil((positionAbsolute.x + sizeAbsolute.x) * globals->gui.scale),
            ceil((positionAbsolute.y + sizeAbsolute.y) * globals->gui.scale)
        );
        globals->rendering.PushScissor(context, topLeft, botRight);
    }
}

void Widget::PopScissor(Rendering::DrawingContext &context) const {
    if (sizeAbsolute.x != 0.0 && sizeAbsolute.y != 0.0) {
        globals->rendering.PopScissor(context);
    }
}

void Widget::Update(vec2 pos, bool selected) {
    pos += margin;
    positionAbsolute = pos;
    highlighted = selected;
    for (Widget* child : children) {
        child->Update(pos, selected);
    }
}

void Widget::Draw(Rendering::DrawingContext &context) const {
    for (const Widget* child : children) {
        child->Draw(context);
    }
}

bool Widget::MouseOver() const {
    const vec2 mouse = vec2(globals->input.cursor) / globals->gui.scale;
    return mouse.x == median(positionAbsolute.x, mouse.x, positionAbsolute.x + sizeAbsolute.x)
        && mouse.y == median(positionAbsolute.y, mouse.y, positionAbsolute.y + sizeAbsolute.y);
}

void Widget::FindMouseoverDepth(i32 actualDepth) {
    if (actualDepth <= globals->gui.mouseoverDepth) return;
    if (MouseOver()) {
        if (occludes) {
            globals->gui.mouseoverDepth = actualDepth;
            globals->gui.mouseoverWidget = this;
        }
        actualDepth++;
        for (Widget *child : children) {
            child->FindMouseoverDepth(actualDepth);
        }
    }
}

Screen::Screen() {
    margin = vec2(0.0);
}

void Screen::Update(vec2 pos, bool selected) {
    UpdateSize(globals->rendering.screenSize / globals->gui.scale);
    Widget::Update(pos, selected);
    FindMouseoverDepth(0);
}

void Screen::UpdateSize(vec2 container) {
    sizeAbsolute = container - margin * 2.0;
    for (Widget* child : children) {
        child->UpdateSize(sizeAbsolute);
    }
}

List::List() : padding(8.0), color(0.05, 0.05, 0.05, 0.9), highlight(0.05, 0.05, 0.05, 0.9), selection(-2), selectionDefault(-1) { occludes = true; }

bool List::UpdateSelection(bool selected, u8 keyCodeSelect, u8 keyCodeBack, u8 keyCodeIncrement, u8 keyCodeDecrement) {
    highlighted = selected;
    if (selected) {
        if (globals->gui.controlDepth == depth) {
            if (selection >= 0 && selection < children.size && globals->objects.Released(keyCodeSelect)) {
                globals->gui.controlDepth = children[selection]->depth;
            }
            if (globals->objects.Pressed(keyCodeIncrement)) {
                for (selection = max(selection+1, 0); selection < children.size; selection++) {
                    if (children[selection]->selectable) {
                        break;
                    }
                }
                if (selection == children.size) {
                    for (selection = 0; selection < children.size; selection++) {
                        if (children[selection]->selectable) {
                            break;
                        }
                    }
                }
                if (selection == children.size) {
                    selection = -1;
                }
            } else if (globals->objects.Pressed(keyCodeDecrement)) {
                if (selection < 0) {
                    selection = children.size - 1;
                } else {
                    --selection;
                }
                for (; selection >= 0; selection--) {
                    if (children[selection]->selectable) {
                        break;
                    }
                }
                if (selection == -1) {
                    for (selection = children.size-1; selection >= 0; selection--) {
                        if (children[selection]->selectable) {
                            break;
                        }
                    }
                }
            }
            if (selection == -2) {
                selection = selectionDefault;
            }
        } else if (globals->gui.controlDepth == depth+1 && globals->objects.Released(keyCodeBack)) {
            globals->gui.controlDepth = depth;
        }
        if (globals->gui.controlDepth > depth) {
            highlighted = false;
        }
    } else {
        selection = -2;
    }
    if (globals->gui.controlDepth == depth && selected) {
        bool reselect = false;
        if (globals->input.cursor != globals->input.cursorPrevious) {
            if (MouseOver()) {
                reselect = true;
            }
            selection = -1;
        }
        return reselect;
    }
    return false;
}

void List::Draw(Rendering::DrawingContext &context) const {
    if (color.a > 0.0) {
        // const vec2 screenSizeFactor = vec2(2.0) / globals->rendering.screenSize;
        globals->rendering.DrawQuad(context, Rendering::texBlank, highlighted ? highlight : color, positionAbsolute * globals->gui.scale, vec2(1.0), sizeAbsolute * globals->gui.scale);
    }
    PushScissor(context);
    Widget::Draw(context);
    PopScissor(context);
}

void ListV::UpdateSize(vec2 container) {
    sizeAbsolute = vec2(0.0);
    if (size.x > 0.0) {
        sizeAbsolute.x = fractionWidth ? (container.x * size.x - margin.x * 2.0) : size.x;
    } else {
        sizeAbsolute.x = padding.x * 2.0;
    }
    if (size.y > 0.0) {
        sizeAbsolute.y = fractionHeight ? (container.y * size.y - margin.y * 2.0) : size.y;
    } else {
        sizeAbsolute.y = padding.y * 2.0;
    }
    LimitSize();
    vec2 sizeForInheritance = sizeAbsolute - padding * 2.0;
    if (size.x == 0.0) {
        for (Widget* child : children) {
            child->UpdateSize(sizeForInheritance);
            vec2 childSize = child->GetSize();
            sizeAbsolute.x = max(sizeAbsolute.x, childSize.x + padding.x * 2.0);
        }
    }
    sizeForInheritance = sizeAbsolute - padding * 2.0;
    for (Widget* child : children) {
        if (child->size.y == 0.0) {
            child->UpdateSize(sizeForInheritance);
            sizeForInheritance.y -= child->GetSize().y;
        } else {
            if (!child->fractionHeight) {
                sizeForInheritance.y -= child->size.y + child->margin.y * 2.0;
            }
        }
    }
    for (Widget* child : children) {
        child->UpdateSize(sizeForInheritance);
        vec2 childSize = child->GetSize();
        if (size.x == 0.0) {
            sizeAbsolute.x = max(sizeAbsolute.x, childSize.x + padding.x * 2.0);
        }
        if (size.y == 0.0) {
            sizeAbsolute.y += childSize.y;
        }
    }
    LimitSize();
}

void ListV::Update(vec2 pos, bool selected) {
    pos += margin;
    positionAbsolute = pos;
    const bool mouseSelect = UpdateSelection(selected, KC_GP_BTN_A, KC_GP_BTN_B, KC_GP_AXIS_LS_DOWN, KC_GP_AXIS_LS_UP);
    pos += padding;
    if (mouseSelect) {
        f32 childY = pos.y;
        for (selection = 0; selection < children.size; selection++) {
            Widget *child = children[selection];
            if (!child->selectable) {
                childY += child->GetSize().y;
                continue;
            }
            child->positionAbsolute.x = pos.x + child->margin.x;
            child->positionAbsolute.y = childY + child->margin.y;
            if (child->MouseOver()) {
                break;
            }
            childY += child->GetSize().y;
        }
        if (selection == children.size) {
            selection = -1;
        }
    }
    for (i32 i = 0; i < children.size; i++) {
        Widget *child = children[i];
        child->Update(pos, selected && i == selection);
        pos.y += child->GetSize().y;
    }
}

ListH::ListH() {
    color = vec4(0.0, 0.0, 0.0, 0.9);
    highlight = vec4(0.1, 0.1, 0.1, 0.9);
    occludes = true;
}

void ListH::UpdateSize(vec2 container) {
    sizeAbsolute = vec2(0.0);
    if (size.x > 0.0) {
        sizeAbsolute.x = fractionWidth ? (container.x * size.x - margin.x * 2.0) : size.x;
    } else {
        sizeAbsolute.x = padding.x * 2.0;
    }
    if (size.y > 0.0) {
        sizeAbsolute.y = fractionHeight ? (container.y * size.y - margin.y * 2.0) : size.y;
    } else {
        sizeAbsolute.y = padding.y * 2.0;
    }
    LimitSize();
    vec2 sizeForInheritance = sizeAbsolute - padding * 2.0;
    if (size.y == 0.0) {
        for (Widget* child : children) {
            child->UpdateSize(sizeForInheritance);
            vec2 childSize = child->GetSize();
            sizeAbsolute.y = max(sizeAbsolute.y, childSize.y + padding.y * 2.0);
        }
        sizeForInheritance = sizeAbsolute - padding * 2.0;
    }
    for (Widget* child : children) {
        if (child->size.x == 0.0) {
            child->UpdateSize(sizeForInheritance);
            sizeForInheritance.x -= child->GetSize().x;
        } else {
            if (!child->fractionWidth) {
                sizeForInheritance.x -= child->size.x + child->margin.x * 2.0;
            }
        }
    }
    for (Widget* child : children) {
        child->UpdateSize(sizeForInheritance);
        vec2 childSize = child->GetSize();
        if (size.x == 0.0) {
            sizeAbsolute.x += childSize.x;
        }
        if (size.y == 0.0) {
            sizeAbsolute.y = max(sizeAbsolute.y, childSize.y + padding.y * 2.0);
        }
    }
    LimitSize();
}

void ListH::Update(vec2 pos, bool selected) {
    pos += margin;
    positionAbsolute = pos;
    const bool mouseSelect = UpdateSelection(selected, KC_GP_BTN_A, KC_GP_BTN_B, KC_GP_AXIS_LS_RIGHT, KC_GP_AXIS_LS_LEFT);
    pos += padding;
    if (mouseSelect) {
        f32 childX = pos.x;
        for (selection = 0; selection < children.size; selection++) {
            Widget *child = children[selection];
            if (child->selectable) {
                child->positionAbsolute.x = childX + child->margin.x;
                child->positionAbsolute.y = pos.y + child->margin.y;
                if (child->MouseOver()) {
                    break;
                }
            }
            childX += child->GetSize().x;
        }
        if (selection == children.size) {
            selection = -1;
        }
    }
    for (i32 i = 0; i < children.size; i++) {
        Widget *child = children[i];
        child->Update(pos, selected && i == selection);
        pos.x += child->GetSize().x;
    }
}

Text::Text() : stringFormatted(), string(), padding(0.1), fontSize(32.0), fontIndex(1), bold(false), paddingEM(true), alignH(Rendering::LEFT), alignV(Rendering::TOP), color(1.0), colorOutline(0.0, 0.0, 0.0, 1.0), outline(false) {
    size.y = 0.0;
}

void Text::UpdateSize(vec2 container) {
    if (size.x > 0.0) {
        sizeAbsolute.x = fractionWidth ? (container.x * size.x - margin.x * 2.0) : size.x;
    } else {
        sizeAbsolute.x = globals->rendering.StringWidth(stringFormatted, fontIndex) * fontSize + padding.x * (paddingEM ? fontSize * 2.0 : 2.0);
    }
    if (size.y > 0.0) {
        sizeAbsolute.y = fractionHeight ? (container.y * size.y - margin.y * 2.0) : size.y;
    } else {
        sizeAbsolute.y = Rendering::StringHeight(stringFormatted) * fontSize + padding.y * (paddingEM ? fontSize * 2.0 : 2.0);
    }
    LimitSize();
}

void Text::Update(vec2 pos, bool selected) {
    if (size.x != 0.0) {
        stringFormatted = globals->rendering.StringAddNewlines(string, fontIndex, sizeAbsolute.x / fontSize);
    } else {
        stringFormatted = string;
    }
    Widget::Update(pos, selected);
}

void Text::Draw(Rendering::DrawingContext &context) const {
    PushScissor(context);
    vec2 drawPos = positionAbsolute * globals->gui.scale;
    vec2 scale = vec2(fontSize) * globals->gui.scale;
    f32 maxWidth = sizeAbsolute.x * globals->gui.scale;
    if (alignH == Rendering::CENTER) {
        drawPos.x += maxWidth * 0.5;
    } else if (alignH == Rendering::RIGHT) {
        drawPos.x += maxWidth;
    } else {
        drawPos.x += padding.x * globals->gui.scale * (paddingEM ? fontSize : 1.0);
    }
    if (alignV == Rendering::CENTER) {
        drawPos.y += sizeAbsolute.y * 0.5 * globals->gui.scale;
    } else if (alignV == Rendering::BOTTOM) {
        drawPos.y += sizeAbsolute.y * globals->gui.scale;
    } else {
        drawPos.y += padding.y * globals->gui.scale * (paddingEM ? fontSize : 1.0);
    }
    f32 bounds = bold ? 0.425 : 0.525;
    if (outline) {
        globals->rendering.DrawText(context, stringFormatted, fontIndex, colorOutline, drawPos, scale, alignH, alignV, maxWidth, 0.1, bounds - 0.2);
    }
    globals->rendering.DrawText(context, stringFormatted, fontIndex, color, drawPos, scale, alignH, alignV, maxWidth, 0.0, bounds);
    PopScissor(context);
}

Image::Image() : texIndex(0) { occludes = true; }

void Image::Draw(Rendering::DrawingContext &context) const {
    globals->rendering.DrawQuad(context, texIndex, vec4(1.0), positionAbsolute * globals->gui.scale, vec2(1.0), sizeAbsolute * globals->gui.scale);
}

Button::Button() : string(), colorBG(0.15, 0.15, 0.15, 0.9), highlightBG(colorHighlightMedium, 0.9), colorText(1.0), highlightText(0.0, 0.0, 0.0, 1.0), fontIndex(1), fontSize(28.0), state() {
    state.canRepeat = false;
    selectable = true;
    occludes = true;
}

void Button::Update(vec2 pos, bool selected) {
    Widget::Update(pos, selected);
    bool mouseover = MouseOver();
    state.Tick(0.0);
    if (globals->gui.controlDepth != depth) {
        highlighted = false;
    }
    if (mouseover) {
        highlighted = true;
        if (globals->objects.Pressed(KC_MOUSE_LEFT)) {
            state.Press();
        }
        if (globals->objects.Released(KC_MOUSE_LEFT)) {
            state.Release();
        }
    }
    if (globals->gui.controlDepth == depth) {
        if (selected) {
            if (globals->objects.Pressed(KC_GP_BTN_A)) {
                state.Press();
            }
            if (globals->objects.Released(KC_GP_BTN_A)) {
                state.Release();
            }
        } else if (!mouseover) {
            state.Set(false, false, false);
        }
    }
}

void Button::Draw(Rendering::DrawingContext &context) const {
    PushScissor(context);
    f32 scale;
    if (state.Down()) {
        scale = 0.9;
    } else {
        scale = 1.0;
    }
    scale *= globals->gui.scale;
    vec2 drawPos = (positionAbsolute + sizeAbsolute * 0.5) * globals->gui.scale;
    globals->rendering.DrawQuad(context, Rendering::texBlank, highlighted ? highlightBG : colorBG, drawPos, vec2(1.0), sizeAbsolute * scale, vec2(0.5));
    globals->rendering.DrawText(context, string, fontIndex,  highlighted ? highlightText : colorText, drawPos, vec2(fontSize * scale), Rendering::CENTER, Rendering::CENTER, sizeAbsolute.x * globals->gui.scale);
    PopScissor(context);
}

Checkbox::Checkbox() : checked(false), colorOff(0.15, 0.15, 0.15, 0.9), highlightOff(colorHighlightLow, 0.9), colorOn(colorHighlightMedium, 1.0), highlightOn(colorHighlightHigh, 1.0) {
    selectable = true;
    size = vec2(24.0);
    fractionWidth = false;
    fractionHeight = false;
    occludes = true;
}

void Checkbox::Update(vec2 pos, bool selected) {
    Widget::Update(pos, selected);
    const bool mouseover = MouseOver();
    if (globals->gui.controlDepth != depth) {
        highlighted = false;
    }
    if (mouseover) {
        highlighted = true;
        if (globals->objects.Released(KC_MOUSE_LEFT)) {
            checked = !checked;
        }
    }
    if (globals->gui.controlDepth == depth) {
        if (selected) {
            if (globals->objects.Released(KC_GP_BTN_A)) {
                checked = !checked;
            }
        }
    }
}

void Checkbox::Draw(Rendering::DrawingContext &context) const {
    const vec4 &color = checked ? (highlighted ? highlightOn : colorOn) : (highlighted ? highlightOff : colorOff);
    globals->rendering.DrawQuad(context, Rendering::texBlank, color, positionAbsolute * globals->gui.scale, vec2(1.0), sizeAbsolute * globals->gui.scale);
}

TextBox::TextBox() : string(), colorBG(vec3(0.1), 0.9), highlightBG(vec3(0.05), 0.9), colorText(1.0), highlightText(1.0), padding(4.0), cursor(0), fontIndex(1), fontSize(16.0), cursorBlinkTimer(0.0), entry(false), multiline(true) {
    selectable = true;
    occludes = true;
    fractionWidth = false;
    fractionHeight = false;
    size.x = 250.0;
    size.y = 0.0;
    minSize.y = 24.0;
}

inline bool IsWhitespace(const char32 &c) {
    return c == ' ' || c == '\t' || c == '\n' || c == 0;
}

inline bool IsText(const char32 &c) {
    // TODO: Include tabs?
    return c >= 32 && c < 127;
}

void TextBox::CursorFromPosition(vec2 position) {
    vec2 cursorPos = (positionAbsolute + padding) * globals->gui.scale;
    f32 advanceCarry;
    cursorPos.y += fontSize * Rendering::lineHeight * globals->gui.scale;
    for (cursor = 0; cursor < stringFormatted.size; cursor++) {
        const char32 &c = stringFormatted[cursor];
        advanceCarry = globals->assets.fonts[fontIndex].font.GetGlyphInfo(c).advance.x * fontSize * globals->gui.scale * 0.5;
        cursorPos.x += advanceCarry;
        if (cursorPos.x > position.x && cursorPos.y > position.y) {
            break;
        }
        cursorPos.x += advanceCarry;
        if (c == '\n') {
            if (cursorPos.y > position.y) {
                break;
            }
            cursorPos.x = positionAbsolute.x + padding.x;
            cursorPos.y += fontSize * Rendering::lineHeight;
            continue;
        }
    }
    // cursor = max(0, cursor-1);
}

vec2 TextBox::PositionFromCursor() const {
    vec2 cursorPos = 0.0;
    for (i32 i = 0; i < cursor; i++) {
        const char32 &c = stringFormatted[i];
        if (c == '\n') {
            cursorPos.x = 0.0;
            cursorPos.y += fontSize * Rendering::lineHeight;
            continue;
        }
        cursorPos.x += globals->assets.fonts[fontIndex].font.GetGlyphInfo(c).advance.x * fontSize;
    }
    cursorPos += positionAbsolute + padding;
    cursorPos *= globals->gui.scale;
    return cursorPos;
}

void TextBox::UpdateSize(vec2 container) {
    if (size.x > 0.0) {
        sizeAbsolute.x = fractionWidth ? (container.x * size.x - margin.x * 2.0) : size.x;
    } else {
        sizeAbsolute.x = globals->rendering.StringWidth(stringFormatted, fontIndex) * fontSize + padding.x * 2.0;
    }
    if (size.y > 0.0) {
        sizeAbsolute.y = fractionHeight ? (container.y * size.y - margin.y * 2.0) : size.y;
    } else {
        sizeAbsolute.y = Rendering::StringHeight(stringFormatted) * fontSize + padding.y * 2.0;
    }
    LimitSize();
}

void TextBox::Update(vec2 pos, bool selected) {
    if (entry) {
        cursorBlinkTimer += globals->objects.timestep;
        if (cursorBlinkTimer > 1.0) {
            cursorBlinkTimer -= 1.0;
        }
        highlighted = true;
        if (globals->input.AnyKey.Pressed()) {
            for (i32 i = 0; i < globals->input.typingString.size; i++) {
                const char32 c = globals->input.typingString[i];
                if (IsText(c)) {
                    string.Insert(cursor, c);
                    cursorBlinkTimer = 0.0;
                    cursor++;
                }
            }
        }
        globals->input.typingString.Clear();
        if (globals->input.Pressed(KC_KEY_BACKSPACE)) {
            if (cursor <= string.size && cursor > 0) {
                string.Erase(cursor-1);
                cursorBlinkTimer = 0.0;
                cursor--;
            }
        }
        if (globals->input.Pressed(KC_KEY_DELETE)) {
            if (cursor < string.size) {
                string.Erase(cursor);
                cursorBlinkTimer = 0.0;
            }
        }
        if (globals->input.Pressed(KC_KEY_HOME)) {
            if (globals->input.Down(KC_KEY_LEFTCTRL) || globals->input.Down(KC_KEY_RIGHTCTRL)) {
                cursor = 0;
            } else {
                for (cursor--; cursor >= 0; cursor--) {
                    if (stringFormatted[cursor] == '\n') break;
                }
                cursor++;
            }
            cursorBlinkTimer = 0.0;
        }
        if (globals->input.Pressed(KC_KEY_END)) {
            if (globals->input.Down(KC_KEY_LEFTCTRL) || globals->input.Down(KC_KEY_RIGHTCTRL)) {
                cursor = stringFormatted.size;
            } else {
                for (; cursor < string.size; cursor++) {
                    if (stringFormatted[cursor] == '\n') break;
                }
            }
            cursorBlinkTimer = 0.0;
        }
        if (globals->input.Pressed(KC_KEY_ENTER)) {
            string.Insert(cursor, '\n');
            cursor++;
            cursorBlinkTimer = 0.0;
        }
        if (globals->input.Pressed(KC_KEY_UP)) {
            vec2 cursorPos = PositionFromCursor();
            cursorPos.y -= fontSize * globals->gui.scale * Rendering::lineHeight * 0.5;
            CursorFromPosition(cursorPos);
            cursorBlinkTimer = 0.0;
        }
        if (globals->input.Pressed(KC_KEY_DOWN)) {
            vec2 cursorPos = PositionFromCursor();
            cursorPos.y += fontSize * globals->gui.scale * Rendering::lineHeight * 1.5;
            CursorFromPosition(cursorPos);
            cursorBlinkTimer = 0.0;
        }
        if (globals->input.Pressed(KC_KEY_LEFT)) {
            cursorBlinkTimer = 0.0;
            if (globals->input.Down(KC_KEY_LEFTCTRL) || globals->input.Down(KC_KEY_RIGHTCTRL)) {
                if (IsWhitespace(string[--cursor])) {
                    for (; cursor > 0; cursor--) {
                        const char32 c = string[cursor];
                        if (!IsWhitespace(c)) {
                            cursor++;
                            break;
                        }
                    }
                } else {
                    for (; cursor > 0; cursor--) {
                        const char32 c = string[cursor];
                        if (IsWhitespace(c)) {
                            cursor++;
                            break;
                        }
                    }
                }
                cursor = max(0, cursor);
            } else {
                cursor = max(0, cursor-1);
            }
        }
        if (globals->input.Pressed(KC_KEY_RIGHT)) {
            cursorBlinkTimer = 0.0;
            if (globals->input.Down(KC_KEY_LEFTCTRL) || globals->input.Down(KC_KEY_RIGHTCTRL)) {
                if (IsWhitespace(string[cursor])) {
                    for (cursor++; cursor < string.size; cursor++) {
                        const char32 c = string[cursor];
                        if (!IsWhitespace(c)) {
                            break;
                        }
                    }
                } else {
                    for (cursor++; cursor < string.size; cursor++) {
                        const char32 c = string[cursor];
                        if (IsWhitespace(c)) {
                            break;
                        }
                    }
                }
                cursor = min(string.size, cursor);
            } else {
                cursor = min(string.size, cursor+1);
            }
        }
    }
    if (size.x != 0.0 && multiline) {
        stringFormatted = globals->rendering.StringAddNewlines(string, fontIndex, sizeAbsolute.x / fontSize);
    } else {
        stringFormatted = string;
    }
    Widget::Update(pos, selected);
    bool mouseover = MouseOver();
    if (globals->gui.controlDepth != depth) {
        highlighted = false;
    }
    if (mouseover) {
        highlighted = true;
    }
    if (globals->objects.Released(KC_MOUSE_LEFT)) {
        if (mouseover) {
            if (globals->gui.controlDepth == depth) {
                globals->gui.controlDepth = depth+1;
            }
            const vec2 mouse = vec2(globals->input.cursor) / globals->gui.scale;
            CursorFromPosition(mouse);
            cursorBlinkTimer = 0.0;
        }
        if (!mouseover && entry && globals->gui.controlDepth == depth+1) {
            globals->gui.controlDepth = depth;
            entry = false;
        } else {
            entry = mouseover;
        }
    }
    if (globals->gui.controlDepth == depth) {
        if (selected) {
            if (globals->objects.Released(KC_GP_BTN_A)) {
                entry = true;
                globals->gui.controlDepth++;
            }
        }
    } else if (globals->gui.controlDepth == depth+1) {
        if (selected) {
            if (globals->objects.Released(KC_GP_BTN_B)) {
                entry = false;
                globals->gui.controlDepth--;
            }
        }
    }
}

void TextBox::Draw(Rendering::DrawingContext &context) const {
    PushScissor(context);
    vec2 drawPos = positionAbsolute * globals->gui.scale;
    globals->rendering.DrawQuad(context, Rendering::texBlank, highlighted ? highlightBG : colorBG, drawPos, vec2(1.0), sizeAbsolute * globals->gui.scale);
    globals->rendering.DrawText(context, stringFormatted, fontIndex, highlighted ? highlightText : colorText, drawPos + padding * globals->gui.scale, vec2(fontSize * globals->gui.scale), Rendering::LEFT, Rendering::TOP, sizeAbsolute.x * globals->gui.scale);
    if (cursorBlinkTimer < 0.5 && entry) {
        vec2 cursorPos = PositionFromCursor();
        cursorPos.y -= fontSize * globals->gui.scale * 0.1;
        globals->rendering.DrawQuad(context, Rendering::texBlank, highlighted ? highlightText : colorText, cursorPos, vec2(1.0), vec2(1.0, fontSize * globals->gui.scale * Rendering::lineHeight));
    }
    PopScissor(context);
}

} // namespace Int
