#include "Application.h"

#include <GL/glew.h>
#include <GL/glut.h>

#include <iomanip>
#include <iostream>

#include "ImGUI/imgui.h"

// ImGui helper
static void HelpMarker(const char* desc) {
    ImGui::TextDisabled("(!)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void Application::init() {
    srand(time(NULL));

    bPlay = true;
    glClearColor(1.f, 1.f, 1.f, 1.0f);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    for (unsigned int i = 0; i < 256; i++) {
        keys[i] = false;
        specialKeys[i] = false;
    }
    mouseButtons[0] = false;
    mouseButtons[1] = false;
    lastMousePos = glm::ivec2(-1, -1);

    frameCount = 0;
    timeCounter = 0;
    framerate = 0;

    TPS = 10000000;
    hystheresisFactor = 0.5f;
    hystheresisMode = ABS_DIST_HYSTHERESIS;

    cursorInGameMode = true;
    bDrawPoints = false;
    repMode = AVG;
    clusterMode = VOXEL;

    minLODLevel = 6;
    maxLODLevel = 9;

    currentLOD = minLODLevel;

    bUseFixedLODs = false;
    bShowColoredLODs = true;
    bUpdateVisibility = true;
    bUseHystheresis = false;
    bUseVisibility = true;

    scene.init();
}

bool Application::update(int deltaTime) {
    this->deltaTime = deltaTime / 1000.;
    scene.update(this->deltaTime);

    frameCount++;
    timeCounter += deltaTime;

    if (timeCounter >= 1000) {
        this->framerate = (frameCount * 1000) / float(timeCounter);
        frameCount = 0;
        timeCounter = 0;
    }

    return bPlay;
}

void Application::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    scene.render();

    ImGui::Begin("Debug options");
    ImGui::Text("ImGui FR: %.1f FPS", ImGui::GetIO().Framerate);
    ImGui::Text("Computed FR: %.1f FPS", framerate);
    ImGui::Dummy(ImVec2(0.0f, 2.0f));

    ImGui::Separator();  // ------------

    ImGui::Dummy(ImVec2(0.0f, 2.0f));
    ImGui::Checkbox("Render points", &bDrawPoints);
    if (ImGui::Checkbox("Wireframe", &bPolygonFill))
        scene.switchPolygonMode();
    ImGui::Dummy(ImVec2(0.0f, 2.0f));

    ImGui::Separator();  // ------------

    // Representative strategy
    ImGui::Dummy(ImVec2(0.0f, 2.0f));

    ImGui::Text("Representative computation strategy:");
    ImGui::SameLine();
    HelpMarker("CHANGING THIS MAY INTRODUCE A SIGNIFICANT WAIT TIME.\nSelecting a new strategy will reload the whole scene applying it.");

    const char* repItems[] = {"[AVG] Average", "[QEM] Quadric Error Metrics"};
    if (ImGui::Combo("##repmode", &repMode, repItems, IM_ARRAYSIZE(repItems))) {
        TriangleMesh::clearMeshes();
        cout << "\n\n\n---- Switching to representative computation strategy: " << repItems[repMode] << " ---- \n"
             << endl;
        scene.setupMuseumScene(false);
    }

    ImGui::Dummy(ImVec2(0.0f, 2.0f));

    ImGui::Separator();  // ------------

    // Clustering strategy
    ImGui::Dummy(ImVec2(0.0f, 2.0f));

    ImGui::Text("Vertex clustering strategy:");
    ImGui::SameLine();
    HelpMarker("CHANGING THIS MAY INTRODUCE A SIGNIFICANT WAIT TIME.\nSelecting a new strategy will reload the whole scene applying it.");

    const char* clusterItems[] = {"[VOX] Voxel clustering", "[VOX + NC] Voxel + Normal clustering"};

    if (ImGui::Combo("##clustermode", &clusterMode, clusterItems, IM_ARRAYSIZE(clusterItems))) {
        TriangleMesh::clearMeshes();
        cout << "\n\n\n---- Switching to vertex cluster strategy: " << clusterItems[clusterMode] << " ---- \n"
             << endl;
        scene.setupMuseumScene(false);
    }
    ImGui::Dummy(ImVec2(0.0f, 2.0f));

    ImGui::Separator();  // ------------
    
    ImGui::Dummy(ImVec2(0.0f, 2.0f));

    ImGui::Checkbox("Show colored LODs", &bShowColoredLODs);

    ImGui::Dummy(ImVec2(0.0f, 2.0f));

    ImGui::Separator();  // ------------

    ImGui::Dummy(ImVec2(0.0f, 2.0f));

    if (ImGui::Checkbox("Use fixed LODs", &bUseFixedLODs)) {
        if (bUseFixedLODs)
            scene.setAllNodesToLOD(currentLOD);
    }

    if (bUseFixedLODs) {
        int prevLod = currentLOD;
        if (ImGui::SliderInt("LOD", &currentLOD, minLODLevel, maxLODLevel)) {
            if (currentLOD > prevLod)
                scene.increaseAllNodesLOD();
            else
                scene.decreaseAllNodesLOD();
        }
    } else {
        ImGui::Text("Triangles per second (TPS):");
        ImGui::SliderInt("##Triangles per Second", &TPS, 1000000, 300000000);
    }

    ImGui::Dummy(ImVec2(0.0f, 2.0f));

    ImGui::Separator();  // ------------

    ImGui::Dummy(ImVec2(0.0f, 2.0f));

    ImGui::Checkbox("Use hystheresis", &bUseHystheresis);

    if (bUseHystheresis) {
        ImGui::Text("Hystheresis mode:");
        const char* hystheresisModeItems[] = {"[A] Absolute distance hystheresis", "[R] Relative distance hystheresis"};

        HelpMarker("Absolute distance: Individual LOD transitions will be re-allowed after moving closer/further than the defined distance\n Relative distance: Individual LOD transitions will be re-allowed after moving closer/further than the distance from the node to the the camera scaled by the defined factor");
        if (ImGui::Combo("##hystheresismode", &hystheresisMode, hystheresisModeItems, IM_ARRAYSIZE(hystheresisModeItems)))
            hystheresisFactor = (hystheresisMode == ABS_DIST_HYSTHERESIS) ? 0.5f : 0.1f;
        if (hystheresisMode == ABS_DIST_HYSTHERESIS) {
            ImGui::Text("Hystheresis distance:");
            ImGui::SliderFloat("##Hystheresis distance", &hystheresisFactor, 0.f, 5.f);
        } else if (hystheresisMode == REL_DIST_HYSTHERESIS) {
            ImGui::Text("Hystheresis factor:");
            ImGui::SliderFloat("##Hystheresis factor", &hystheresisFactor, 0.f, 2.f);
        }
    }

    ImGui::Dummy(ImVec2(0.0f, 2.0f));

    ImGui::Separator();  // ------------

    ImGui::Dummy(ImVec2(0.0f, 2.0f));

    ImGui::Checkbox("Use visibility", &bUseVisibility);

    if (ImGui::Button(bUpdateVisibility ? "Freeze visibility" : "Unfreeze visibility"))
        bUpdateVisibility = !bUpdateVisibility;

    ImGui::End();
}

void Application::resize(int width, int height) {
    glViewport(0, 0, width, height);
    scene.getCamera().resizeCameraViewport(width, height);
    this->width = width;
    this->height = height;
}

void Application::keyPressed(int key) {
    if (key == 27)  // Escape code
        bPlay = false;
    keys[key] = true;

    if (key > 48 && key < 58)
        scene.setNumInstances(key - 48);
}

void Application::keyReleased(int key) {
    keys[key] = false;
}

void Application::specialKeyPressed(int key) {
    specialKeys[key] = true;
}

void Application::specialKeyReleased(int key) {
    specialKeys[key] = false;
    if (key == GLUT_KEY_F1)
        scene.switchPolygonMode();
}

void Application::mouseMove(int x, int y) {
    // Zoom
    // if (mouseButtons[1] && lastMousePos.x != -1)
    //     glutSetCursor(GLUT_CURSOR_CROSSHAIR);

    lastMousePos = glm::ivec2(x, y);
}

void Application::mousePassiveMove(int x, int y) {
    if (!cursorInGameMode) return;

    glm::vec2 center((float)width / 2, (float)height / 2);
    glm::vec2 cameraAngle(-(y - center.y), -(x - center.x));

    if (cameraAngle.x != 0.0f || cameraAngle.y != 0.0f) {
        scene.getCamera().rotateCamera(0.5f * deltaTime * cameraAngle.x, 0.5f * deltaTime * cameraAngle.y);
        glutPostRedisplay();

        if (x != center.x || y != center.y)
            glutWarpPointer(center.x, center.y);
    }
}

void Application::mousePress(int button) {
    mouseButtons[button] = true;
}

void Application::mouseRelease(int button) {
    mouseButtons[button] = false;
    if (button == mouseButtons[0]) {
        lastMousePos = glm::ivec2(width / 2.0f, height / 2.0f);
        cursorInGameMode = !cursorInGameMode;

        cursorInGameMode ? glutSetCursor(GLUT_CURSOR_NONE) : glutSetCursor(GLUT_CURSOR_INHERIT);
    }
}

bool Application::getKey(int key) const {
    return keys[key];
}

bool Application::getSpecialKey(int key) const {
    return specialKeys[key];
}