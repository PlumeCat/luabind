// glfw.h

// #include "../../glfw/include/GLFW/glfw3.h"

struct GLFWmonitor;
struct GLFWwindow;
struct GLFWimage;
struct GLFWwindowposfun;

void glfwDefaultWindowHints (void);
void glfwWindowHint (int hint, int value);
void glfwWindowHintString (int hint, const char *value);
GLFWwindow * glfwCreateWindow (int width, int height, const char *title, GLFWmonitor *monitor, GLFWwindow *share);
void glfwDestroyWindow (GLFWwindow *window);
int glfwWindowShouldClose (GLFWwindow *window);
void glfwSetWindowShouldClose (GLFWwindow *window, int value);
void glfwSetWindowTitle (GLFWwindow *window, const char *title);
void glfwSetWindowIcon (GLFWwindow *window, int count, const GLFWimage *images);
void glfwGetWindowPos (GLFWwindow *window, int *xpos, int *ypos);
void glfwSetWindowPos (GLFWwindow *window, int xpos, int ypos);
void glfwGetWindowSize (GLFWwindow *window, int *width, int *height);
void glfwSetWindowSizeLimits (GLFWwindow *window, int minwidth, int minheight, int maxwidth, int maxheight);
void glfwSetWindowAspectRatio (GLFWwindow *window, int numer, int denom);
void glfwSetWindowSize (GLFWwindow *window, int width, int height);
void glfwGetFramebufferSize (GLFWwindow *window, int *width, int *height);
void glfwGetWindowFrameSize (GLFWwindow *window, int *left, int *top, int *right, int *bottom);
void glfwGetWindowContentScale (GLFWwindow *window, float *xscale, float *yscale);
float glfwGetWindowOpacity (GLFWwindow *window);
void glfwSetWindowOpacity (GLFWwindow *window, float opacity);
void glfwIconifyWindow (GLFWwindow *window);
void glfwRestoreWindow (GLFWwindow *window);
void glfwMaximizeWindow (GLFWwindow *window);
void glfwShowWindow (GLFWwindow *window);
void glfwHideWindow (GLFWwindow *window);
void glfwFocusWindow (GLFWwindow *window);
void glfwRequestWindowAttention (GLFWwindow *window);
GLFWmonitor * glfwGetWindowMonitor (GLFWwindow *window);
void glfwSetWindowMonitor (GLFWwindow *window, GLFWmonitor *monitor, int xpos, int ypos, int width, int height, int refreshRate);
int glfwGetWindowAttrib (GLFWwindow *window, int attrib);
void glfwSetWindowAttrib (GLFWwindow *window, int attrib, int value);
void glfwSetWindowUserPointer (GLFWwindow *window, void *pointer);
void * glfwGetWindowUserPointer (GLFWwindow *window);
// GLFWwindowposfun glfwSetWindowPosCallback (GLFWwindow *window, GLFWwindowposfun callback);
// GLFWwindowsizefun glfwSetWindowSizeCallback (GLFWwindow *window, GLFWwindowsizefun callback);
// GLFWwindowclosefun glfwSetWindowCloseCallback (GLFWwindow *window, GLFWwindowclosefun callback);
// GLFWwindowrefreshfun glfwSetWindowRefreshCallback (GLFWwindow *window, GLFWwindowrefreshfun callback);
// GLFWwindowfocusfun glfwSetWindowFocusCallback (GLFWwindow *window, GLFWwindowfocusfun callback);
// GLFWwindowiconifyfun glfwSetWindowIconifyCallback (GLFWwindow *window, GLFWwindowiconifyfun callback);
// GLFWwindowmaximizefun glfwSetWindowMaximizeCallback (GLFWwindow *window, GLFWwindowmaximizefun callback);
// GLFWframebuffersizefun glfwSetFramebufferSizeCallback (GLFWwindow *window, GLFWframebuffersizefun callback);
// GLFWwindowcontentscalefun glfwSetWindowContentScaleCallback (GLFWwindow *window, GLFWwindowcontentscalefun callback);
void glfwPollEvents (void);
void glfwWaitEvents (void);
void glfwWaitEventsTimeout (double timeout);
void glfwPostEmptyEvent (void);
void glfwSwapBuffers (GLFWwindow *window);