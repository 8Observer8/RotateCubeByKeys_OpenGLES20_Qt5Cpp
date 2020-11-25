// Add this line to .pro:
// win32: LIBS += -lopengl32

#ifdef _WIN32
#include <windows.h>
extern "C" __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
extern "C" __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;
#endif

#include <QtWidgets/QApplication>
#include <QtWidgets/QOpenGLWidget>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QKeyEvent>
#include <QtGui/QMatrix4x4>

class OpenGLWidget : public QOpenGLWidget {
    Q_OBJECT
public:
    OpenGLWidget(QWidget *parent = nullptr)
        : QOpenGLWidget(parent)
        , m_indexBuffer(QOpenGLBuffer::IndexBuffer)
    {
        setWindowTitle("Qt C++, OpenGL");
        resize(268, 268);
    }
private:
    QOpenGLShaderProgram m_program;
    QOpenGLBuffer m_vertPosBuffer;
    QOpenGLBuffer m_normalBuffer;
    QOpenGLBuffer m_indexBuffer;
    QMatrix4x4 m_projMatrix;
    QMatrix4x4 m_viewMatrix;
    QMatrix4x4 m_modelMatrix;
    int m_amountOfVertices;
    const float m_ANGLE_STEP = 3.f;
    float m_angle = 0.f;

    void initializeGL() override {
        glClearColor(0.5f, 0.8f, 0.7f, 1.f);
        glEnable(GL_DEPTH_TEST);
        const char *vertShaderSrc =
                "attribute vec3 aPosition;"
                "attribute vec4 aNormal;"
                "uniform mat4 uMvpMatrix;"
                "uniform mat4 uNormalMatrix;"
                "varying vec4 vColor;"
                "void main()"
                "{"
                "    gl_Position = uMvpMatrix * vec4(aPosition, 1.0);"
                "    vec3 lightDirection = normalize(vec3(0.0, 0.5, 0.7));"
                "    vec4 color = vec4(1.0, 0.4, 0.0, 1.0);"
                "    vec3 normal = normalize((uNormalMatrix * aNormal).xyz);"
                "    float nDotL = max(dot(normal, lightDirection), 0.0);"
                "    vColor = vec4(color.rgb * nDotL + vec3(0.1), color.a);"
                "}";
        const char *fragShaderSrc =
                "varying vec4 vColor;"
                "void main()"
                "{"
                "    gl_FragColor = vColor;"
                "}";
        m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vertShaderSrc);
        m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragShaderSrc);
        m_program.link();
        m_program.bind();
        m_amountOfVertices = initVertexBuffers();
        m_viewMatrix.lookAt(
                    QVector3D(20.f, 15.f, 30.f), // eye position
                    QVector3D(0.f, 0.f, 0.f),    // center
                    QVector3D(0.f, 1.f, 0.f));   // up
    }
    void paintGL() override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_modelMatrix.setToIdentity();
        m_modelMatrix.translate(QVector3D(0.f, 0.f, 0.f));
        m_modelMatrix.rotate(m_angle, QVector3D(0.f, 1.f, 0.f));
        m_modelMatrix.scale(7.f, 7.f, 7.f);
        drawBox();
    }
    void resizeGL(int w, int h) override {
        glViewport(0, 0, w, h);
        m_projMatrix.setToIdentity();
        m_projMatrix.perspective(50.f, (float)width()/height(), 0.1f, 100.f);
        m_program.bind();
        m_program.setUniformValue("uMvpMatrix", m_projMatrix * m_viewMatrix);
    }
    void keyPressEvent(QKeyEvent *e) override {
        switch (e->key()) {
            case Qt::Key_Left:
            case Qt::Key_A:
                m_angle = (int)(m_angle + m_ANGLE_STEP) % 360;
                break;
            case Qt::Key_Right:
            case Qt::Key_D:
                m_angle = (int)(m_angle - m_ANGLE_STEP) % 360;
                break;
        }
        update();
    }
    int initVertexBuffers() {
        // Create a cube
        //    v6----- v5
        //   /|      /|
        //  v1------v0|
        //  | |     | |
        //  | |v7---|-|v4
        //  |/      |/
        //  v2------v3
        float vertPositions[] = {
            // v0-v1-v2-v3 front
            1.f, 1.f, 1.f, -1.f, 1.f, 1.f, -1.f, -1.f, 1.f, 1.f, -1.f, 1.f,
            // v0-v3-v4-v5 right
            1.f, 1.f, 1.f, 1.f, -1.f, 1.f, 1.f, -1.f, -1.f, 1.f, 1.f, -1.f,
            // v0-v5-v6-v1 up
            1.f, 1.f, 1.f, 1.f, 1.f, -1.f, -1.f, 1.f, -1.f, -1.f, 1.f, 1.f,
            // v1-v6-v7-v2 left
           -1.f, 1.f, 1.f, -1.f, 1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f, 1.f,
            // v7-v4-v3-v2 down
           -1.f, -1.f, -1.f, 1.f, -1.f, -1.f, 1.f, -1.f, 1.f, -1.f, -1.f, 1.f,
            // v4-v7-v6-v5 back
            1.f, -1.f, -1.f, -1.f, -1.f, -1.f, -1.f, 1.f, -1.f, 1.f, 1.f, -1.f
        };
        m_vertPosBuffer.create();
        m_vertPosBuffer.bind();
        m_vertPosBuffer.allocate(vertPositions, sizeof(vertPositions));
        m_program.bindAttributeLocation("aPosition", 0);
        m_program.setAttributeBuffer(0, GL_FLOAT, 0, 3);
        m_program.enableAttributeArray(0);
        float normals[] = {
            // v0-v1-v2-v3 front
            0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f,
            // v0-v3-v4-v5 right
            1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f,
            // v0-v5-v6-v1 up
            0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f,
            // v1-v6-v7-v2 left
            -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f,
            // v7-v4-v3-v2 down
            0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f,
            // v4-v7-v6-v5 back
            0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f
        };
        m_normalBuffer.create();
        m_normalBuffer.bind();
        m_normalBuffer.allocate(normals, sizeof(normals));
        m_program.bindAttributeLocation("aNormal", 1);
        m_program.setAttributeBuffer(1, GL_FLOAT, 0, 3);
        m_program.enableAttributeArray(1);
        int indices[] = {
            0, 1, 2, 0, 2, 3,           // front
            4, 5, 6, 4, 6, 7,           // right
            8, 9, 10, 8, 10, 11,        // up
            12, 13, 14, 12, 14, 15,     // left
            16, 17, 18, 16, 18, 19,     // down
            20, 21, 22, 20, 22, 23      // back
        };
        m_indexBuffer.create();
        m_indexBuffer.bind();
        m_indexBuffer.allocate(indices, sizeof(indices));

        int amountOfVertices = sizeof(indices) / sizeof(indices[0]);
        return amountOfVertices;
    }
    void drawBox() {
        QMatrix4x4 mvpMatrix = m_projMatrix * m_viewMatrix * m_modelMatrix;
        m_program.bind();
        m_program.setUniformValue("uMvpMatrix", mvpMatrix);
        QMatrix4x4 normalMatrix;
        normalMatrix = m_modelMatrix.inverted();
        normalMatrix = normalMatrix.transposed();
        m_program.setUniformValue("uNormalMatrix", normalMatrix);
        m_vertPosBuffer.bind();
        m_program.setAttributeBuffer(0, GL_FLOAT, 0, 3);
        m_program.enableAttributeArray(0);
        m_normalBuffer.bind();
        m_program.setAttributeBuffer(1, GL_FLOAT, 0, 3);
        m_program.enableAttributeArray(1);
        glDrawElements(GL_TRIANGLES, m_amountOfVertices, GL_UNSIGNED_INT, nullptr);
    }
};

#include "main.moc"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    OpenGLWidget w;
    w.show();
    return a.exec();
}
