/*
* Author: Chris Campbell - www.iforce2d.net
*
* Copyright (c) 2006-2011 Erin Catto http://www.box2d.org
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#pragma once

#include <vector>
#include <set>

#ifndef DEGTORAD
#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f
#endif

enum {
    TDC_LEFT     = 0x1,
    TDC_RIGHT    = 0x2,
    TDC_UP       = 0x4,
    TDC_DOWN     = 0x8
};

//types of fixture user data
enum fixtureUserDataType {
    FUD_CAR_TIRE,
    FUD_GROUND_AREA
};

//a class to allow subclassing of different fixture user data
class FixtureUserData {
    fixtureUserDataType m_type;
protected:
    FixtureUserData(fixtureUserDataType type) : m_type(type) {}
public:
    virtual fixtureUserDataType getType() { return m_type; }
    virtual ~FixtureUserData() {}
};

//class to allow marking a fixture as a car tire
class CarTireFUD : public FixtureUserData {
public:
    CarTireFUD() : FixtureUserData(FUD_CAR_TIRE) {}
};

//class to allow marking a fixture as a ground area
class GroundAreaFUD : public FixtureUserData {
public:
    float frictionModifier;
    bool outOfCourse;

    GroundAreaFUD(float fm, bool ooc) : FixtureUserData(FUD_GROUND_AREA) {
        frictionModifier = fm;
        outOfCourse = ooc;
    }
};

class TDTire {
public:
    b2Body* m_body;
    float m_maxForwardSpeed;
    float m_maxBackwardSpeed;
    float m_maxDriveForce;
    float m_maxLateralImpulse;
    std::set<GroundAreaFUD*> m_groundAreas;
    float m_currentTraction;

    TDTire(b2World* world) {
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        m_body = world->CreateBody(&bodyDef);

        b2PolygonShape polygonShape;
        polygonShape.SetAsBox(0.5f, 1.25f);
        b2Fixture* fixture = m_body->CreateFixture(&polygonShape, 1);//shape, density
        fixture->SetUserData(new CarTireFUD());

        m_body->SetUserData(this);

        m_currentTraction = 1;
    }

    ~TDTire() {
        m_body->GetWorld()->DestroyBody(m_body);
    }

    void setCharacteristics(float maxForwardSpeed, float maxBackwardSpeed, float maxDriveForce, float maxLateralImpulse) {
        m_maxForwardSpeed = maxForwardSpeed;
        m_maxBackwardSpeed = maxBackwardSpeed;
        m_maxDriveForce = maxDriveForce;
        m_maxLateralImpulse = maxLateralImpulse;
    }

    void addGroundArea(GroundAreaFUD* ga) { m_groundAreas.insert(ga); updateTraction(); }
    void removeGroundArea(GroundAreaFUD* ga) { m_groundAreas.erase(ga); updateTraction(); }

    void updateTraction()
    {
        if (m_groundAreas.empty()) {
            m_currentTraction = 1;
        } else {
            //find area with highest traction
            m_currentTraction = 0;
            std::set<GroundAreaFUD*>::iterator it = m_groundAreas.begin();
            while (it != m_groundAreas.end()) {
                GroundAreaFUD* ga = *it;
                if (ga->frictionModifier > m_currentTraction)
                    m_currentTraction = ga->frictionModifier;
                ++it;
            }
        }
    }

    b2Vec2 getLateralVelocity() {
        b2Vec2 currentRightNormal = m_body->GetWorldVector(b2Vec2(1, 0));
        return b2Dot(currentRightNormal, m_body->GetLinearVelocity()) * currentRightNormal;
    }

    b2Vec2 getForwardVelocity() {
        b2Vec2 currentForwardNormal = m_body->GetWorldVector(b2Vec2(0, 1));
        return b2Dot(currentForwardNormal, m_body->GetLinearVelocity()) * currentForwardNormal;
    }

    void updateFriction() {
        //lateral linear velocity
        b2Vec2 impulse = m_body->GetMass() * -getLateralVelocity();
        if (impulse.Length() > m_maxLateralImpulse)
            impulse *= m_maxLateralImpulse / impulse.Length();
        m_body->ApplyLinearImpulse(m_currentTraction * impulse, m_body->GetWorldCenter());

        //angular velocity
        m_body->ApplyAngularImpulse(m_currentTraction * 0.1f * m_body->GetInertia() * -m_body->GetAngularVelocity());

        //forward linear velocity
        b2Vec2 currentForwardNormal = getForwardVelocity();
        float currentForwardSpeed = currentForwardNormal.Normalize();
        float dragForceMagnitude = -2 * currentForwardSpeed;
        m_body->ApplyForce(m_currentTraction * dragForceMagnitude * currentForwardNormal, m_body->GetWorldCenter());
    }

    void updateDrive(int controlState) {
        //find desired speed
        float desiredSpeed = 0;
        switch (controlState & (TDC_UP|TDC_DOWN)) {
            case TDC_UP:   desiredSpeed = m_maxForwardSpeed;  break;
            case TDC_DOWN: desiredSpeed = m_maxBackwardSpeed; break;
            default: return;//do nothing
        }

        //find current speed in forward direction
        b2Vec2 currentForwardNormal = m_body->GetWorldVector(b2Vec2(0, 1));
        float currentSpeed = b2Dot(getForwardVelocity(), currentForwardNormal);

        //apply necessary force
        float force = 0;
        if (desiredSpeed > currentSpeed)
            force = m_maxDriveForce;
        else if (desiredSpeed < currentSpeed)
            force = -m_maxDriveForce;
        else
            return;
        m_body->ApplyForce(m_currentTraction * force * currentForwardNormal, m_body->GetWorldCenter());
    }

    void updateTurn(int controlState) {
        float desiredTorque = 0;
        switch (controlState & (TDC_LEFT|TDC_RIGHT)) {
            case TDC_LEFT:  desiredTorque = 15;  break;
            case TDC_RIGHT: desiredTorque = -15; break;
            default: ;//nothing
        }
        m_body->ApplyTorque(desiredTorque);
    }
};


class TDCar {
    b2Body* m_body;
    std::vector<TDTire*> m_tires;
    b2RevoluteJoint *flJoint, *frJoint;
public:
    TDCar(b2World* world) {
        //create car body
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        m_body = world->CreateBody(&bodyDef);
        m_body->SetAngularDamping(3);

        b2Vec2 vertices[8];
        vertices[0].Set( 1.5,   0);
        vertices[1].Set(   3, 2.5);
        vertices[2].Set( 2.8, 5.5);
        vertices[3].Set(   1,  10);
        vertices[4].Set(  -1,  10);
        vertices[5].Set(-2.8, 5.5);
        vertices[6].Set(  -3, 2.5);
        vertices[7].Set(-1.5,   0);
        b2PolygonShape polygonShape;
        polygonShape.Set(vertices, 8);
        b2Fixture* fixture = m_body->CreateFixture(&polygonShape, 0.1f);//shape, density

        //prepare common joint parameters
        b2RevoluteJointDef jointDef;
        jointDef.bodyA = m_body;
        jointDef.enableLimit = true;
        jointDef.lowerAngle = 0;
        jointDef.upperAngle = 0;
        jointDef.localAnchorB.SetZero();//center of tire

        float maxForwardSpeed = 250;
        float maxBackwardSpeed = -40;
        float backTireMaxDriveForce = 300;
        float frontTireMaxDriveForce = 500;
        float backTireMaxLateralImpulse = 8.5;
        float frontTireMaxLateralImpulse = 7.5;

        //back left tire
        TDTire* tire = new TDTire(world);
        tire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, backTireMaxDriveForce, backTireMaxLateralImpulse);
        jointDef.bodyB = tire->m_body;
        jointDef.localAnchorA.Set(-3, 0.75f);
        world->CreateJoint(&jointDef);
        m_tires.push_back(tire);

        //back right tire
        tire = new TDTire(world);
        tire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, backTireMaxDriveForce, backTireMaxLateralImpulse);
        jointDef.bodyB = tire->m_body;
        jointDef.localAnchorA.Set(3, 0.75f);
        world->CreateJoint(&jointDef);
        m_tires.push_back(tire);

        //front left tire
        tire = new TDTire(world);
        tire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, frontTireMaxDriveForce, frontTireMaxLateralImpulse);
        jointDef.bodyB = tire->m_body;
        jointDef.localAnchorA.Set( -3, 8.5f );
        flJoint = (b2RevoluteJoint*)world->CreateJoint(&jointDef);
        m_tires.push_back(tire);

        //front right tire
        tire = new TDTire(world);
        tire->setCharacteristics(maxForwardSpeed, maxBackwardSpeed, frontTireMaxDriveForce, frontTireMaxLateralImpulse);
        jointDef.bodyB = tire->m_body;
        jointDef.localAnchorA.Set( 3, 8.5f );
        frJoint = (b2RevoluteJoint*)world->CreateJoint(&jointDef);
        m_tires.push_back(tire);
    }

    ~TDCar() {
        for (int i = 0; i < m_tires.size(); i++)
            delete m_tires[i];
    }

    void update(int controlState) {
        for (int i = 0; i < m_tires.size(); i++)
            m_tires[i]->updateFriction();
        for (int i = 0; i < m_tires.size(); i++)
            m_tires[i]->updateDrive(controlState);

        //control steering
        float lockAngle = 35 * DEGTORAD;
        float turnSpeedPerSec = 160 * DEGTORAD;//from lock to lock in 0.5 sec
        float turnPerTimeStep = turnSpeedPerSec / 60.0f;
        float desiredAngle = 0;
        switch (controlState & (TDC_LEFT|TDC_RIGHT)) {
        case TDC_LEFT:  desiredAngle = lockAngle;  break;
        case TDC_RIGHT: desiredAngle = -lockAngle; break;
        default: ;//nothing
        }
        float angleNow = flJoint->GetJointAngle();
        float angleToTurn = desiredAngle - angleNow;
        angleToTurn = b2Clamp(angleToTurn, -turnPerTimeStep, turnPerTimeStep);
        float newAngle = angleNow + angleToTurn;
        flJoint->SetLimits(newAngle, newAngle);
        frJoint->SetLimits(newAngle, newAngle);
    }
};

class MyDestructionListener :  public b2DestructionListener
{
    void SayGoodbye(b2Fixture* fixture)
    {
        if ( FixtureUserData* fud = (FixtureUserData*)fixture->GetUserData() )
            delete fud;
    }

    //(unused but must implement all pure virtual functions)
    void SayGoodbye(b2Joint* joint) {}
};

#if 0
class iforce2d_TopdownCar : public Test
{
public:
    iforce2d_TopdownCar()
    {
        m_world->SetGravity( b2Vec2(0,0) );
        m_world->SetDestructionListener(&m_destructionListener);

        //set up ground areas
        {
            b2BodyDef bodyDef;
            m_groundBody = m_world->CreateBody( &bodyDef );

            b2PolygonShape polygonShape;
            b2FixtureDef fixtureDef;
            fixtureDef.shape = &polygonShape;
            fixtureDef.isSensor = true;

            polygonShape.SetAsBox( 9, 7, b2Vec2(-10,15), 20*DEGTORAD );
            b2Fixture* groundAreaFixture = m_groundBody->CreateFixture(&fixtureDef);
            groundAreaFixture->SetUserData( new GroundAreaFUD( 0.5f, false ) );

            polygonShape.SetAsBox( 9, 5, b2Vec2(5,20), -40*DEGTORAD );
            groundAreaFixture = m_groundBody->CreateFixture(&fixtureDef);
            groundAreaFixture->SetUserData( new GroundAreaFUD( 0.2f, false ) );
        }

        //m_tire = new TDTire(m_world);
        //m_tire->setCharacteristics(100, -20, 150);

        m_car = new TDCar(m_world);

        m_controlState = 0;
    }

    ~iforce2d_TopdownCar()
    {
        //delete m_tire;
        delete m_car;
        m_world->DestroyBody( m_groundBody );
    }

    void Keyboard(unsigned char key)
    {
        switch (key) {
        case 'a' : m_controlState |= TDC_LEFT; break;
        case 'd' : m_controlState |= TDC_RIGHT; break;
        case 'w' : m_controlState |= TDC_UP; break;
        case 's' : m_controlState |= TDC_DOWN; break;
        default: Test::Keyboard(key);
        }
    }

    void KeyboardUp(unsigned char key)
    {
        switch (key) {
        case 'a' : m_controlState &= ~TDC_LEFT; break;
        case 'd' : m_controlState &= ~TDC_RIGHT; break;
        case 'w' : m_controlState &= ~TDC_UP; break;
        case 's' : m_controlState &= ~TDC_DOWN; break;
        default: Test::Keyboard(key);
        }
    }

    void handleContact(b2Contact* contact, bool began)
    {
        b2Fixture* a = contact->GetFixtureA();
        b2Fixture* b = contact->GetFixtureB();
        FixtureUserData* fudA = (FixtureUserData*)a->GetUserData();
        FixtureUserData* fudB = (FixtureUserData*)b->GetUserData();

        if ( !fudA || !fudB )
            return;

        if ( fudA->getType() == FUD_CAR_TIRE || fudB->getType() == FUD_GROUND_AREA )
            tire_vs_groundArea(a, b, began);
        else if ( fudA->getType() == FUD_GROUND_AREA || fudB->getType() == FUD_CAR_TIRE )
            tire_vs_groundArea(b, a, began);
    }

    void BeginContact(b2Contact* contact) { handleContact(contact, true); }
    void EndContact(b2Contact* contact) { handleContact(contact, false); }

    void tire_vs_groundArea(b2Fixture* tireFixture, b2Fixture* groundAreaFixture, bool began)
    {
        TDTire* tire = (TDTire*)tireFixture->GetBody()->GetUserData();
        GroundAreaFUD* gaFud = (GroundAreaFUD*)groundAreaFixture->GetUserData();
        if ( began )
            tire->addGroundArea( gaFud );
        else
            tire->removeGroundArea( gaFud );
    }

    void Step(Settings* settings)
    {
        /*m_tire->updateFriction();
        m_tire->updateDrive(m_controlState);
        m_tire->updateTurn(m_controlState);*/

        m_car->update(m_controlState);

        Test::Step(settings);

        //show some useful info
        m_debugDraw.DrawString(5, m_textLine, "Press w/a/s/d to control the car");
        m_textLine += 15;

        //m_debugDraw.DrawString(5, m_textLine, "Tire traction: %.2f", m_tire->m_currentTraction);
        //m_textLine += 15;
    }

    static Test* Create()
    {
        return new iforce2d_TopdownCar;
    }

    int m_controlState;
    MyDestructionListener m_destructionListener;

    b2Body* m_groundBody;
    TDCar* m_car;
};
#endif
