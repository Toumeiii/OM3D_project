#include "Waves.h"
#include <chrono>

namespace OM3D {
    Waves::Waves() : _program(Program::from_file("waves.comp")) {
        auto init_waves_program = Program::from_file("init_waves.comp");

        _init_waves_textures.clear();
        for (size_t i = 0; i < octave; i++)
        {
            Texture wave(glm::uvec2(_size, _size), ImageFormat::RGBA16_FLOAT, WrapMode::Repeat);

            init_waves_program->set_uniform("octave", static_cast<float>(i));
            init_waves_program->bind();

            // bind textures
            wave.bind_as_image(0, AccessType::WriteOnly);
            
            glDispatchCompute((_size + 15) / 16, (_size + 15) / 16, 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            _init_waves_textures.push_back(std::move(wave));
        }
        // delete program
        init_waves_program = nullptr;
    }

    std::vector<Texture> Waves::get_waves(){
        std::vector<Texture> Textures;

        for (size_t i = 0; i < octave; i++)
        {
            Texture wave( glm::uvec2(_size, _size), ImageFormat::RGBA16_FLOAT, WrapMode::Repeat);

            Texture jacobien( glm::uvec2(_size, _size), ImageFormat::RGBA16_FLOAT, WrapMode::Repeat);

            _program->set_uniform("time", _time);
            _program->set_uniform("octave", static_cast<float>(i));
            _program->bind();
            
            // bind textures
            wave.bind_as_image(0, AccessType::WriteOnly);
            jacobien.bind_as_image(1, AccessType::WriteOnly);
            _init_waves_textures[i].bind_as_image(2, AccessType::ReadOnly);
            
            glDispatchCompute(_size / 16, _size / 16, 1);
            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            
            Textures.push_back(std::move(wave));
            Textures.push_back(std::move(jacobien));
        }
        _time += 0.03f;
        return Textures;
    }

}