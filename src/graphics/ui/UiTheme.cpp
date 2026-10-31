#include "UiTheme.h"
namespace UiTheme {
Color elementColor(Element element) {
    switch (element) {
        case Element::KIM: return {219,222,232,255};
        case Element::MOC: return {113,190,133,255};
        case Element::THUY: return {107,158,215,255};
        case Element::HOA: return {215,115,99,255};
        case Element::THO: return {195,157,107,255};
    }
    return TEXT;
}
}
