#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <chrono>

#include <SDL2/SDL.h>

class V3
{
    public:
        float x;
        float y;
        float z;

        V3()
        {
            this->x = 0;
            this->y = 0;
            this->z = 0;
        };

        V3(float x, float y, float z){
            this->x = x;
            this->y = y;
            this->z = z;
        }
        
        V3 operator + (V3 const &v)
        {
            return V3(this->x + v.x, this->y + v.y, this->z + v.z);
        };

        V3 operator - (V3 const &v)
        {
            return V3(this->x - v.x, this->y - v.y, this->z - v.z);
        };

        // flip
        V3 operator - ()
        {
            return V3(-this->x, -this->y, -this->z);
        };

        // scalar multiply
        V3 operator * (float s)
        {
            return V3(this->x * s, this->y * s, this->z * s);
        };

        // dot product
        float operator * (V3 v)
        {
            return this->x * v.x + this->y * v.y + this->z * v.z;
        };

        // piecewise multiply
        V3 operator & (V3 v)
        {
            return V3(this->x * v.x, this->y * v.y, this->z * v.z);
        };

        // cross product
        V3 operator ^ (V3 v)
        {
            return V3(
                this->y * v.z - this->z * v.y,
                this->x * v.z - this->z * v.x,
                this->x * v.y - this->y * v.x
            );
        };

        // normal
        V3 operator ~ ()
        {
            float m = sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
            return *this * (1 / m);
        };

        V3 rotate(float cosX, float sinX, float cosY, float sinY, float cosZ, float sinZ)
        {
            V3 rotated = V3(this->x, this->y, this->z);
            float temp;

            // y axis
            temp = rotated.x * cosY + rotated.z * sinY;
            rotated.z = -rotated.x  * sinY + rotated.z * cosY;
            rotated.x = temp;

            // x axis
            temp = rotated.y * cosX - rotated.z * sinX;
            rotated.z = rotated.y * sinX + rotated.z * cosX;
            rotated.y = temp;

            // z axis
            temp = rotated.x * cosZ - rotated.y * sinZ;
            rotated.y = rotated.x * sinZ + rotated.y * cosZ;
            rotated.x = temp;

            return rotated;
        };

        std::string toString()
        {
            return "<" + std::to_string(this->x) + ", " + std::to_string(this->y) + ", " + std::to_string(this->z) + ">";
        };
};

class Transform
{
    public:
        V3 pos;
        V3 scale;
        V3 rot;

        float cosX;
        float sinX;
        float cosY;
        float sinY;
        float cosZ;
        float sinZ;

        void setRotX(float rotX)
        {
            this->rot.x = rotX;
            this->cosX = cos(rotX);
            this->sinX = sin(rotX);
        };

        void setRotY(float rotY)
        {
            this->rot.y = rotY;
            this->cosY = cos(rotY);
            this->sinY = sin(rotY);
        };

        void setRotZ(float rotZ)
        {
            this->rot.z = rotZ;
            this->cosZ = cos(rotZ);
            this->sinZ = sin(rotZ);
        };

        void changeRotX(float deltaRotX)
        {
            this->setRotX(this->rot.x + deltaRotX);
        };

        void changeRotY(float deltaRotY)
        {
            this->setRotY(this->rot.y + deltaRotY);
        };

        void changeRotZ(float deltaRotZ)
        {
            this->setRotZ(this->rot.z + deltaRotZ);
        };

        void setRot(float rotX, float rotY, float rotZ)
        {
            this->setRotX(rotX);
            this->setRotY(rotY);
            this->setRotZ(rotZ);
        };

        Transform()
        {
            this->pos = V3();
            this->scale = V3(1, 1, 1);
            this->setRot(0, 0, 0);
        };

        Transform(V3 pos, V3 scale, V3 rot)
        {
            this->pos = pos;
            this->scale = scale;
            this->setRot(rot.x, rot.y, rot.z);
        };
};

class Primitive
{
    public:
        V3 p1;
        V3 p2;
        V3 p3;
        bool cullable;
        V3 ambientColor;
        V3 diffuseColor;

        Primitive()
        {
            this->p1 = V3();
            this->p2 = V3();
            this->p3 = V3();
            this->cullable = false;
            this->ambientColor = V3();
            this->diffuseColor = V3();
        };

        Primitive(V3 p1, V3 p2, V3 p3, bool cullable, V3 ambientColor, V3 diffuseColor)
        {
            this->p1 = p1;
            this->p2 = p2;
            this->p3 = p3;
            this->cullable = cullable;
            this->ambientColor = ambientColor;
            this->diffuseColor = diffuseColor;
        };

        Primitive transformGeometry(Transform transform, bool rotateFirst)
        {
            if(rotateFirst){
                return Primitive(
                    (this->p1 & transform.scale).rotate(transform.cosX, transform.sinX, transform.cosY, transform.sinY, transform.cosZ, transform.sinZ) + transform.pos,
                    (this->p2 & transform.scale).rotate(transform.cosX, transform.sinX, transform.cosY, transform.sinY, transform.cosZ, transform.sinZ) + transform.pos,
                    (this->p3 & transform.scale).rotate(transform.cosX, transform.sinX, transform.cosY, transform.sinY, transform.cosZ, transform.sinZ) + transform.pos,
                    this->cullable,
                    this->ambientColor,
                    this->diffuseColor
                );
            } else {
                return Primitive(
                    ((this->p1 & transform.scale) + transform.pos).rotate(transform.cosX, transform.sinX, transform.cosY, transform.sinY, transform.cosZ, transform.sinZ),
                    ((this->p2 & transform.scale) + transform.pos).rotate(transform.cosX, transform.sinX, transform.cosY, transform.sinY, transform.cosZ, transform.sinZ),
                    ((this->p3 & transform.scale) + transform.pos).rotate(transform.cosX, transform.sinX, transform.cosY, transform.sinY, transform.cosZ, transform.sinZ),
                    this->cullable,
                    this->ambientColor,
                    this->diffuseColor
                );
            };
        };

        void print()
        {
            std::cout << "Primitive: " << std::endl << "\t" << this->p1.toString() << std::endl << "\t" << this->p2.toString() << std::endl << "\t" << this->p3.toString() << std::endl << std::endl;
        };
};

class SceneObject
{
    public:
        std::vector<Primitive> primitives;
        Transform internalTransform;

        SceneObject()
        {
            this->primitives = {};
            this->internalTransform = Transform();
        };

        SceneObject(std::vector<Primitive> primitives, Transform internalTransform)
        {
            this->primitives = primitives;
            this->internalTransform = internalTransform;
        };

        SceneObject transformGeometry(Transform transform, bool rotateFirst)
        {
            std::vector<Primitive> transformedPrimitives;
            transformedPrimitives.reserve(this->primitives.size());
            for(int i = 0;i<this->primitives.size();i++)
            {
                transformedPrimitives.push_back(this->primitives[i].transformGeometry(transform, rotateFirst));
            };
            return SceneObject(transformedPrimitives, this->internalTransform);
        };

        static SceneObject ColoredUnitCube(V3 pos)
        {
            return SceneObject(
                {
                    // back face
                    Primitive(
                        V3(-1, -1, 1),
                        V3(1, -1, 1),
                        V3(1, 1, 1),
                        true,
                        V3(255, 0, 0),
                        V3(255, 0, 0)
                    ),
                    Primitive(
                        V3(-1, -1, 1),
                        V3(1, 1, 1),
                        V3(-1, 1, 1),
                        true,
                        V3(255, 0, 0),
                        V3(255, 0, 0)
                    ),
                    // right face
                    Primitive(
                        V3(1, -1, 1),
                        V3(1, -1, -1),
                        V3(1, 1, -1),
                        true,
                        V3(0, 0, 255),
                        V3(0, 0, 255)
                    ),
                    Primitive(
                        V3(1, -1, 1),
                        V3(1, 1, -1),
                        V3(1, 1, 1),
                        true,
                        V3(0, 0, 255),
                        V3(0, 0, 255)
                    ),
                    // front face
                    Primitive(
                        V3(1, -1, -1),
                        V3(-1, -1, -1),
                        V3(-1, 1, -1),
                        true,
                        V3(0, 255, 0),
                        V3(0, 255, 0)
                    ),
                    Primitive(
                        V3(1, -1, -1),
                        V3(-1, 1, -1),
                        V3(1, 1, -1),
                        true,
                        V3(0, 255, 0),
                        V3(0, 255, 0)
                    ),
                    // left face
                    Primitive(
                        V3(-1, -1, -1),
                        V3(-1, -1, 1),
                        V3(-1, 1, 1),
                        true,
                        V3(255, 100, 0),
                        V3(255, 100, 0)
                    ),
                    Primitive(
                        V3(-1, -1, -1),
                        V3(-1, 1, 1),
                        V3(-1, 1, -1),
                        true,
                        V3(255, 100, 0),
                        V3(255, 100, 0)
                    ),
                    // bottom face
                    Primitive(
                        V3(-1, -1, -1),
                        V3(1, -1, -1),
                        V3(1, -1, 1),
                        true,
                        V3(255, 0, 255),
                        V3(255, 0, 255)
                    ),
                    Primitive(
                        V3(-1, -1, -1),
                        V3(1, -1, 1),
                        V3(-1, -1, 1),
                        true,
                        V3(255, 0, 255),
                        V3(255, 0, 255)
                    ),
                    // top face
                    Primitive(
                        V3(-1, 1, -1),
                        V3(1, 1, 1),
                        V3(1, 1, -1),
                        true,
                        V3(255, 255, 0),
                        V3(255, 255, 0)
                    ),
                    Primitive(
                        V3(-1, 1, -1),
                        V3(-1, 1, 1),
                        V3(1, 1, 1),
                        true,
                        V3(255, 255, 0),
                        V3(255, 255, 0)
                    )
                },
                Transform(pos, V3(1, 1, 1), V3())
            );
        };
};

class SceneLight
{
    public:
        V3 pos;
        V3 color;
        float strength;

        SceneLight()
        {
            this->pos = V3();
            this->color = V3();
            this->strength = 0;
        };

        SceneLight(V3 pos, V3 color, float strength)
        {
            this->pos = pos;
            this->color = color;
            this->strength = strength;
        };
};

class Scene
{
    public:
        std::vector<SceneObject> objects;
        std::vector<SceneLight> lights;
};

class BufferPixel
{
    public:
        float z;
        V3 color;

        BufferPixel()
        {
            this->z = 10000;
            this->color = V3();
        };

        BufferPixel(float z, V3 color)
        {
            this->z = z;
            this->color = color;
        };
};

class Camera
{
    public:
        V3 pos;
        V3 rot;
        float fov;
        float focal;
        float min;
        float max;
        bool renderProcessLogs = false;

        Camera()
        {
            this->pos = V3();
            this->rot = V3();
            this->fov = 90;
            this->focal = 1;
            this->min = 0;
            this->max = 100;
        };

        Camera(V3 pos, V3 rot, float fov, float focal, float min, float max)
        {
            this->pos = pos;
            this->rot = rot;
            this->fov = fov;
            this->focal = focal;
            this->min = min;
            this->max = max;
        };

        void moveForward(float distance)
        {
            this->pos.x += distance * cos(this->rot.y + M_PI_2);
            this->pos.z += distance * sin(this->rot.y + M_PI_2);
        };

        void moveBackward(float distance)
        {
            this->pos.x -= distance * cos(this->rot.y + M_PI_2);
            this->pos.z -= distance * sin(this->rot.y + M_PI_2);
        };

        void moveLeft(float distance)
        {
            this->pos.x -= distance * cos(this->rot.y);
            this->pos.z -= distance * sin(this->rot.y);
        };

        void moveRight(float distance)
        {
            this->pos.x += distance * cos(this->rot.y);
            this->pos.z += distance * sin(this->rot.y);
        };

        void moveUp(float distance)
        {
            this->pos.y += distance;
        };

        void moveDown(float distance)
        {
            this->pos.y -= distance;
        };

        void log(std::string message)
        {
            if(this->renderProcessLogs){
                std::cout << message << std::endl;
            };
        };

        void render(SDL_Renderer* renderer, int canvasWidth, int canvasHeight, Scene scene)
        {
            this->log("Started Render");

            Transform cameraTransform = Transform(-this->pos, V3(1, 1, 1), this->rot);
            float fovCoefficient = canvasWidth / (this->focal * tan(this->fov * M_PI / 360));

            this->log("Precomp Completed");
            
            std::vector<std::vector<BufferPixel>> buffer;
            buffer.reserve(2 * canvasWidth);
            for(int i = 0;i<2 * canvasWidth;i++)
            {
                buffer.push_back(std::vector<BufferPixel>());
                buffer[i].reserve(2 * canvasHeight);
                for(int j = 0;j<2 * canvasHeight;j++)
                {
                    buffer[i].push_back(BufferPixel(this->max + 1, V3(0, 0, 0)));
                };
            };

            this->log("Init Buffer Completed");

            std::vector<Primitive> primitives;
            for(int i = 0;i<scene.objects.size();i++)
            {
                std::vector<Primitive> objectPrimitives = scene.objects[i].transformGeometry(scene.objects[i].internalTransform, true).transformGeometry(cameraTransform, false).primitives;
                // TODO: look for memory efficient way to concat
                for(int j = 0;j<objectPrimitives.size();j++)
                {
                    if(objectPrimitives[j].p1.z > this->min && objectPrimitives[j].p2.z > this->min && objectPrimitives[j].p3.z > this->min && objectPrimitives[j].p1.z < this->max && objectPrimitives[j].p2.z < this->max && objectPrimitives[j].p3.z < this->max){
                        primitives.push_back(objectPrimitives[j]);

                        // TODO: troubleshoot culling inaccuracy
                        // if(!primitive.cullable){
                        //     primitives.push_back(primitive);
                        // } else if((primitive.p2.x - primitive.p1.x) * (primitive.p3.y - primitive.p1.y) - (primitive.p2.y - primitive.p1.y) * (primitive.p3.x - primitive.p1.x) < 0) {
                        //     primitives.push_back(primitive);
                        // }
                    };
                };
            };

            this->log("Transform Completed");

            for(int i = 0;i<primitives.size();i++)
            {
                primitives[i].p1.x *= fovCoefficient / primitives[i].p1.z;
                primitives[i].p1.y *= fovCoefficient / primitives[i].p1.z;
                primitives[i].p2.x *= fovCoefficient / primitives[i].p2.z;
                primitives[i].p2.y *= fovCoefficient / primitives[i].p2.z;
                primitives[i].p3.x *= fovCoefficient / primitives[i].p3.z;
                primitives[i].p3.y *= fovCoefficient / primitives[i].p3.z;

                primitives[i].p1.x += canvasWidth;
                primitives[i].p1.y = canvasHeight - primitives[i].p1.y;
                primitives[i].p2.x += canvasWidth;
                primitives[i].p2.y = canvasHeight - primitives[i].p2.y;
                primitives[i].p3.x += canvasWidth;
                primitives[i].p3.y = canvasHeight - primitives[i].p3.y;

                // TODO: this is a really confusing way to sort a, b, c
                std::vector<V3> vertices = { primitives[i].p1, primitives[i].p2, primitives[i].p3 };
                int index = 0;
                if(vertices[1].y > vertices[0].y){
                    index = 1;
                };
                if(vertices[2].y > vertices[index].y){
                    index = 2;
                }
                V3 a = vertices[index];
                vertices.erase(vertices.begin() + index);
                V3 b, c;
                if(vertices[0].y > vertices[1].y){
                    b = vertices[0];
                    c = vertices[1];
                } else {
                    b = vertices[1];
                    c = vertices[0];
                };

                bool hasTopFlat = false;
                std::vector<V3> topFlat;
                topFlat.reserve(3);

                bool hasBottomFlat = false;
                std::vector<V3> bottomFlat;
                bottomFlat.reserve(3);

                if(a.y == b.y){
                    hasTopFlat = true;
                    topFlat.push_back(a);
                    topFlat.push_back(b);
                    topFlat.push_back(c);
                } else if(b.y == c.y){
                    hasBottomFlat = true;
                    bottomFlat.push_back(a);
                    bottomFlat.push_back(b);
                    bottomFlat.push_back(c);
                } else {
                    float dx = (c.x - a.x) / (c.y - a.y);
                    float dz = (c.z - a.z) / (c.y - a.y);
                    V3 d = V3(a.x - dx * (a.y - b.y), b.y, a.z - dz * (a.y - b.y));

                    hasTopFlat = true;
                    topFlat.push_back(b);
                    topFlat.push_back(d);
                    topFlat.push_back(c);

                    hasBottomFlat = true;
                    bottomFlat.push_back(a);
                    bottomFlat.push_back(b);
                    bottomFlat.push_back(d);
                };
                
                if(hasTopFlat){
                    V3 a = topFlat[0];
                    V3 b = topFlat[1];
                    V3 c = topFlat[2];

                    if(a.y - c.y != 0 && b.y - c.y != 0){
                        float m1 = (a.x - c.x) / (a.y - c.y);
                        float m2 = (b.x - c.x) / (b.y - c.y);

                        for(float y = c.y;y<a.y;y++)
                        {
                            float x1 = c.x + m1 * (y - c.y);
                            float x2 = c.x + m2 * (y - c.y);
                            float minX = x1 < x2 ? x1 : x2;
                            float maxX = x1 > x2 ? x1 : x2;

                            for(float x = minX;x<maxX;x++)
                            {
                                if(x >= 0 && int(x) < 2 * canvasWidth && y >= 0 && int(y) < 2 * canvasHeight){
                                    float denominator = (b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y);
                                    float numerator1 = (b.y - c.y) * (x - c.x) + (c.x - b.x) * (y - c.y);
                                    float numerator2 = (c.y - a.y) * (x - c.x) + (a.x - c.x) * (y - c.y);

                                    float w1 = numerator1 / denominator;
                                    float w2 = numerator2 / denominator;
                                    float w3 = 1 - w1 - w2;

                                    float z = a.z * w1 + b.z * w2 + c.z * w3;

                                    int intX = x;
                                    int intY = y;

                                    if(buffer[intX][intY].z > z){
                                        buffer[intX][intY].z = z;
                                        // TODO: shade
                                        buffer[intX][intY].color = primitives[i].ambientColor;
                                    };
                                };
                            };
                        };
                    }
                };
                if(hasBottomFlat){
                    V3 a = bottomFlat[0];
                    V3 b = bottomFlat[1];
                    V3 c = bottomFlat[2];

                    if(b.y - a.y != 0 && c.y - a.y != 0){
                        float m1 = (b.x - a.x) / (b.y - a.y);
                        float m2 = (c.x - a.x) / (c.y - a.y);

                        for(float y = c.y;y<a.y;y++)
                        {
                            float x1 = a.x + m1 * (y - a.y);
                            float x2 = a.x + m2 * (y - a.y);
                            float minX = x1 < x2 ? x1 : x2;
                            float maxX = x1 > x2 ? x1 : x2;
                            for(float x = minX;x<maxX;x++)
                            {
                                if(x >= 0 && int(x) < 2 * canvasWidth && y >= 0 && int(y) < 2 * canvasHeight){
                                    float denominator = (b.y - c.y) * (a.x - c.x) + (c.x - b.x) * (a.y - c.y);
                                    float numerator1 = (b.y - c.y) * (x - c.x) + (c.x - b.x) * (y - c.y);
                                    float numerator2 = (c.y - a.y) * (x - c.x) + (a.x - c.x) * (y - c.y);

                                    float w1 = numerator1 / denominator;
                                    float w2 = numerator2 / denominator;
                                    float w3 = 1 - w1 - w2;

                                    float z = a.z * w1 + b.z * w2 + c.z * w3;

                                    int intX = x;
                                    int intY = y;

                                    if(buffer[intX][intY].z > z){
                                        buffer[intX][intY].z = z;
                                        // TODO: shade
                                        buffer[intX][intY].color = primitives[i].ambientColor;
                                    };
                                };
                            };
                        };
                    };
                };
            };
        
            this->log("Raster Completed");

            for(int x = 0;x<canvasWidth;x++)
            {
                for(int y = 0;y<canvasHeight;y++)
                {
                    V3 c1 = buffer[2 * x][2 * y].color;
                    V3 c2 = buffer[2 * x + 1][2 * y].color;
                    V3 c3 = buffer[2 * x + 1][2 * y + 1].color;
                    V3 c4 = buffer[2 * x][2 * y + 1].color;

                    int r = 0.25 * (c1.x + c2.x + c3.x + c4.x);
                    int g = 0.25 * (c1.y + c2.y + c3.y + c4.y);
                    int b = 0.25 * (c1.z + c2.z + c3.z + c4.z);

                    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                    SDL_RenderDrawPoint(renderer, x, y);
                };
            };

            SDL_RenderPresent(renderer);
        
            this->log("Render Completed");
        };
};

int main()
{
    int canvasWidth = 400;
    int canvasHeight = 300;

    SDL_Window* window;
    SDL_Renderer* renderer;

    SDL_CreateWindowAndRenderer(canvasWidth, canvasHeight, 0, &window, &renderer);
    SDL_SetWindowTitle(window, "Perspective Projection");

    Scene myScene = Scene();

    SceneObject cube1 = SceneObject::ColoredUnitCube(V3(0, 0, 5));
    cube1.internalTransform.setRotX(-0.2 * M_PI);
    myScene.objects.push_back(cube1);

    SceneObject cube2 = SceneObject::ColoredUnitCube(V3(1, 1, 6));
    cube2.internalTransform.setRotX(0.2 * M_PI);
    myScene.objects.push_back(cube2);

    SceneObject cube3 = SceneObject::ColoredUnitCube(V3(0, 2, 6));
    cube3.internalTransform.setRotY(1.2 * M_PI);
    myScene.objects.push_back(cube3);
    
    Camera myCamera = Camera();

    std::chrono::steady_clock::time_point lastTimestamp = std::chrono::steady_clock::now();

    SDL_Event e;
    bool running = true;
    while(running)
    {
        std::chrono::steady_clock::time_point currentTimestamp = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(currentTimestamp - lastTimestamp).count();
        lastTimestamp = currentTimestamp;
        std::cout << std::to_string(1000 * dt) + " ms" << std::endl;

        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT){
                running = false;
            } else {
                float moveDistance = 5 * dt;
                float rotateDistance = 2 * dt;
                
                switch(e.key.keysym.sym)
                {
                    case SDLK_UP:
                        myCamera.moveForward(moveDistance);
                        break;
                    case SDLK_DOWN:
                        myCamera.moveBackward(moveDistance);
                        break;
                    case SDLK_LEFT:
                    myCamera.moveLeft(moveDistance);
                        break;
                    case SDLK_RIGHT:
                        myCamera.moveRight(moveDistance);
                        break;
                    case SDLK_a:
                        myCamera.rot.y += rotateDistance;
                        break;
                    case SDLK_d:
                        myCamera.rot.y -= rotateDistance;
                        break;
                    case SDLK_w:
                        myCamera.rot.x += rotateDistance;
                        break;
                    case SDLK_s:
                        myCamera.rot.x -= rotateDistance;
                        break;
                };
            };
        };

        myScene.objects[0].internalTransform.changeRotY(0.1 * dt * M_PI);
        myScene.objects[1].internalTransform.changeRotY(-0.05 * dt * M_PI);
        myScene.objects[2].internalTransform.changeRotX(0.05 * dt * M_PI);

        myCamera.render(renderer, canvasWidth, canvasHeight, myScene);
    };

    return 0;
};