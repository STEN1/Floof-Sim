#include "Physics.h"
#include "cmath"
#include "../Components.h"
#include "../Vertex.h"
#include "../LoggerMacros.h"

bool FLOOF::Physics::TriangleBallIntersect(const FLOOF::Triangle &triangle, const glm::vec3 &position, const float &radius) {
   auto intersect = std::abs(glm::dot(position-triangle.A,triangle.N));
   return (intersect < radius);
}

glm::vec3
FLOOF::Physics::GetVelocityBall(const FLOOF::Triangle &triangle, const glm::vec3 &position, const BallComponent &ball, const glm::vec3 &velocity, const double & deltatime) {

    //calculate magic so old velocity matters
    glm::vec3 F{velocity};
    glm::vec3 G(0.0,-Math::Gravity,0.0);
    glm::vec3 N = -G;

    glm::vec3 tmpVelocity(0.f);
    F = G;
    // TODO fix velocity calculations
   // F = ball.Mass*velocity*static_cast<float>(deltatime);
    // F = mv/t
    //calculate reflect between ball velocity and triangle
    //glm::vec3 test = glm::reflect(G, triangle.N);
    //test += velocity;
    //test *= deltatime;
    //return new velocity
    if(TriangleBallIntersect(triangle,position,ball.Radius)){
        //LOG_INFO("Ball and triangle intersecting");
        F = (triangle.N+G)*glm::length(velocity);
        glm::vec3 tmp(triangle.N.x*triangle.N.y,triangle.N.z*triangle.N.y,(triangle.N.y*triangle.N.y)-1);
        F = tmp*static_cast<float>(Math::Gravity);
        //F =glm::cross(triangle.N,G) * (1/ball.Mass);
        F =(triangle.N-G) * (1/ball.Mass);
        if(glm::length(velocity) > 0.f){
            //F +=velocity;
        }
    }
    LOG_VEC("velocity ",velocity);
    LOG_VEC("F vector",F);
    return F;
}


