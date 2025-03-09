#include <glad/glad.h> // have to be included before glfw3.h
#include <glm/ext/vector_float3.hpp>

#include "load_obj.h"
#include "setShaders.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/gl.h> // also included in glad
#include <GLFW/glfw3.h> // also included in glad

#include <assert.h>
#include <cstdlib>
#include <stdio.h> // fuck u iostream
#include <string.h>
#include <strings.h>
#include <vector>

#define BG_COLOR glHexColor(0x87CEEB), 1.0

#define WIDTH 640
#define HEIGHT 480

unsigned int c_lock = 0;

float pos_y = 24.0f;
float pos_x = 4.0f;

/* Global to made easy share it among funtions */

enum {
        VIEW_1_PERSON,
        VIEW_3_PERSON,
} VIEW = VIEW_3_PERSON;

struct Object {
        GLuint shader_program;
        GLuint texture;
        GLuint vao;
        GLuint indexes_n;
        GLuint printable;
        glm::vec3 position;
        int apply_movement;
        float rotation;
        float rotationZ;
};

std::vector<struct Object> objects;

int mainloop(GLFWwindow *window);

void
__load_texture(const char *filename, GLuint *texture)
{
        int w;
        int h;
        int comp;
        unsigned char *image;

        stbi_set_flip_vertically_on_load(1);

        image = stbi_load(filename, &w, &h, &comp, STBI_rgb_alpha);

        if (image == NULL) {
                fprintf(stderr, "Failed to load texture\n");
                return;
        }

        glGenTextures(1, texture);
        glBindTexture(GL_TEXTURE_2D, *texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(image);
}

GLuint
load_ground()
{
        GLuint VAO, VBO, EBO;
        const GLuint size = 100;
        const GLuint tex_scale = 6;
        /* Sets the polygon rasterization mode for rendering.
         * This determines how OpenGL draws polygons.
         *
         * Options for `face`:
         * - `GL_FRONT`: Applies the mode to front-facing polygons only.
         * - `GL_BACK`: Applies the mode to back-facing polygons only.
         * - `GL_FRONT_AND_BACK`: Applies the mode to both front and back faces.
         *
         * Options for `mode`:
         * - `GL_POINT`: Renders only the vertices as points.
         * - `GL_LINE`: Renders only the edges as wireframes.
         * - `GL_FILL`: Renders solid, filled polygons (default).  */
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        float vertices[] = {
                -(size / 2.0f), 0.0f, -(size / 2.0f),
                0.0f,           0.0f, //
                -(size / 2.0f), 0.0f, (size / 2.0f),
                0.0f,           tex_scale, //
                (size / 2.0f),  0.0f, (size / 2.0f),
                tex_scale,           tex_scale, //
                (size / 2.0f),  0.0f, -(size / 2.0f),
                tex_scale,           0.0f //

                /* 3 ---- 2
                 * |      |
                 * |      |
                 * 0 ---- 1 */
        };
        unsigned int indices[] = {
                0, 1, 2, // triangulo 1
                2, 3, 0 // triangulo 2
        };

        /* ----[ VAO ]---- */
        /* Generates a Vertex Array Object (VAO) and binds it.
         * - VAO stores vertex attribute configurations and
         *   buffer bindings. It must be bound before
         *   configuring any vertex attributes. */
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        /* ----[ VBO ]---- */
        /* Generates a Vertex Buffer Object (VBO) and binds it.
         * - VBO stores the vertex data in GPU memory.
         * - `GL_ARRAY_BUFFER`: Specifies that this buffer
         *   holds vertex attributes. */
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        /* Allocates and copies vertex data into the VBO.
         * - `sizeof(vertices)`: Size of the data in bytes.
         * - `vertices`: Pointer to the vertex data.
         * - `GL_STATIC_DRAW`: Data is set once and used
         *   many times (optimized for performance). */
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        /* ----[ EBO ]---- */
        /* Generates an Element Buffer Object (EBO) and binds it.
         * - EBO stores indices that define the order of vertices.
         * - `GL_ELEMENT_ARRAY_BUFFER`: Specifies that this buffer
         *   holds indices for indexed drawing. */
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        /* Allocates and copies index data into the EBO.
         * - `sizeof(indices)`: Size of the index data in bytes.
         * - `indices`: Pointer to the index data.
         * - `GL_STATIC_DRAW`: Data is set once and used
         *   many times (optimized for performance). */
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        /* ----[ ~~~ ]---- */

        /*
         * Defines how OpenGL should interpret the vertex
         * data.
         * - `0`: Attribute location in the shader
         *   (layout location 0).
         * - `3`: Number of components per vertex (x, y, z).
         * - `GL_FLOAT`: Data type of each component.
         * - `GL_FALSE`: No normalization.
         * - `3 * sizeof(float)`: Stride (distance between
         *   consecutive vertices).
         * - `(void *)0`: Offset in the buffer
         *   (starts at the beginning).
         */
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);

        /* Enables the vertex attribute at location 0.
         * This allows OpenGL to use the vertex data
         * when rendering. */
        glEnableVertexAttribArray(0);

        /* Texture */
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                              (void *) (3 * sizeof(float)));

        /* Enables the vertex attribute at location 0.
         * This allows OpenGL to use the vertex data
         * when rendering. */
        glEnableVertexAttribArray(1);

        /* Unbinds the VBO to prevent accidental
         * modification. Not neccesary */
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        /* Unbinds the VAO to avoid modifying it
         * unintentionally. This is good practice when
         * working with multiple VAOs. */
        glBindVertexArray(0);

        return VAO;
}


/* This funtion is going to be executed in main loop
 * to catch input and do whatever is needed */
void
__process_input(GLFWwindow *window)
{
        const float moveSpeed = 0.1f;
        const float cameraSpeed = 0.1f;
        const float rotateSpeed = 2.0f;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                glfwSetWindowShouldClose(window, true);

        float radians = glm::radians(objects[1].rotation);
        float dx = glm::sin(radians) * moveSpeed;
        float dz = glm::cos(radians) * moveSpeed * -1;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                for (size_t i = 1; i < objects.size(); ++i) {
                        objects[i].position.x += dx;
                        objects[i].position.z -= dz;
                }
        }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                for (size_t i = 1; i < objects.size(); ++i) {
                        objects[i].position.x -= dx;
                        objects[i].position.z += dz;
                }
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
                for (Object &obj : objects) {
                        obj.rotation += rotateSpeed;
                }
        }

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                for (Object &obj : objects) {
                        obj.rotation -= rotateSpeed;
                }
        }

        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
                for (size_t i = 2; i < objects.size(); ++i) {
                        objects[i].rotation += rotateSpeed;
                }
        }

        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
                for (size_t i = 2; i < objects.size(); ++i) {
                        objects[i].rotation -= rotateSpeed;
                }
        }

        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
                if (objects[2].rotationZ > -16)
                        objects[2].rotationZ -= rotateSpeed;
        }

        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
                if (objects[2].rotationZ < 26)
                        objects[2].rotationZ += rotateSpeed;
        }

        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !c_lock) {
                if (VIEW == VIEW_1_PERSON)
                        VIEW = VIEW_3_PERSON;
                else
                        VIEW = VIEW_1_PERSON;
                c_lock = 1;
        }

        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE && c_lock)
                c_lock = 0;

        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
                pos_y += cameraSpeed;
        }

        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
                pos_y -= cameraSpeed;
        }

        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
                pos_x += cameraSpeed;
        }

        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
                pos_x -= cameraSpeed;
        }

        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
                for (struct Object &obj : objects)
                        obj.printable = 1;

#define __add_num(num)                                                                     \
        if (glfwGetKey(window, GLFW_KEY_0 + num) == GLFW_PRESS && objects.size() >= num) { \
                for (struct Object & obj : objects)                                        \
                        obj.printable = 0;                                                 \
                objects[num - 1].printable = 1;                                            \
        }

        __add_num(1);
        __add_num(2);
        __add_num(3);
        __add_num(4);
        __add_num(5);
        __add_num(6);
        __add_num(7);
        __add_num(8);
        __add_num(9);
}

/* This function is called on window resize */
void
__framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
        // make sure the viewport matches the new window dimensions; note that width and
        // height will be significantly larger than specified on retina displays.
        glViewport(0, 0, width, height);
}

/* Entry point: Initialize stuff and then enter mainloop. */
int
main(int argc, char **argv)
{
        /* ---- Init GLFW ---- */
        if (!glfwInit()) {
                printf("Can not init glfw\n");
                return 1;
        }
        // just info that I dont know if it is needed
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        // TODO: Creo que me descarque el otro
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        /* ---- Create the main window ---- */
        // GLFWmonitor *monitor = glfwGetPrimaryMonitor(); // fullscreen
        GLFWmonitor *monitor = NULL; // floating (or not)
        //  Share = NULL
        GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Titulo", monitor, NULL);

        if (window == NULL) {
                perror("glfwCreateWindow");
                glfwTerminate(); // terminate initialized glfw
                return 1;
        }

        /* @brief Makes the context of the specified window
         * current for the calling thread.
         * This function makes the OpenGL or OpenGL ES
         * context of the specified window current on the
         * calling thread. It can also detach the current
         * context from the calling thread without making a
         * new one current by passing in `NULL`. */
        glfwMakeContextCurrent(window);

        /* Call __framebuffer_size_callback() on window resize */
        glfwSetFramebufferSizeCallback(window, __framebuffer_size_callback);

        /* Load the GLAD. IDK what is this */
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
                perror("gladLoadGLLoader");
                /* In main1_33a.cpp it just return but I think
                 * it is needed to call glfwTerminate(). */
                return 1;
        }

        // z-buffer value
        glClearDepth(1.0f);
        // clear buffer color
        glClearColor(BG_COLOR);
        // enable z-buffer
        glEnable(GL_DEPTH_TEST);
        // hide back faces
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        // do not hide
        // glDisable(GL_CULL_FACE);

        /* Sets the polygon rasterization mode for rendering.
         * This determines how OpenGL draws polygons.
         *
         * Options for `face`:
         * - `GL_FRONT`: Applies the mode to front-facing polygons only.
         * - `GL_BACK`: Applies the mode to back-facing polygons only.
         * - `GL_FRONT_AND_BACK`: Applies the mode to both front and back faces.
         *
         * Options for `mode`:
         * - `GL_POINT`: Renders only the vertices as points.
         * - `GL_LINE`: Renders only the edges as wireframes.
         * - `GL_FILL`: Renders solid, filled polygons (default).  */
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        unsigned int *VAO_ARR = NULL;
        unsigned int VAO_ARR_SIZE = 0;
        unsigned int *indexes_size_arr = NULL;

        printf("Loading ./untitled.obj\n");
        load_obj("./untitled.obj", &VAO_ARR, &VAO_ARR_SIZE, &indexes_size_arr, LOAD_3_3);

        assert(VAO_ARR);
        assert(VAO_ARR_SIZE);

        /* Program shader */
        GLuint shader_program = setShaders("vertex_shader_texture.glsl",
                                           "fragment_shader_texture.glsl");

        GLuint texture;
        __load_texture("./DirtWeeds/GroundDirtWeedsPatchy004_COL_2K.jpg", &texture);

        // ground
        objects.push_back((struct Object) {
        .shader_program = shader_program,
        .texture = texture,
        .vao = load_ground(),
        .indexes_n = 6,
        .printable = 1,
        .position = glm::vec3(0, 0, 0),
        .apply_movement = 0,
        .rotation = 0.0f,
        .rotationZ = 0.0f,
        });

        shader_program =
        setShaders("vertex_shader_solid.glsl", "fragment_shader_solid.glsl");

        for (int i = 0; i < VAO_ARR_SIZE; i++) {
                objects.push_back((struct Object) {
                .shader_program = shader_program,
                .texture = 0,
                .vao = VAO_ARR[i],
                .indexes_n = indexes_size_arr[i],
                .printable = 1,
                .position = glm::vec3(0, 0, 0),
                .apply_movement = 1,
                .rotation = 0.0f,
                .rotationZ = 0.0f,
                });
        }

        free(VAO_ARR);
        free(indexes_size_arr);

        printf(".obj loaded\n");
        printf("%zd vaos loaded\n", objects.size());

        mainloop(window);
        glfwDestroyWindow(window);
        glfwTerminate();

        return 0;
}

/* Main loop. Executed until program is closed manually. */
int
mainloop(GLFWwindow *window)
{
        unsigned int VAO;

        /* Execute until window is closed */
        while (!glfwWindowShouldClose(window)) {
                // Call our process input function
                __process_input(window);

                // Clear buffers
                glClearColor(BG_COLOR); // BG
                glClear(GL_COLOR_BUFFER_BIT);
                glClear(GL_DEPTH_BUFFER_BIT);


                glm::vec3 cameraEye;
                glm::vec3 cameraPosition;
                glm::vec3 cameraUp;
                glm::mat4 view;

                if (VIEW == VIEW_3_PERSON) {
                        cameraEye = glm::vec3(0, 0, 0);
                        cameraPosition = glm::vec3(0, 0, 0);
                        cameraPosition.x += pos_x;
                        cameraPosition.y += pos_y;
                        cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

                        view = glm::lookAt(cameraPosition, cameraEye, cameraUp);


                } else if (VIEW == VIEW_1_PERSON) {
                        // Obtenemos la matriz de rotacion del objeto 3
                        glm::mat4 model = glm::mat4(1.0f);
                        model = glm::rotate(model, glm::radians(objects[4].rotation),
                                            glm::vec3(0.0f, 1.0f, 0.0f));

                        glm::vec3 dirf = glm::normalize(glm::vec3(model[2])); // La tercera columna es el eje Z (dirección frontal)
                        cameraEye = objects[4].position;
                        cameraPosition =
                        cameraEye - dirf * pos_x + glm::vec3(0.0f, pos_y, 0.0f);
                        cameraUp = glm::vec3(model[1]); // Segunda columna es el eje Y (arriba)
                        view = glm::lookAt(cameraPosition, cameraEye, cameraUp);
                }


                for (struct Object &obj : objects) {
                        VAO = obj.vao;

                        if (obj.printable == 0)
                                continue;

                        // Use shader program
                        glUseProgram(obj.shader_program);

                        GLuint viewLoc = glGetUniformLocation(obj.shader_program, "view");
                        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
                        glm::mat4 projection =
                        glm::perspective(glm::radians(45.0f),
                                         (float) WIDTH / (float) HEIGHT, 0.1f, 100.0f);
                        GLuint projectionLoc =
                        glGetUniformLocation(obj.shader_program, "projection");
                        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE,
                                           glm::value_ptr(projection));


                        glm::mat4 model = glm::mat4(1.0f);

                        if (obj.apply_movement) {
                                model = glm::translate(model, obj.position);
                                model = glm::rotate(model, glm::radians(obj.rotation),
                                                    glm::vec3(0.0f, 1.0f, 0.0f));
                                model = glm::rotate(model, glm::radians(obj.rotationZ),
                                                    glm::vec3(1.0f, 0.0f, 0.0f));
                        }

                        GLuint modelLoc = glGetUniformLocation(obj.shader_program, "model");
                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

                        if (obj.texture > 0) {
                                glEnable(GL_TEXTURE_2D);
                                glBindTexture(GL_TEXTURE_2D, obj.texture);
                        } else {
                                glDisable(GL_TEXTURE_2D);
                        }

                        glBindVertexArray(obj.vao);
                        glDrawElements(GL_TRIANGLES, obj.indexes_n, GL_UNSIGNED_INT, 0);

                        /* Binds the specified Vertex Array Object
                         * (VAO).
                         * This ensures that subsequent vertex
                         * operations use the correct VAO
                         * configuration.
                         * Without this, OpenGL wouldn't know which
                         * vertex data to use. */
                        glBindVertexArray(VAO);

                        /* Renders primitives (lines, triangles, etc.)
                         * using vertex data in order.
                         * - `GL_LINES`: Draws lines, each pair of
                         *   vertices forms a line.
                         * - `0`: Starts from the first vertex in
                         *   the buffer.
                         * - `6`: Number of vertices to process
                         *   (draws 3 lines).
                         * Use this when you don't need indexed
                         * drawing. */
                        // glDrawArrays(GL_LINES, 0, 6);
                        // glDrawArrays(GL_TRIANGLES, 0, 6); // for square

                        /* Renders primitives using indexed drawing
                         * with an Element Buffer Object (EBO).
                         * - `GL_LINES`: Draws lines using the
                         *   specified indices.
                         * - `6`: Number of indices to read from the
                         *   EBO.
                         * - `GL_UNSIGNED_INT`: Type of the indices
                         *   in the EBO.
                         * - `0`: Start at the beginning of the EBO.
                         * Use this when vertices are reused to
                         * optimize memory usage. */
                        // glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, 0);

                        assert(VAO);
                        assert(obj.indexes_n > 0);

                        glDrawElements(GL_TRIANGLES, obj.indexes_n, GL_UNSIGNED_INT, 0);

                        /* Unbinds the currently active VAO.
                         * This prevents unintended modifications to
                         * the VAO.
                         * It's a good practice when working with
                         * multiple VAOs. */
                        glBindVertexArray(0);
                }

                /* @brief Swaps the front and back buffers of
                 * the specified window.
                 * This function swaps the front and back
                 * buffers of the specified window when
                 * rendering with OpenGL or OpenGL ES.
                 * If the swap interval is greater than zero,
                 * the GPU driver waits the specified number of
                 * screen updates before swapping the buffers. */
                /* ~~ load new frame */
                glfwSwapBuffers(window);

                /* @brief Processes all pending events.
                 * This function processes only those
                 * events that are already in the event
                 * queue and then returns immediately.
                 * Processing events will cause the
                 * window and input callbacks associated
                 * with those events to be called. */
                /* It does not block */
                glfwPollEvents();
        }

        /* Deletes the specified Vertex Array Object (VAO).
         * This frees the GPU memory associated with the VAO.
         * It's important to call this when the VAO is no
         * longer needed to avoid memory leaks.
         */

        for (struct Object &obj : objects) {
                glDeleteVertexArrays(1, &obj.vao);
        }


        return 0;
}
