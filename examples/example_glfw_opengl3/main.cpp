#include "glad/glad.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>

#include "imgui_impl_opengl3_loader.h"
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#include "examples/example_glfw_opengl3/shader.cpp"
#include "examples/example_glfw_opengl3/stb_image.h"
#include "examples/libs/glm/glm.hpp"
#include "examples/libs/glm/gtc/matrix_transform.hpp"
#include "examples/libs/glm/gtc/type_ptr.hpp"

class Camera {
public:
    Camera(glm::vec3 pos, glm::vec3 dir) {
        this->pos = pos;
        this->orientation = dir;

        this->updateView();
    }
    void setPos(glm::vec3 newPos) {
        this->pos = newPos;
        this->updateView();
    }

    void movePos(glm::vec3 delta) {
        this->pos += delta;
        this->updateView();
    }

    void setPitchYaw() {
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        this->orientation = glm::normalize(direction);
        this->updateView();
    }

    glm::mat4 view{};
    float pitch = 0.0;
    float yaw = 0.0;
    float speed = 0.05;
    glm::vec3 pos{};
    glm::vec3 orientation{};
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
private:
    void updateView() {
        this->view = glm::lookAt(pos, pos + orientation, up);
    }
};
std::unique_ptr<Camera> camera = std::make_unique<Camera>(glm::vec3(-4., 0., 0.), glm::vec3(0., 0., 1.));


static void errorCallback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static void processInput(GLFWwindow* window) {
    if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera->movePos(camera->speed * camera->orientation);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera->movePos( -camera->speed * camera->orientation);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera->movePos(-glm::normalize(glm::cross(camera->orientation, camera->up)) * camera->speed);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera->movePos(glm::normalize(glm::cross(camera->orientation, camera->up)) * camera->speed);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

constexpr unsigned int mainWidth = 1280;
constexpr unsigned int mainHeight = 800;

float lastX = mainWidth / 2.0;
float lastY = mainHeight / 2.0;
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos); // thanks gl btw

    lastX = static_cast<float>(xpos); lastY = static_cast<float>(ypos);

    constexpr float sensitivity = 0.1f;
    xoffset *= sensitivity; yoffset *= sensitivity;

    camera->yaw += xoffset; camera->pitch += yoffset;

    camera->pitch = std::clamp(camera->pitch, -89.0f, 89.0f);
};

double zoom = 45.0;
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    zoom -= static_cast<float>(yoffset);
    zoom = std::clamp(zoom, 1.0, 45.0);
}

unsigned int genTexture(const char* name, GLint format) {
    stbi_set_flip_vertically_on_load(true);
    unsigned int texture;
    glGenTextures(1, &texture);
    //glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(name, &width, &height, &nrChannels, 0);
    if (!data) {
        std::cout << "Failed to load image: " << name << std::endl;
        stbi_image_free(data);
        return -1;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    return texture;
}

void renderGUI(GLFWwindow *window, ImVec4* clear_color, bool* renderWire) {
    ImGui::Text("General");
    ImGui::ColorEdit4("Color", reinterpret_cast<float *>(clear_color));
    if(ImGui::Button("Quit")) {
        glfwSetWindowShouldClose(window, true);
    }
    ImGui::Checkbox("Should draw wire?", renderWire);

    ImGui::Text("Camera Controls");
    ImGui::SliderFloat("Camera speed", &(camera->speed), 0.01, 0.2);
}

void render(ImVec4 clear_color, bool renderWire, int display_w, int display_h, const Shader* shader, GLFWwindow* window, double deltaTime) {
    auto model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(static_cast<float>(glfwGetTime())) * 100, glm::vec3(0.0f, 1.0f, 0.0f));

    camera->setPitchYaw();

    auto view = camera->view;
    auto projection = glm::perspective(glm::radians(static_cast<float>(zoom)), 800.0f/ 600.0f, 0.1f, 100.0f);

    int modelLoc = glGetUniformLocation(shader->ID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    int viewLoc = glGetUniformLocation(shader->ID, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    int projLoc = glGetUniformLocation(shader->ID, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPolygonMode(GL_FRONT_AND_BACK, renderWire ? GL_LINE : GL_FILL);
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void createAndBindBuffer(float vertices[], unsigned int vertSize, /*unsigned int indices[], unsigned int idxSize, */unsigned int* VBO, unsigned int* VAO /*unsigned int* EBO*/) {
    glGenVertexArrays(1, VAO);
    glGenBuffers(1, VBO);
    //glGenBuffers(1, EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(*VAO);

    glBindBuffer(GL_ARRAY_BUFFER, *VBO);
    glBufferData(GL_ARRAY_BUFFER, vertSize, vertices, GL_STATIC_DRAW);

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxSize, indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), static_cast<void *>(nullptr));
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void *>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    //glBindVertexArray(*VAO);
}

// Main code
int main(int, char**) {
    glfwSetErrorCallback(errorCallback);
    if (!glfwInit())
        return 1;
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
    //glfwWindowHint(GLFW_SAMPLES, 4);

    //glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, 1);

    float main_scale = 1.0f;//ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
    GLFWwindow* window = glfwCreateWindow(static_cast<int>(mainWidth * main_scale), static_cast<int>(mainHeight * main_scale), "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scroll_callback);
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    glEnable(GL_DEPTH_TEST);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    Shader shader("vertexShaderSource.vert", "fragmentShaderSource.frag");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };


    unsigned int VBO, VAO;
    createAndBindBuffer(vertices, sizeof(vertices), &VBO, &VAO);

    bool renderWire = false;

    unsigned int texture1 = genTexture("brickwall.jpg", GL_RGB);
    unsigned int texture2 = genTexture("awesomeface.png", GL_RGBA);

    shader.use();

    glUniform1i(glGetUniformLocation(shader.ID, "ourTexture"), 0);
    glUniform1i(glGetUniformLocation(shader.ID, "secondTexture"), 1);

    shader.setFloat("aColor", 0.0);

    float deltaTime = 0.0f;	// time between current frame and last frame
    float lastFrame = 0.0f;

    std::chrono::time_point<std::chrono::steady_clock> timeLast = std::chrono::steady_clock::now();

    while (!glfwWindowShouldClose(window))
    {
        auto currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        processInput(window);
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        glBindVertexArray(VAO);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //ImGui::Begin("New window", 0, ImGuiWindowFlags_NoInputs);
        ImGui::Begin("New Window");
        const auto deltaTimeUI = std::chrono::high_resolution_clock::now() - timeLast;
        ImGui::Text(
        std::to_string(static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(deltaTimeUI).count()) / 1000.0).c_str(), "" //???
        );
        // thanks, really cool
        timeLast = std::chrono::high_resolution_clock::now();
        renderGUI(window, &clear_color, &renderWire);
        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        render(clear_color, renderWire, display_w, display_h, &shader, window, deltaTime);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


        glfwSwapBuffers(window);
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    //glDeleteBuffers(1, &EBO);
    //glDeleteProgram(shaderProgram);

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();

    return 0;
}