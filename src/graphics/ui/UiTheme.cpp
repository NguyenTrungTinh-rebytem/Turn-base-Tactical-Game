#include "UiTheme.h"
namespace UiTheme {
Color elementColor(Element element) {
    switch (element) {
        case Element::KIM:  return {210, 218, 224, 255};
        case Element::MOC:  return {119, 194, 132, 255};
        case Element::THUY: return {108, 172, 220, 255};
        case Element::HOA:  return {237, 110, 83, 255};
        case Element::THO:  return {195, 146, 102, 255};
    }
    return TEXT;
}
}
