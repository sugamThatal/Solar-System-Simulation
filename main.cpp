//  =====================================================
//   Solar System Simulation
//   Computer Graphics & Visualization — BTech Project
//   OpenGL + GLFW + GLM | macOS
//  =====================================================

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"

#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <cstdlib>

// ─────────────────────────────────────────────
//  Window & Camera
// ─────────────────────────────────────────────
const int   SCR_W = 1280, SCR_H = 720;
const float PI    = 3.14159265358979f;

glm::vec3 camPos   = {0.0f, 40.0f, 120.0f};
glm::vec3 camFront = glm::normalize(glm::vec3(0.0f, -0.3f, -1.0f));
glm::vec3 camUp    = {0.0f, 1.0f,  0.0f};

float yaw = -90.0f, pitch = -15.0f;
float lastX = SCR_W / 2.f, lastY = SCR_H / 2.f;
bool  firstMouse = true;

float  simSpeed   = 1.0f;
bool   paused     = false;
double lastFrame  = 0.0;

// ─────────────────────────────────────────────
//  Planet data
// ─────────────────────────────────────────────
struct Planet {
    std::string name;
    float radius, orbitRadius, orbitSpeed, rotSpeed;
    float orbitAngle, rotAngle, tilt;
    GLuint texture;
    bool   hasRing;
    float  ringInner, ringOuter;
    GLuint ringTexture;
    bool   hasMoon;
    float  moonRadius, moonOrbit, moonSpeed, moonAngle;
    GLuint moonTexture;
    glm::vec3 fallbackColor;
};

std::vector<Planet> planets;
GLuint sunTexture = 0, starsTexture = 0;

// ─────────────────────────────────────────────
//  Draw UV sphere
// ─────────────────────────────────────────────
void drawSphere(float r, int stacks = 40, int slices = 40) {
    for (int i = 0; i < stacks; ++i) {
        float phi0 = PI * (-0.5f + (float)i       / stacks);
        float phi1 = PI * (-0.5f + (float)(i + 1) / stacks);
        float cp0 = cosf(phi0), sp0 = sinf(phi0);
        float cp1 = cosf(phi1), sp1 = sinf(phi1);
        glBegin(GL_TRIANGLE_STRIP);
        for (int j = 0; j <= slices; ++j) {
            float theta = 2.0f * PI * j / slices;
            float ct = cosf(theta), st = sinf(theta);
            glNormal3f(ct*cp1, sp1, st*cp1);
            glTexCoord2f((float)j/slices, (float)(i+1)/stacks);
            glVertex3f(r*ct*cp1, r*sp1, r*st*cp1);
            glNormal3f(ct*cp0, sp0, st*cp0);
            glTexCoord2f((float)j/slices, (float)i/stacks);
            glVertex3f(r*ct*cp0, r*sp0, r*st*cp0);
        }
        glEnd();
    }
}

// ─────────────────────────────────────────────
//  Draw ring (Saturn)
// ─────────────────────────────────────────────
void drawRing(float inner, float outer, int seg = 80) {
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= seg; ++i) {
        float a = 2.0f * PI * i / seg;
        float c = cosf(a), s = sinf(a), t = (float)i / seg;
        glTexCoord2f(t, 0.0f); glVertex3f(inner*c, 0, inner*s);
        glTexCoord2f(t, 1.0f); glVertex3f(outer*c, 0, outer*s);
    }
    glEnd();
}

// ─────────────────────────────────────────────
//  Draw orbit path
// ─────────────────────────────────────────────
void drawOrbit(float r, int seg = 120) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < seg; ++i) {
        float a = 2.0f * PI * i / seg;
        glVertex3f(r*cosf(a), 0, r*sinf(a));
    }
    glEnd();
}

// ─────────────────────────────────────────────
//  Texture helpers
// ─────────────────────────────────────────────
GLuint solidColor(unsigned char r, unsigned char g, unsigned char b) {
    GLuint id; glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    unsigned char px[3] = {r, g, b};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, px);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return id;
}

GLuint loadTexture(const char* path) {
    int w, h, ch;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &w, &h, &ch, 0);
    if (!data) return 0;

    GLuint id; glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GLenum fmt = (ch == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
    glGenerateMipmapEXT(GL_TEXTURE_2D);
    stbi_image_free(data);
    std::cout << "[OK] " << path << "\n";
    return id;
}

GLuint tryLoad(const char* path, glm::vec3 fb) {
    GLuint id = loadTexture(path);
    if (!id) {
        std::cout << "[FB] " << path << " -> using fallback color\n";
        return solidColor((unsigned char)(fb.r*255), (unsigned char)(fb.g*255), (unsigned char)(fb.b*255));
    }
    return id;
}

// ─────────────────────────────────────────────
//  Initialise planets
// ─────────────────────────────────────────────
void initPlanets() {
    sunTexture   = tryLoad("textures/sun.jpg",   {1.0f, 0.7f, 0.0f});
    starsTexture = tryLoad("textures/stars.jpg", {0.0f, 0.0f, 0.05f});

    struct Def {
        const char* name, *tex, *ringTex, *moonTex;
        float rad, orbit, ospd, rspd, tilt;
        bool ring; float ri, ro;
        bool moon; float mr, mo, ms;
        glm::vec3 fb;
    };

    std::vector<Def> defs = {
        {"Mercury","textures/mercury.jpg","","",       1.2f, 22.f,47.87f,10.8f, 0.03f, false,0,0, false,0,0,0, {0.6f,0.6f,0.6f}},
        {"Venus",  "textures/venus.jpg",  "","",       2.0f, 35.f,35.02f, 6.5f,177.4f, false,0,0, false,0,0,0, {0.9f,0.7f,0.2f}},
        {"Earth",  "textures/earth.jpg",  "","textures/moon.jpg",
                                                       2.2f, 50.f,29.78f,36.0f, 23.4f, false,0,0, true, 0.7f,4.5f,100.f, {0.2f,0.4f,0.9f}},
        {"Mars",   "textures/mars.jpg",   "","",       1.6f, 65.f,24.13f,35.0f, 25.2f, false,0,0, false,0,0,0, {0.8f,0.3f,0.1f}},
        {"Jupiter","textures/jupiter.jpg","","",       5.5f, 90.f, 8.31f,87.0f,  3.1f, false,0,0, false,0,0,0, {0.8f,0.6f,0.4f}},
        {"Saturn", "textures/saturn.jpg", "textures/saturn_ring.png","",
                                                       4.5f,120.f, 6.09f,82.0f, 26.7f, true, 6.f,11.f, false,0,0,0, {0.9f,0.8f,0.5f}},
        {"Uranus", "textures/uranus.jpg", "","",       3.0f,148.f, 4.28f,57.0f, 97.8f, false,0,0, false,0,0,0, {0.5f,0.8f,0.9f}},
        {"Neptune","textures/neptune.jpg","","",       2.9f,172.f, 3.37f,54.0f, 28.3f, false,0,0, false,0,0,0, {0.2f,0.3f,0.9f}},
    };

    for (auto& d : defs) {
        Planet p{};
        p.name = d.name; p.radius = d.rad; p.orbitRadius = d.orbit;
        p.orbitSpeed = d.ospd; p.rotSpeed = d.rspd; p.tilt = d.tilt;
        p.orbitAngle = (float)(rand() % 360); p.rotAngle = 0;
        p.fallbackColor = d.fb;
        p.texture = tryLoad(d.tex, d.fb);
        p.hasRing = d.ring; p.ringInner = d.ri; p.ringOuter = d.ro;
        p.ringTexture = d.ring ? tryLoad(d.ringTex, {0.8f,0.7f,0.5f}) : 0;
        p.hasMoon = d.moon; p.moonRadius = d.mr; p.moonOrbit = d.mo;
        p.moonSpeed = d.ms; p.moonAngle = 0;
        p.moonTexture = (d.moon && d.moonTex[0]) ? tryLoad(d.moonTex, {0.7f,0.7f,0.7f}) : 0;
        planets.push_back(p);
    }
}

// ─────────────────────────────────────────────
//  Draw one planet
// ─────────────────────────────────────────────
void drawPlanet(Planet& p) {
    // Orbit path
    glDisable(GL_TEXTURE_2D); glDisable(GL_LIGHTING);
    glColor4f(1,1,1,0.12f);
    glEnable(GL_LINE_STIPPLE); glLineStipple(1, 0x00FF);
    drawOrbit(p.orbitRadius);
    glDisable(GL_LINE_STIPPLE);
    glEnable(GL_LIGHTING); glEnable(GL_TEXTURE_2D);

    glPushMatrix();
    glRotatef(p.orbitAngle, 0,1,0);
    glTranslatef(p.orbitRadius, 0, 0);

    // Moon
    if (p.hasMoon && p.moonTexture) {
        glPushMatrix();
        glRotatef(p.moonAngle, 0,1,0);
        glTranslatef(p.moonOrbit, 0, 0);
        glBindTexture(GL_TEXTURE_2D, p.moonTexture);
        glColor3f(1,1,1); drawSphere(p.moonRadius, 20, 20);
        glPopMatrix();
    }

    glRotatef(p.tilt,    0,0,1);
    glRotatef(p.rotAngle, 0,1,0);

    glBindTexture(GL_TEXTURE_2D, p.texture);
    glColor3f(1,1,1); drawSphere(p.radius);

    // Ring
    if (p.hasRing && p.ringTexture) {
        glPushMatrix(); glRotatef(90,1,0,0);
        glBindTexture(GL_TEXTURE_2D, p.ringTexture);
        glColor4f(1,1,1,0.75f);
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        drawRing(p.ringInner, p.ringOuter);
        glDisable(GL_BLEND);
        glPopMatrix();
    }
    glPopMatrix();
}

// ─────────────────────────────────────────────
//  Draw starfield background
// ─────────────────────────────────────────────
void drawStarfield() {
    glDisable(GL_LIGHTING); glDepthMask(GL_FALSE);
    glBindTexture(GL_TEXTURE_2D, starsTexture);
    glColor3f(1,1,1);
    glFrontFace(GL_CW); drawSphere(600.f, 30, 30); glFrontFace(GL_CCW);
    glDepthMask(GL_TRUE); glEnable(GL_LIGHTING);
}

// ─────────────────────────────────────────────
//  Draw Sun
// ─────────────────────────────────────────────
void drawSun() {
    static float sr = 0.0f; sr += 0.015f;
    glPushMatrix(); glDisable(GL_LIGHTING);
    glBindTexture(GL_TEXTURE_2D, sunTexture);
    glColor3f(1.0f, 0.95f, 0.55f);
    glRotatef(sr, 0,1,0); drawSphere(8.f, 50, 50);
    glEnable(GL_LIGHTING); glPopMatrix();
}

// ─────────────────────────────────────────────
//  Update
// ─────────────────────────────────────────────
void update(float dt) {
    if (paused) return;
    float sd = dt * simSpeed;
    for (auto& p : planets) {
        p.orbitAngle = fmodf(p.orbitAngle + p.orbitSpeed * sd, 360.f);
        p.rotAngle   = fmodf(p.rotAngle   + p.rotSpeed   * sd, 360.f);
        if (p.hasMoon) p.moonAngle = fmodf(p.moonAngle + p.moonSpeed * sd, 360.f);
    }
}

// ─────────────────────────────────────────────
//  Input callbacks
// ─────────────────────────────────────────────
void keyCallback(GLFWwindow* win, int key, int, int action, int) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
    float spd   = 3.0f;
    glm::vec3 right = glm::normalize(glm::cross(camFront, camUp));
    if (key == GLFW_KEY_W)          camPos += spd * camFront;
    if (key == GLFW_KEY_S)          camPos -= spd * camFront;
    if (key == GLFW_KEY_A)          camPos -= spd * right;
    if (key == GLFW_KEY_D)          camPos += spd * right;
    if (key == GLFW_KEY_Q)          camPos += spd * camUp;
    if (key == GLFW_KEY_E)          camPos -= spd * camUp;
    if (key == GLFW_KEY_EQUAL)      simSpeed = fminf(simSpeed * 1.5f, 200.f);
    if (key == GLFW_KEY_MINUS)      simSpeed = fmaxf(simSpeed / 1.5f, 0.05f);
    if (key == GLFW_KEY_P && action == GLFW_PRESS) paused = !paused;
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        camPos = {0,40,120}; camFront = glm::normalize(glm::vec3(0,-0.3f,-1)); 
        yaw=-90; pitch=-15; simSpeed=1;
    }
    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(win, true);
}

void mouseCallback(GLFWwindow*, double xpos, double ypos) {
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float dx = (xpos - lastX) * 0.1f, dy = (lastY - ypos) * 0.1f;
    lastX = xpos; lastY = ypos;
    yaw += dx; pitch = glm::clamp(pitch + dy, -89.f, 89.f);
    glm::vec3 d;
    d.x = cosf(glm::radians(yaw)) * cosf(glm::radians(pitch));
    d.y = sinf(glm::radians(pitch));
    d.z = sinf(glm::radians(yaw)) * cosf(glm::radians(pitch));
    camFront = glm::normalize(d);
}

// ─────────────────────────────────────────────
//  Main
// ─────────────────────────────────────────────
int main() {
    if (!glfwInit()) { std::cerr << "GLFW init failed\n"; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow* win = glfwCreateWindow(SCR_W, SCR_H, "Solar System Simulation — BTech CG Project", nullptr, nullptr);
    if (!win) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(win);
    glfwSwapInterval(1);
    glfwSetKeyCallback(win, keyCallback);
    glfwSetCursorPosCallback(win, mouseCallback);
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_NORMALIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_SMOOTH);
    glClearColor(0, 0, 0.02f, 1);

    // Lighting – point light at sun (origin)
    glEnable(GL_LIGHTING); glEnable(GL_LIGHT0);
    float amb[]  = {0.05f,0.05f,0.05f,1}; glLightfv(GL_LIGHT0, GL_AMBIENT,  amb);
    float diff[] = {1.0f, 0.95f,0.8f, 1}; glLightfv(GL_LIGHT0, GL_DIFFUSE,  diff);
    float spec[] = {0.5f, 0.5f, 0.5f, 1}; glLightfv(GL_LIGHT0, GL_SPECULAR, spec);
    float lpos[] = {0,0,0,1};             glLightfv(GL_LIGHT0, GL_POSITION, lpos);

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    float ms[] = {0.3f,0.3f,0.3f,1}; glMaterialfv(GL_FRONT, GL_SPECULAR, ms);
    glMaterialf(GL_FRONT, GL_SHININESS, 32.f);

    initPlanets();

    std::cout << "\n Controls: W/S/A/D = fly | Q/E = up/down | Mouse = look\n";
    std::cout << "           +/- = speed | P = pause | R = reset | ESC = quit\n\n";

    while (!glfwWindowShouldClose(win)) {
        double now = glfwGetTime();
        float  dt  = (float)(now - lastFrame); lastFrame = now;

        update(dt);

        int fw, fh; glfwGetFramebufferSize(win, &fw, &fh);
        glViewport(0, 0, fw, fh);

        glMatrixMode(GL_PROJECTION); glLoadIdentity();
        gluPerspective(60.0, (double)fw/fh, 0.5, 1200.0);

        glMatrixMode(GL_MODELVIEW); glLoadIdentity();
        glm::mat4 view = glm::lookAt(camPos, camPos + camFront, camUp);
        glLoadMatrixf(glm::value_ptr(view));

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawStarfield();
        drawSun();
        for (auto& p : planets) drawPlanet(p);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
