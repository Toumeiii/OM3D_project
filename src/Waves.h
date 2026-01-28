#ifndef OM3D_WAVES_HH
#define OM3D_WAVES_HH

#include "SceneObject.h"

namespace OM3D {
    class Waves {
    private:
        std::shared_ptr<Program> _program;
        int _size = 512;
        float _time = 0.f;
        std::vector<Texture> _init_waves_textures;
    
    public:
        size_t octave = 4;
        
        Waves();
        [[nodiscard]] std::vector<Texture> get_waves();
    };
}

#endif //OM3D_WAVES_HH