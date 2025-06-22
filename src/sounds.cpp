#include <uiohook.h>
#include <iostream>
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>
#include <thread>
#include <string>
#include <fstream>
#include <atomic>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <chrono>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <filesystem>
#include "nlohmann/json.hpp"
using json = nlohmann::json;
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include "nuklear.h"
#include "nuklear_glfw_gl3.h"

#define WINDOW_WIDTH 260
#define WINDOW_HEIGHT 218

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024
namespace fs = std::filesystem;
std::string pathtosound;

ma_engine engine;
std::atomic<bool> playSoundFlag(false);
int playonkeyup = 0;
float volume = 0.0f;
float lastvolune = 0.0f;
bool ran = 0;
enum keytoplayon {UP, DOWN};
 keytoplayon keytobeon = DOWN;
 bool justmuted =false;

struct settings{
    float volume;
    keytoplayon keyonoroff;
    int currentsound;
};

static void error_callback(int e, const char *d)
{
    printf("Error %d: %s\n", e, d);
}
void playfunny()
{
    ma_result result = ma_engine_play_sound(&engine, pathtosound.c_str(), NULL);
    if (result != MA_SUCCESS)
    {
        std::cerr << "Failed to play sound." << std::endl;
    }
}
void handle_event(uiohook_event *const event)
{
    if (event->type == EVENT_KEY_PRESSED && keytobeon == DOWN)
    {
        playSoundFlag = true;
    }
    else if(event->type == EVENT_KEY_RELEASED && keytobeon == UP){
        
        playSoundFlag = true;
    }
}
void openFolder() {
    fs::path currentPath = fs::current_path();
std::string path = currentPath.string() + "\\keypresses";
#if defined(_WIN32)
    std::string command = "explorer \"" + path + "\"";
#elif defined(__APPLE__)
    std::string command = "open \"" + path + "\"";
#else
    std::string command = "xdg-open \"" + path + "\"";
#endif

    int result = std::system(command.c_str());
}


int main()
{

    std::vector<std::string> files;

    struct nk_glfw glfw{};
    static GLFWwindow *win;
    int width = 0, height = 0;
    struct nk_context *ctx;

    if (ma_engine_init(NULL, &engine) != MA_SUCCESS)
    {
        std::cerr << "Failed to initialize audio engine." << std::endl;
        return -1;
    }
    hook_set_dispatch_proc(handle_event);

    std::thread hookThread([]()
                           { hook_run(); });

    /* GLFW */
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
    {
        fprintf(stdout, "[GFLW] failed to init!\n");
        exit(1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    win = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Click Sounds", NULL, NULL);
    glfwMakeContextCurrent(win);
    glfwGetWindowSize(win, &width, &height);

    /* OpenGL */
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glewExperimental = 1;
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to setup GLEW\n");
        exit(1);
    }

    ctx = nk_glfw3_init(&glfw, win, NK_GLFW3_INSTALL_CALLBACKS);
    {
        struct nk_font_atlas *atlas;
        nk_glfw3_font_stash_begin(&glfw, &atlas);
        nk_glfw3_font_stash_end(&glfw);
    }
    std::cout << "files found: " << std::endl;
    for (const auto &entry : fs::directory_iterator("./keypresses"))
    {
        if (fs::is_regular_file(entry.status()))
        {

            files.push_back(entry.path().string());
            std::cout << entry.path() << std::endl;
        }
    }
std::vector<std::string> purepath = files;

std::vector<const char*> file_cstrs;

for (const auto& str : files) {

    file_cstrs.push_back(str.c_str());
}
for (auto& path : files){
    path.erase(0, 13);
    path.erase(path.end() - 4,path.end());
}
float volume = 0.0f;
float lastvolune = 0.0f;
std::string volumetext;

int muted;

std::ifstream inputFile("./settings/settings.json");
if (!inputFile.is_open()) {
    std::cerr << "Failed to open settings.json\n";
    // Handle error: return, exit, or create default JSON
}
static int selected_item_index = 0;
json j;
inputFile >> j;
inputFile.close();

volume = j["volume"];
keytobeon = j["keyonoroff"];
selected_item_index = j["currentsound"];

int widthimg, heightimg, channelsimg;
fs::path currentPathy = fs::current_path();
std::string filepathimage = currentPathy.string() + "/assets/key.png";
    unsigned char* pixels = stbi_load(filepathimage.c_str(), &widthimg, &heightimg, &channelsimg, 4); // force RGBA
std::cout << filepathimage;
    if (!pixels) {
        std::cerr << "Failed to load icon image\n";
    } else {
        GLFWimage icon;
        icon.width = widthimg;
        icon.height = heightimg;
        icon.pixels = pixels;

        glfwSetWindowIcon(win, 1, &icon);

        stbi_image_free(pixels);
    }


// std::cout << j.dump(4);
    while (!glfwWindowShouldClose(win))
    {
        if (playSoundFlag.exchange(false))
        {
            playfunny();
        }

        glfwPollEvents();
        nk_glfw3_new_frame(&glfw);

        /* GUI */
        if (nk_begin(ctx, "Click Sounds", nk_rect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT),
                     NK_WINDOW_SCALABLE |
                         NK_WINDOW_TITLE))
        {

            nk_window_set_size(ctx, "Click Sounds", nk_vec2(glfw.width, glfw.height));
            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "Click Sounds", NK_TEXT_CENTERED);

            // The options we want to display
             // Selected item index
            struct nk_vec2 size = {300, 100};   // Size of the dropdown that displays all our items
            nk_combobox(ctx, file_cstrs.data(), file_cstrs.size(), &selected_item_index, 20, size);
            pathtosound = purepath[selected_item_index];
             
             nk_layout_row_dynamic(ctx, 20, 1);
            volumetext = std::to_string(volume);
            volumetext.erase(volumetext.end() - 5, volumetext.end());
            volumetext = "volume: " + volumetext;
            nk_label(ctx, volumetext.c_str(), NK_TEXT_CENTERED);
            nk_layout_row_dynamic(ctx, 20, 1);
            nk_slider_float(ctx, 0, &volume, 1.0f, 0.01f);
            ma_engine_set_volume(&engine, volume);
              struct nk_rect window_bounds = nk_rect(50, 50, 255, 340);
            if (nk_contextual_begin(ctx, NK_PANEL_CONTEXTUAL, nk_vec2(100, 250), window_bounds)) {
                nk_layout_row_dynamic(ctx, 25, 1);
                
                if(nk_contextual_item_label(ctx, "Save", NK_TEXT_CENTERED)){
                    std::ofstream outputFile("./settings/settings.json");
                    if (!outputFile.is_open()) {
                        std::cerr << "Failed to open settings.json\n";
                        // Handle error: return, exit, or create default JSON
                    }
                    json save;
                    save["volume"] = volume;
                    save["keyonoroff"] = keytobeon;
                    save["currentsound"] = selected_item_index;
                    
                    outputFile << save.dump(4);
                    outputFile.close();
                }
                if(nk_contextual_item_label(ctx, "Load", NK_TEXT_CENTERED)){
                    std::ifstream inputFile("./settings/settings.json");
                    if (!inputFile.is_open()) {
                        std::cerr << "Failed to open settings.json\n";
                        // Handle error: return, exit, or create default JSON
                    }
                    
                    json load;
                    inputFile >> load;
                    inputFile.close();

                    volume = load["volume"];
                    keytobeon = load["keyonoroff"];
                    selected_item_index = load["currentsound"];
                }
                if(nk_contextual_item_label(ctx, "Reset", NK_TEXT_CENTERED)){
                    keytobeon = DOWN;
                    volume = 0.0f;
                    selected_item_index = 0;
                    
                }
                
                nk_contextual_end(ctx);
            }
                nk_label(ctx, "play on:", NK_TEXT_CENTERED);
                nk_layout_row_dynamic(ctx, 30, 3);
                if (nk_option_label(ctx, "Key down", keytobeon == DOWN)){
                     keytobeon = DOWN;
                    }
                if (nk_option_label(ctx, "Key up", keytobeon == UP)){ 
                    keytobeon = UP;
                
                }

            nk_checkbox_label(ctx, "Muted", &muted);
            if(muted == 1){
                volume = 0.0f;
                justmuted = true;
            }  if(muted == 0){
                if(justmuted == true){
                    volume = lastvolune;
                }else{
                lastvolune = volume;
                }
                justmuted = false;
            }
            nk_layout_row_dynamic(ctx, 20, 1);
            if(nk_button_label(ctx, "Open folder")){
            openFolder();
            }
        }
        nk_end(ctx);

        glfwGetWindowSize(win, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        nk_glfw3_render(&glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
        glfwSwapBuffers(win);
    }
    nk_glfw3_shutdown(&glfw);
    glfwTerminate();

    ma_engine_uninit(&engine);
    
}
