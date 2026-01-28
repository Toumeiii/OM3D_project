//
// Created by remi on 25/11/2025.
//

#ifndef OM3D_PASSTYPE_HH
#define OM3D_PASSTYPE_HH

enum class PassType {
    DEFAULT,
    MAIN,
    DEPTH,
    SHADOW,
    DEFFERED,
    SUN_IBL,
    POINT_LIGHT,
    ALPHA_LIGHT,
    OCEAN,
};

#endif //OM3D_PASSTYPE_HH