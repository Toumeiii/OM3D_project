
#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <graphics.h>
#include <Scene.h>
#include <Texture.h>
#include <Framebuffer.h>
#include <TimestampQuery.h>
#include <ImGuiRenderer.h>
#include <Ocean.h>

#include <imgui/imgui.h>

#include <iostream>
#include <vector>
#include <filesystem>

using namespace OM3D;


static constexpr int STATES_SIZE = 7;
static const char * states[STATES_SIZE] = {
    "None",
    "Depth",
    "Albedo",
    "Normals",
    "Roughness",
    "Metalness",
    "No deferred",
};

static const char* current_state = states[6];

static float delta_time = 0.0f;
static float sun_altitude = 45.0f;
static float sun_azimuth = 45.0f;
static float sun_intensity = 7.0f;
static float ibl_intensity = 1.0f;
static float exposure = 0.33f;
static float ocean_size = 2.f;
static int ocean_iteration = 5.f;
static float tesselation_level = 10.f;
static float y_level = 0.f;

static std::shared_ptr<SceneObject> sphere;
static std::unique_ptr<Ocean> ocean;
static std::unique_ptr<Scene> scene;
static std::shared_ptr<Texture> envmap;

namespace OM3D {
extern bool audit_bindings_before_draw;
}

void parse_args(int argc, char** argv) {
    for(int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];

        if(arg == "--validate") {
            OM3D::audit_bindings_before_draw = true;
        } else {
            std::cerr << "Unknown argument \"" << arg << "\"" << std::endl;
        }
    }
}

void glfw_check(bool cond) {
    if(!cond) {
        const char* err = nullptr;
        glfwGetError(&err);
        std::cerr << "GLFW error: " << err << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void update_delta_time() {
    static double time = 0.0;
    const double new_time = program_time();
    delta_time = float(new_time - time);
    time = new_time;
}

void process_inputs(GLFWwindow* window, Camera& camera) {
    static glm::dvec2 mouse_pos;

    glm::dvec2 new_mouse_pos;
    glfwGetCursorPos(window, &new_mouse_pos.x, &new_mouse_pos.y);

    {
        glm::vec3 movement = {};
        if(glfwGetKey(window, 'W') == GLFW_PRESS) {
            movement += camera.forward();
        }
        if(glfwGetKey(window, 'S') == GLFW_PRESS) {
            movement -= camera.forward();
        }
        if(glfwGetKey(window, 'D') == GLFW_PRESS) {
            movement += camera.right();
        }
        if(glfwGetKey(window, 'A') == GLFW_PRESS) {
            movement -= camera.right();
        }
        if(glfwGetKey(window, 'E') == GLFW_PRESS) {
            movement += camera.up();
        }
        if(glfwGetKey(window, 'Q') == GLFW_PRESS) {
            movement -= camera.up();
        }

        float speed = 10.0f;
        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            speed *= 10.0f;
        }

        if(glm::length(movement) > 0.0f) {
            const glm::vec3 new_pos = camera.position() + movement * delta_time * speed;
            camera.set_view(glm::lookAt(new_pos, new_pos + camera.forward(), camera.up()));
        }
    }

    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        const glm::vec2 delta = glm::vec2(mouse_pos - new_mouse_pos) * 0.01f;
        if(glm::length(delta) > 0.0f) {
            glm::mat4 rot = glm::rotate(glm::mat4(1.0f), delta.x, glm::vec3(0.0f, 1.0f, 0.0f));
            rot = glm::rotate(rot, delta.y, camera.right());
            camera.set_view(glm::lookAt(camera.position(), camera.position() + (glm::mat3(rot) * camera.forward()), (glm::mat3(rot) * camera.up())));
        }
    }

    {
        int width = 0;
        int height = 0;
        glfwGetWindowSize(window, &width, &height);
        camera.set_ratio(float(width) / float(height));
    }

    mouse_pos = new_mouse_pos;
}

void load_envmap(const std::string& filename) {
    if(auto res = TextureData::from_file(filename); res.is_ok) {
        envmap = std::make_shared<Texture>(Texture::cubemap_from_equirec(res.value));
        scene->set_envmap(envmap);
    } else {
        std::cerr << "Unable to load envmap (" << filename << ")" << std::endl;
    }
}


void load_scene(const std::string& filename) {
    if(auto res = Scene::from_gltf(filename); res.is_ok) {
        scene = std::move(res.value);
        scene->set_envmap(envmap);
        scene->set_ibl_intensity(ibl_intensity);
        scene->set_sun(sun_altitude, sun_azimuth, glm::vec3(sun_intensity));
        scene->add_sphere(sphere);
        scene->add_ocean(ocean->get_ocean(scene->camera(), y_level, ocean_size, tesselation_level));
    } else {
        std::cerr << "Unable to load scene (" << filename << ")" << std::endl;
    }
}

void load_sphere(const std::string& filename) {
    if(auto res = Scene::from_gltf(filename); res.is_ok) {
        auto objects = res.value->objects();
        if (objects.size() < 1) {
            std::cerr << "Unable to load sphere (" << filename << ")" << std::endl;
        } else {
            sphere = std::make_shared<SceneObject>(objects[0]);
        }
    } else {
        std::cerr << "Unable to load scene (" << filename << ")" << std::endl;
    }
}

std::vector<std::string> list_data_files(Span<const std::string> extensions = {}) {
    std::vector<std::string> files;
    for(auto&& entry : std::filesystem::directory_iterator(data_path)) {
        if(entry.status().type() == std::filesystem::file_type::regular) {
            const auto ext = entry.path().extension();

            bool ext_match = extensions.is_empty();
            for(const std::string& e : extensions) {
                ext_match |= (ext == e);
            }

            if(ext_match) {
                files.emplace_back(entry.path().string());
            }
        }
    }
    return files;
}

template<typename F>
bool load_file_window(Span<std::string> files, F&& load_func) {
    char buffer[1024] = {};
    if(ImGui::InputText("Load file", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
        load_func(buffer);
        return true;
    }

    if(!files.is_empty()) {
        for(const std::string& p : files) {
            const auto abs = std::filesystem::absolute(p).string();
            if(ImGui::MenuItem(abs.c_str())) {
                load_func(p);
                return true;
            }
        }
    }

    return false;
}

void gui(ImGuiRenderer& imgui) {
    const ImVec4 error_text_color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
    const ImVec4 warning_text_color = ImVec4(1.0f, 0.8f, 0.4f, 1.0f);

    static bool open_gpu_profiler = false;

    PROFILE_GPU("GUI");

    imgui.start();
    DEFER(imgui.finish());


    static std::vector<std::string> load_files;

    // ImGui::ShowDemoWindow();

    bool open_scene_popup = false;
    bool load_envmap_popup = false;
    if(ImGui::BeginMainMenuBar()) {
        if(ImGui::BeginMenu("File")) {
            if(ImGui::MenuItem("Open Scene")) {
                open_scene_popup = true;
            }
            if(ImGui::MenuItem("Open Envmap")) {
                load_envmap_popup = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug")) {

            if (ImGui::BeginCombo("##combo", current_state)) {
                for (auto & item : states) {
                    const bool is_selected = current_state == item;

                    if (ImGui::Selectable(item, is_selected))
                        current_state = item;
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Lighting")) {
            ImGui::DragFloat("Exposure", &exposure, 0.01f, 0.01f, 10.0f, "%.2f", ImGuiSliderFlags_Logarithmic);

            ImGui::Separator();

            ImGui::DragFloat("IBL intensity", &ibl_intensity, 0.01f, 0.0f, 1.0f);
            scene->set_ibl_intensity(ibl_intensity);

            ImGui::Separator();

            ImGui::DragFloat("Sun Altitude", &sun_altitude, 0.1f, 0.0f, 90.0f, "%.0f");
            ImGui::DragFloat("Sun Azimuth", &sun_azimuth, 0.1f, 0.0f, 360.0f, "%.0f");
            ImGui::DragFloat("Sun Intensity", &sun_intensity, 0.05f, 0.0f, 100.0f, "%.1f");
            scene->set_sun(sun_altitude, sun_azimuth, glm::vec3(sun_intensity));

            ImGui::EndMenu();
        }

        if(scene && ImGui::BeginMenu("Ocean parameter")) {
            ImGui::InputInt("Number of iteration", &ocean_iteration);
            ImGui::DragFloat("Tesselation level", &tesselation_level, 1.f, 0.f, 100.f, "%.1f");
            ImGui::DragFloat("Minimum panel size", &ocean_size, .1f, 0.f, 100.f, "%.1f");
            ImGui::DragFloat("Y level", &y_level);

            if (ocean_iteration < 0)
                ocean_iteration = 0;

            ocean->set_iteration(ocean_iteration);

            ImGui::EndMenu();
        }

        if(scene && ImGui::BeginMenu("Scene Info")) {
            ImGui::Text("%u objects", u32(scene->objects().size()));
            ImGui::Text("%u point lights", u32(scene->point_lights().size()));
            ImGui::EndMenu();
        }

        if(ImGui::MenuItem("GPU Profiler")) {
            open_gpu_profiler = true;
        }

        ImGui::Separator();
        ImGui::TextUnformatted(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

        ImGui::Separator();
        ImGui::Text("%.2f ms", delta_time * 1000.0f);

#ifdef OM3D_DEBUG
        ImGui::Separator();
        ImGui::TextColored(warning_text_color, ICON_FA_BUG " (DEBUG)");
#endif

        if(!bindless_enabled()) {
            ImGui::Separator();
            ImGui::TextColored(error_text_color, ICON_FA_EXCLAMATION_TRIANGLE " Bindless textures not supported");
        }
        ImGui::EndMainMenuBar();
    }

    if(open_scene_popup) {
        ImGui::OpenPopup("###openscenepopup");

        const std::array<std::string, 2> extensions = {".gltf", ".glb"};
        load_files = list_data_files(extensions);
    }

    if(ImGui::BeginPopup("###openscenepopup", ImGuiWindowFlags_AlwaysAutoResize)) {
        if(load_file_window(load_files, load_scene)) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if(load_envmap_popup) {
        ImGui::OpenPopup("###openenvmappopup");

        const std::array<std::string, 3> extensions = {".png", ".jpg", ".tga"};
        load_files = list_data_files(extensions);
    }

    if(ImGui::BeginPopup("###openenvmappopup", ImGuiWindowFlags_AlwaysAutoResize)) {
        if(load_file_window(load_files, load_envmap)) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if(open_gpu_profiler) {
        if(ImGui::Begin(ICON_FA_CLOCK " GPU Profiler")) {
            const ImGuiTableFlags table_flags =
                ImGuiTableFlags_SortTristate |
                ImGuiTableFlags_NoSavedSettings |
                ImGuiTableFlags_SizingFixedFit |
                ImGuiTableFlags_BordersInnerV |
                ImGuiTableFlags_Resizable |
                ImGuiTableFlags_RowBg;

            ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, ImVec4(1, 1, 1, 0.01f));
            DEFER(ImGui::PopStyleColor());

            if(ImGui::BeginTable("##timetable", 3, table_flags)) {
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("CPU (ms)", ImGuiTableColumnFlags_NoResize, 70.0f);
                ImGui::TableSetupColumn("GPU (ms)", ImGuiTableColumnFlags_NoResize, 70.0f);
                ImGui::TableHeadersRow();

                std::vector<u32> indents;
                for(const auto& zone : retrieve_profile()) {
                    auto color_from_time = [](float time) {
                        const float t = std::min(time / 0.008f, 1.0f); // 8ms = red
                        return ImVec4(t, 1.0f - t, 0.0f, 1.0f);
                    };

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(zone.name.data());

                    ImGui::TableSetColumnIndex(1);
                    ImGui::PushStyleColor(ImGuiCol_Text, color_from_time(zone.cpu_time));
                    ImGui::Text("%.2f", zone.cpu_time * 1000.0f);

                    ImGui::TableSetColumnIndex(2);
                    ImGui::PushStyleColor(ImGuiCol_Text, color_from_time(zone.gpu_time));
                    ImGui::Text("%.2f", zone.gpu_time * 1000.0f);

                    ImGui::PopStyleColor(2);

                    if(!indents.empty() && --indents.back() == 0) {
                        indents.pop_back();
                        ImGui::Unindent();
                    }

                    if(zone.contained_zones) {
                        indents.push_back(zone.contained_zones);
                        ImGui::Indent();
                    }
                }

                ImGui::EndTable();
            }
        }
        ImGui::End();
    }
}




void load_default_scene() {
    load_sphere(std::string(data_path) + "sphere.glb");
    ocean = std::make_unique<Ocean>(Ocean());
    ocean->set_iteration(ocean_iteration);
    load_scene(std::string(data_path) + "DamagedHelmet.glb");
    load_envmap(std::string(data_path) + "cubemap.png");

    // Add lights
    {
        PointLight light;
        light.set_position(glm::vec3(1.0f, 2.0f, 4.0f));
        light.set_color(glm::vec3(0.0f, 50.0f, 0.0f));
        light.set_radius(100.0f);
        scene->add_light(std::move(light));
    }
    {
        PointLight light;
        light.set_position(glm::vec3(1.0f, 2.0f, -4.0f));
        light.set_color(glm::vec3(50.0f, 0.0f, 0.0f));
        light.set_radius(50.0f);
        scene->add_light(std::move(light));
    }
}

struct RendererState {
    static RendererState create(glm::uvec2 size) {
        RendererState state;

        state.size = size;

        if(state.size.x > 0 && state.size.y > 0) {
            state.depth_texture = Texture(size, ImageFormat::Depth32_FLOAT, WrapMode::Clamp);
            state.lit_hdr_texture = Texture(size, ImageFormat::RGBA16_FLOAT, WrapMode::Clamp);
            state.tone_mapped_texture = Texture(size, ImageFormat::RGBA8_UNORM, WrapMode::Clamp);
            state.shadow_texture = Texture(glm::uvec2(2048,2048), ImageFormat::Depth32_FLOAT, WrapMode::Clamp, TEXTURE_FLAG_COMPARE | TEXTURE_FLAG_LINEAR);
            state.albedo_roughness_texture = Texture(size, ImageFormat::RGBA8_sRGB, WrapMode::Clamp);
            state.normal_metalness_texture = Texture(size, ImageFormat::RGBA8_UNORM, WrapMode::Clamp);

            state.main_framebuffer = Framebuffer(&state.depth_texture, std::array{&state.lit_hdr_texture});
            state.tone_map_framebuffer = Framebuffer(nullptr, std::array{&state.tone_mapped_texture});
            state.depth_framebuffer = Framebuffer(&state.depth_texture);
            state.shadow_framebuffer = Framebuffer(&state.shadow_texture);
            state.deferred_framebuffer = Framebuffer(&state.depth_texture, std::array{&state.lit_hdr_texture, &state.albedo_roughness_texture, &state.normal_metalness_texture});
            state.debug_framebuffer = Framebuffer(nullptr, std::array{&state.tone_mapped_texture});
            state.sun_ibl_framebuffer = Framebuffer(nullptr, std::array{&state.lit_hdr_texture});
        }

        return state;
    }

    glm::uvec2 size = {};

    Texture depth_texture;
    Texture lit_hdr_texture;
    Texture tone_mapped_texture;
    Texture shadow_texture;
    Texture albedo_roughness_texture;
    Texture normal_metalness_texture;
    Texture sun_texture;

    Framebuffer main_framebuffer;
    Framebuffer tone_map_framebuffer;
    Framebuffer depth_framebuffer;
    Framebuffer shadow_framebuffer;
    Framebuffer deferred_framebuffer;
    Framebuffer debug_framebuffer;
    Framebuffer sun_ibl_framebuffer;
};





int main(int argc, char** argv) {
    DEBUG_ASSERT([] { std::cout << "Debug asserts enabled" << std::endl; return true; }());

    parse_args(argc, argv);

    glfw_check(glfwInit());
    DEFER(glfwTerminate());

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow* window = glfwCreateWindow(1600, 900, "OM3D", nullptr, nullptr);
    glfw_check(window);
    DEFER(glfwDestroyWindow(window));

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    init_graphics();

    std::unique_ptr<ImGuiRenderer> imgui = std::make_unique<ImGuiRenderer>(window);

    load_default_scene();

    // TODO: create program for sea (maybe not here) and delete unused
    auto tonemap_program = Program::from_files("tonemap.frag", "screen.vert");
    auto debug_program = Program::from_files("debug.frag", "screen.vert");
    auto sun_ibl_program = Program::from_files("sun_ibl.frag", "screen.vert");
    RendererState renderer;

    for(;;) {
        glfwPollEvents();
        if(glfwWindowShouldClose(window) || glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            break;
        }

        process_profile_markers();

        {
            int width = 0;
            int height = 0;
            glfwGetWindowSize(window, &width, &height);

            if(renderer.size != glm::uvec2(width, height)) {
                renderer = RendererState::create(glm::uvec2(width, height));
            }
        }

        update_delta_time();

        if(const auto& io = ImGui::GetIO(); !io.WantCaptureMouse && !io.WantCaptureKeyboard) {
            process_inputs(window, scene->camera());
        }
        // Draw everything
        {
            PROFILE_GPU("Frame");
            glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Frame");

            {
                PROFILE_GPU("Ocean Pass");
                glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Ocean Pass");

                scene->add_ocean(ocean->get_ocean(scene->camera(), y_level, ocean_size, tesselation_level));

                for (const auto &obj : scene->objects()) {
                    obj.material().set_uniform("y_level", y_level);
                }

                glPopDebugGroup(); // Ocean Pass
            }

            if (current_state == states[6]) {
                {
                    PROFILE_GPU("Main Pass");
                    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Main Pass");

                    renderer.main_framebuffer.bind(true, true);
                    renderer.shadow_texture.bind(6);

                    scene->render(PassType::MAIN);

                    glPopDebugGroup();  // Main Pass
                }
            } else {
                {
                    PROFILE_GPU("Z-prepass");
                    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Z-prepass");


                    renderer.depth_framebuffer.bind(true, false);
                    scene->render(PassType::DEPTH);

                    glPopDebugGroup();  // Z-prepass
                }


                {
                    PROFILE_GPU("Shadow Pass");
                    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Shadow Pass");

                    renderer.shadow_framebuffer.bind(true, false);
                    scene->render(PassType::SHADOW);

                    glPopDebugGroup();  // Shadow Pass
                }

                {
                    PROFILE_GPU("Deferred Pass");
                    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Deferred Pass");

                    renderer.deferred_framebuffer.bind(false, true);
                    scene->render(PassType::DEFFERED);

                    glPopDebugGroup(); // Deferred Pass
                }

                {
                    PROFILE_GPU("Sun & IBL Pass");
                    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Sun & IBL Pass");

                    renderer.sun_ibl_framebuffer.bind(false, false);

                    renderer.depth_texture.bind(0);
                    renderer.albedo_roughness_texture.bind(1);
                    renderer.normal_metalness_texture.bind(2);
                    renderer.shadow_texture.bind(6);

                    sun_ibl_program->bind();

                    scene->render(PassType::SUN_IBL);

                    glPopDebugGroup(); // Sun & IBL Pass
                }

                {
                    PROFILE_GPU("Point Lights Pass");
                    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Point Lights Pass");

                    renderer.sun_ibl_framebuffer.bind(false, false);

                    renderer.shadow_texture.bind(6);
                    renderer.depth_texture.bind(7);
                    renderer.albedo_roughness_texture.bind(8);
                    renderer.normal_metalness_texture.bind(9);

                    scene->render(PassType::POINT_LIGHT);

                    glPopDebugGroup();
                }


                // TODO: redo to show opaque and transparent objects (sea dont now type -> to decide)
                // Render the scene
                {
                    PROFILE_GPU("Alpha Pass");
                    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Alpha Pass");

                    renderer.main_framebuffer.bind(false, false);
                    renderer.shadow_texture.bind(6);

                    scene->render(PassType::ALPHA_LIGHT);

                    glPopDebugGroup();  // Alpha Pass
                }
            }

            // TODO: keep but modify a bit
            // Apply a tonemap as a full screen pass
            {
                PROFILE_GPU("Tonemap");
                glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Tonemap");

                renderer.tone_map_framebuffer.bind(false, true);
                tonemap_program->bind();
                tonemap_program->set_uniform(HASH("exposure"), exposure);
                renderer.lit_hdr_texture.bind(0);
                draw_full_screen_triangle();

                glPopDebugGroup();  // Tonemap
            }

            {
                if (current_state != nullptr && current_state != states[0] && current_state != states[6]) {
                    PROFILE_GPU("Debug");
                    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Debug");

                    size_t state = 0;
                    for (; state < STATES_SIZE && states[state] != current_state; ++state);

                    renderer.debug_framebuffer.bind(false, true);
                    debug_program->bind();
                    debug_program->set_uniform(HASH("state"), static_cast<u32>(state));
                    renderer.depth_texture.bind(0);
                    renderer.albedo_roughness_texture.bind(1);
                    renderer.normal_metalness_texture.bind(2);
                    draw_full_screen_triangle();

                    glPopDebugGroup();  // Debug
                }
            }

            // Blit tonemap result to screen
            {
                PROFILE_GPU("Blit");
                glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Blit");
                blit_to_screen(renderer.tone_mapped_texture);

                glPopDebugGroup();
            }

            // Draw GUI on top
            glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "GUI");
            gui(*imgui);
            glPopDebugGroup();  // GUI
            glPopDebugGroup();  // Frame
        }
        glfwSwapBuffers(window);
    }


    // destroy scene and child OpenGL objects
    scene = nullptr;
    envmap = nullptr;
    imgui = nullptr;
    tonemap_program = nullptr;
    renderer = {};
    destroy_graphics();
}
