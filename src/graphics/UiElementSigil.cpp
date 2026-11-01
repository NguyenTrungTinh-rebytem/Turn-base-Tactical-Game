#include "UiElementSigil.h"
#include "UiIcons.h"
#include "UiTheme.h"

Color UiElementSigil::color(Element element) {
    switch (element) {
        case Element::KIM: return UiTheme::kim;
        case Element::MOC: return UiTheme::moc;
        case Element::THUY: return UiTheme::thuy;
        case Element::HOA: return UiTheme::hoa;
        case Element::THO: return UiTheme::tho;
    }
    return UiTheme::text;
}

const char* UiElementSigil::iconKey(Element element) {
    switch (element) {
        case Element::KIM: return "element:kim";
        case Element::MOC: return "element:moc";
        case Element::THUY: return "element:thuy";
        case Element::HOA: return "element:hoa";
        case Element::THO: return "element:tho";
    }
    return "element:kim";
}

void UiElementSigil::draw(Element element, Rectangle bounds, Color tint) {
    UiIcons::draw(iconKey(element), bounds, tint);
}
