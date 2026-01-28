#include "Waves.h"

#include "Scene.h"

namespace OM3D {
    Waves::Waves() : _program(Program::from_file("waves.comp")) {
    }

    std::vector<Texture> Waves::get_waves(){
        std::vector<Texture> wave_textures;

        Texture previous_wave_derivate( glm::uvec2(_size, _size), ImageFormat::RGBA16_FLOAT, WrapMode::Repeat);

        Texture wave_derivate( glm::uvec2(_size, _size), ImageFormat::RGBA16_FLOAT, WrapMode::Repeat);


        for (size_t i = 0; i < octave; i++)
        {
            Texture wave( glm::uvec2(_size, _size), ImageFormat::RGBA16_FLOAT, WrapMode::Repeat);

            _program->set_uniform("time", _time);
            _program->set_uniform("octave", static_cast<float>(i));
            _program->bind();
            
            // bind textures
            previous_wave_derivate.bind_as_image(0, AccessType::ReadOnly);
            wave.bind_as_image(1, AccessType::WriteOnly);
            wave_derivate.bind_as_image(2, AccessType::WriteOnly);
            
            glDispatchCompute(_size / 16, _size / 16, 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            std::swap(previous_wave_derivate, wave_derivate);

            wave_textures.push_back(std::move(wave));
        }
        _time += 0.03f;
        return wave_textures;
    }

}