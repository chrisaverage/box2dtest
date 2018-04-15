#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QTimer>

#include "Box2D/Box2D.h"

//entire world is 100m x 100m, a sad small box...
const float WORLD_SIZE      = 100.0f;
const float HALF_WORLD_SIZE = WORLD_SIZE / 2.0f;

//Widget used to visualize state of a box2d world
class PreviewWidget : public QWidget, public b2Draw
{
    QPainter p;
    b2World* world;

public:
    PreviewWidget(b2World* world) : world(world)
    {
        //This stops Qt from filling the background with default window color
        setAttribute(Qt::WA_NoSystemBackground);

        //Set which elements to draw
        SetFlags(e_shapeBit);
    }

    //This is called by Qt in response to events like repaint, update or resize
    void paintEvent(QPaintEvent*) override
    {
        p.begin(this);
        p.save();

        p.fillRect(rect(), Qt::white);

        p.setRenderHint(QPainter::Antialiasing);

        const float half_width = width() / 2.0f;
        const float scale = half_width / HALF_WORLD_SIZE;

        // QWidget coordinates are: [0,0] is top left, [w,h] is bottom right
        // Move and scale painter so that [0,0] is at bottom center
        p.translate(half_width, height());
        p.scale(scale, -scale);

        world->DrawDebugData();

        p.restore();
        p.end();
    }

    void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override
    {
        QPen pen;
        pen.setCosmetic(true);
        pen.setColor(QColor::fromRgbF(color.r, color.g, color.b, color.a));
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);

        QPolygonF poly(vertexCount);

        for (int i = 0; i < vertexCount; ++i, ++vertices)
        {
            poly[i].setX(vertices->x);
            poly[i].setY(vertices->y);
        }

        p.drawPolygon(poly);
    }

    void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override
    {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor::fromRgbF(color.r, color.g, color.b, color.a));

        QPolygonF poly(vertexCount);

        for (int i = 0; i < vertexCount; ++i, ++vertices)
        {
            poly[i].setX(vertices->x);
            poly[i].setY(vertices->y);
        }

        p.drawPolygon(poly);
    }

    void DrawCircle(const b2Vec2&, float32, const b2Color&) override{/*TODO*/}
    void DrawSolidCircle(const b2Vec2&, float32, const b2Vec2&, const b2Color&) override {/*TODO*/}
    void DrawSegment(const b2Vec2&, const b2Vec2&, const b2Color&) override {/*TODO*/}
    void DrawTransform(const b2Transform&) override {/*TODO*/}
    void DrawPoint(const b2Vec2&, float32, const b2Color&) override {/*TODO*/}
};


//Adds a box to the world
void addBox(b2World& world, b2BodyType type, float32 center_x, float32 center_y, float32 w, float32 h,
            float32 angle = 0.0f, float32 friction = 0.8f, float32 restitution = 0.3f)
{
    b2PolygonShape shape;
    shape.SetAsBox(w /2.0f, h/2.0f);

    b2BodyDef def;
    def.type = type;
    def.position.Set(center_x, center_y);
    def.angle = angle;

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    fixtureDef.density = 10.0;
    fixtureDef.friction = friction;
    fixtureDef.restitution = restitution;

    b2Body* body = world.CreateBody(&def);
    body->CreateFixture(&fixtureDef);
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //Create a world with some gravity
    b2World world(b2Vec2(0.0f, -9.81f));

    //Add some bounding boxes
    addBox(world, b2_staticBody,  0.0f, 0.0f, WORLD_SIZE, 1.0f);
    addBox(world, b2_staticBody, -HALF_WORLD_SIZE, HALF_WORLD_SIZE, 1.0f, WORLD_SIZE);
    addBox(world, b2_staticBody,  HALF_WORLD_SIZE, HALF_WORLD_SIZE, 1.0f, WORLD_SIZE);

    //Create visualization widget
    PreviewWidget preview(&world);
    preview.resize(300, 300);
    preview.show();

    //Register preview widget as the debug renderer
    world.SetDebugDraw(&preview);

    //Create update timer and run simulation
    QTimer t;
    float32 interval = 1.0f / 60.0f;  // interval in seconds
    t.start(int(1000.0f * interval)); // Qt uses milliseconds so interval*1000

    //Update on timer tick
    QObject::connect(&t, &QTimer::timeout, [&]
    {
        world.Step(interval, 6, 2);
        preview.update();
    });

    //Add dynamic object every second
    QTimer object_timer;
    object_timer.start(1000);
    QObject::connect(&object_timer, &QTimer::timeout, [&]
    {
        float32 pos_x = (qrand() % (int)HALF_WORLD_SIZE) - HALF_WORLD_SIZE / 2;
        float32 rotation = qrand() / 1000.0f;

        addBox(world, b2_dynamicBody, pos_x, WORLD_SIZE, 5.0f, 5.0f, rotation);
    });

    //Start the event loop
    return a.exec();
}
