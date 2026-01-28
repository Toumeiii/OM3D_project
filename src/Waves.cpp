#include "Waves.h"
#include <chrono>

namespace OM3D {
    Waves::Waves() : _program(Program::from_file("waves.comp")),
    _ifft_h(Program::from_file("ifft_horizontal_waves.comp")),
    _ifft_v(Program::from_file("ifft_vertical_waves.comp")){
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

            Texture wave_img( glm::uvec2(_size, _size), ImageFormat::RGBA16_FLOAT, WrapMode::Repeat);

            Texture jacobien( glm::uvec2(_size, _size), ImageFormat::RGBA16_FLOAT, WrapMode::Repeat);

            _program->set_uniform("time", _time);
            _program->set_uniform("octave", static_cast<float>(i));
            _program->bind();
            
            // bind textures
            wave.bind_as_image(0, AccessType::WriteOnly);
            wave_img.bind_as_image(3, AccessType::WriteOnly);
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

    Texture Waves::IFFT(Texture &input_real, Texture &input_img) {
        Texture tmp_real( glm::uvec2(_size, _size), ImageFormat::RGBA16_FLOAT, WrapMode::Repeat);
        Texture tmp_img( glm::uvec2(_size, _size), ImageFormat::RGBA16_FLOAT, WrapMode::Repeat);

        _ifft_h->bind();

        input_real.bind_as_image(0, AccessType::ReadOnly);
        input_img .bind_as_image(1, AccessType::ReadOnly);
        tmp_real.bind_as_image(2, AccessType::WriteOnly);
        tmp_img.bind_as_image(3, AccessType::WriteOnly);

        glDispatchCompute(1, _size / 16, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        Texture result_real( glm::uvec2(_size, _size), ImageFormat::RGBA16_FLOAT, WrapMode::Repeat);

        _ifft_v->bind();

        tmp_real.bind_as_image(0, AccessType::ReadOnly);
        tmp_img.bind_as_image(1, AccessType::ReadOnly);
        result_real.bind_as_image(2, AccessType::WriteOnly);

        glDispatchCompute(_size / 16, 1, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        return result_real;
    }

}