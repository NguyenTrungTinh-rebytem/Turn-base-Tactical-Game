#pragma once

// Enum Ngũ hành chuẩn theo đề bài
enum class Element { 
    KIM = 0, 
    MOC, 
    THUY, 
    HOA, 
    THO 
};

// Ma trận khắc hệ Ngũ hành 5x5
constexpr float ELEMENT_MATRIX[5][5] = {
    { 1.0f, 1.5f, 1.5f, 0.5f, 0.5f }, // KIM
    { 0.5f, 1.0f, 1.5f, 0.5f, 1.5f }, // MOC
    { 0.5f, 0.5f, 1.0f, 1.5f, 1.5f }, // THUY
    { 1.5f, 1.5f, 0.5f, 1.0f, 0.5f }, // HOA
    { 1.5f, 0.5f, 0.5f, 1.5f, 1.0f }  // THO
};
