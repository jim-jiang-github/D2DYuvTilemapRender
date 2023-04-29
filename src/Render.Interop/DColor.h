#pragma once
public value struct DColor24 {
    int R;
    int G;
    int B;
    DColor24(int r, int g, int b) {
        R = r;
        G = g;
        B = b;
    }
};

public value struct DColor24F {
    float R;
    float G;
    float B;
    DColor24F(float r, float g, float b) {
        R = r;
        G = g;
        B = b;
    }
};

public value struct DColor32 {
    int A;
    int R;
    int G;
    int B;
    DColor32(int a, int r, int g, int b) {
        A = a;
        R = r;
        G = g;
        B = b;
    }
};

public value struct DColor32F {
    float A;
    float R;
    float G;
    float B;
    DColor32F(float a, float r, float g, float b) {
        A = a;
        R = r;
        G = g;
        B = b;
    }
};