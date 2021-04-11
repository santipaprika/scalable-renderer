#include <GL/glew.h>
#include <GL/glut.h>
#include "Application.h"
#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_glut.h"
#include "ImGUI/imgui_impl_opengl3.h"

//Remove console (only works in Visual Studio)
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

#define TIME_PER_FRAME 1000.f / 60.f // Approx. 60 fps

static int prevTime;
static Application app; // This object represents our whole app

// If a key is pressed this callback is called

static void keyboardDownCallback(unsigned char key, int x, int y)
{
    ImGui_ImplGLUT_KeyboardFunc(key, x, y);
    if (ImGui::GetIO().WantCaptureKeyboard) return;

    Application::instance().keyPressed(key);
}

// If a key is released this callback is called

static void keyboardUpCallback(unsigned char key, int x, int y)
{
    ImGui_ImplGLUT_KeyboardUpFunc(key, x, y);
    if (ImGui::GetIO().WantCaptureKeyboard) return;

    Application::instance().keyReleased(key);
}

// If a special key is pressed this callback is called

static void specialDownCallback(int key, int x, int y)
{
    ImGui_ImplGLUT_SpecialFunc(key, x, y);
    if (ImGui::GetIO().WantCaptureKeyboard) return;

    Application::instance().specialKeyPressed(key);
}

// If a special key is released this callback is called

static void specialUpCallback(int key, int x, int y)
{
    ImGui_ImplGLUT_SpecialUpFunc(key, x, y);
    if (ImGui::GetIO().WantCaptureKeyboard) return;

    Application::instance().specialKeyReleased(key);
}

// Same for changes in mouse cursor position

static void motionCallback(int x, int y)
{
    ImGui_ImplGLUT_MotionFunc(x, y);
    if (ImGui::GetIO().WantCaptureMouse) return;

    Application::instance().mouseMove(x, y);
}

static void passiveMotionCallback(int x, int y)
{
    // ImGui_ImplGLUT_PassiveMotionFunc(x, y);
    // if (ImGui::GetIO().WantCaptureMouse) return;
    Application::instance().mousePassiveMove(x, y);
}

// Same for mouse button presses or releases

static void mouseCallback(int button, int state, int x, int y)
{
    ImGui_ImplGLUT_MouseFunc(button, state, x, y);
    if (ImGui::GetIO().WantCaptureMouse) return;

    int buttonId;

    switch (button)
    {
    case GLUT_LEFT_BUTTON:
        buttonId = 0;
        break;
    case GLUT_RIGHT_BUTTON:
        buttonId = 1;
        break;
    case GLUT_MIDDLE_BUTTON:
        buttonId = 2;
        break;
    default:
        return;
    }

    if (state == GLUT_DOWN)
        Application::instance().mousePress(buttonId);
    else if (state == GLUT_UP)
        Application::instance().mouseRelease(buttonId);
}

// Resizing the window calls this function

static void resizeCallback(int width, int height)
{
    ImGui_ImplGLUT_ReshapeFunc(width, height);

    Application::instance().resize(width, height);
}

static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
static void drawCallback()
{


    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGLUT_NewFrame();

    Application::instance().render();
    ImGui::ShowDemoWindow();
    
    // Rendering
    ImGui::Render();
    ImGuiIO &io = ImGui::GetIO();
    glViewport(0, 0, (GLsizei)io.DisplaySize.x, (GLsizei)io.DisplaySize.y);
    // glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    // glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound, but prefer using the GL3+ code.
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glutSwapBuffers();
}

static void idleCallback()
{
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    int deltaTime = currentTime - prevTime;

    if (deltaTime > TIME_PER_FRAME)
    {
        // Every time we enter here is equivalent to a game loop execution
        if (!Application::instance().update(deltaTime))
            exit(0);
        prevTime = currentTime;
        glutPostRedisplay();
    }
}

int main(int argc, char **argv)
{
    // GLUT initialization
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(640, 480);

    glutCreateWindow(argv[0]);
    glutReshapeFunc(resizeCallback);
    glutDisplayFunc(drawCallback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGLUT_Init();
    ImGui_ImplOpenGL3_Init();

    glutIdleFunc(idleCallback);
    glutKeyboardFunc(keyboardDownCallback);
    glutKeyboardUpFunc(keyboardUpCallback);
    glutSpecialFunc(specialDownCallback);
    glutSpecialUpFunc(specialUpCallback);
    glutMouseFunc(mouseCallback);
    glutPassiveMotionFunc(passiveMotionCallback);
    glutMotionFunc(motionCallback);
    glutSetCursor( GLUT_CURSOR_NONE );

    // GLEW will take care of OpenGL extension functions
    glewExperimental = GL_TRUE;
    glewInit();

    // Application instance initialization
    Application::instance().init();
    if (argc > 1)
        Application::instance().loadMesh(argv[1]);
    prevTime = glutGet(GLUT_ELAPSED_TIME);
    // GLUT gains control of the application
    glutMainLoop();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLUT_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
